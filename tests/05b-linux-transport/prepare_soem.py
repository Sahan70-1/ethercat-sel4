#!/usr/bin/env python3
"""Export and adapt a pinned SOEM revision into a NEW disposable directory.

Never patches the student's existing checkout. Fails on revision/anchor drift.
The generated source retains SOEM's existing upstream license notices.
"""
import argparse
import io
from pathlib import Path
import subprocess
import tarfile

REVISION = "49fc34923b973c17700592319159af22157350c9"


def replace_once(text, old, new):
    if text.count(old) != 1:
        raise RuntimeError(f"SOEM patch anchor is not unique: {old[:80]!r}")
    return text.replace(old, new, 1)


def prepare(source, output):
    actual = subprocess.check_output(["git", "-C", str(source), "rev-parse", "HEAD"], text=True).strip()
    if actual != REVISION:
        raise RuntimeError(f"Expected {REVISION}, found {actual}")
    if subprocess.check_output(["git", "-C", str(source), "status", "--porcelain", "--untracked-files=no"]):
        raise RuntimeError("SOEM has tracked changes; preserve/review them first")
    output.mkdir(parents=True, exist_ok=False)
    archive = subprocess.check_output(["git", "-C", str(source), "archive", REVISION])
    with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
        tar.extractall(output, filter="data")

    header = output / "oshw/linux/nicdrv.h"
    text = replace_once(header.read_text(), "#include <pthread.h>", '#include <pthread.h>\n#include "ec_transport.h"')
    text = replace_once(text, "   int *sock;", "   int *sock;\n   ec_transport transport;")
    header.write_text(text)

    path = output / "oshw/linux/nicdrv.c"
    text = replace_once(path.read_text(), '#include "osal.h"', '#include "osal.h"\n#include "ec_linux_transport.h"')
    text = replace_once(text, "   rval = 0;\n   if (secondary)",
                        "   /* 5B supports one port only; redundancy is not qualified. */\n   if (secondary) return 0;\n   rval = 0;\n   if (secondary)")
    text = replace_once(text, "   /* we use RAW packet socket, with packet type ETH_P_ECAT */",
                        "   ec_linux_transport_bind(&port->stack.transport, psock, EC_BUFSIZE);\n   /* we use RAW packet socket, with packet type ETH_P_ECAT */")
    text = replace_once(text, "   rval = send(*stack->sock, (*stack->txbuf)[idx], lp, 0);",
                        "   rval = ec_transport_send(&stack->transport, (*stack->txbuf)[idx], lp) == EC_IO_OK ? lp : -1;")
    text = replace_once(text, "   bytesrx = recv(*stack->sock, (*stack->tempbuf), lp, MSG_DONTWAIT);",
                        "   size_t received = 0;\n   bytesrx = ec_transport_receive(&stack->transport, (*stack->tempbuf), lp, &received) == EC_IO_OK ? (int)received : -1;")
    text = replace_once(text, "   lp = (*stack->txbuflength)[idx];",
                        "   if (idx >= EC_MAXBUF) return -1;\n   lp = (*stack->txbuflength)[idx];")
    text = replace_once(text, "   rxbuf = &(*stack->rxbuf)[idx];",
                        "   if (idx >= EC_MAXBUF) return EC_NOFRAME;\n   rxbuf = &(*stack->rxbuf)[idx];")
    # The upstream capacity check alone does not validate the actual RX length.
    text = replace_once(text, "      else if (ecx_recvpkt(port, stacknumber))",
                        "      else if (ecx_recvpkt(port, stacknumber) && port->tempinbufs >= (int)(ETH_HEADERSIZE + EC_HEADERSIZE))")
    text = replace_once(text, "            if (l <= EC_BUFSIZE - 2) /* Drop packet if wkc offset is outside bounds */",
                        "            if (l >= EC_HEADERSIZE && l <= EC_BUFSIZE - 2 && (int)(ETH_HEADERSIZE + l + 2) <= port->tempinbufs)")
    text = replace_once(text, "               if (idxf == idx)",
                        "               if (idxf == idx && idx < EC_MAXBUF &&\n                   (*stack->txbuflength)[idx] >= (int)(ETH_HEADERSIZE + l + 2) &&\n                   (*stack->txbuflength)[idx] <= port->tempinbufs)")
    text = replace_once(text, "                  if (idxf < EC_MAXBUF && (*stack->rxbufstat)[idxf] == EC_BUF_TX)",
                        "                  if (idxf < EC_MAXBUF && (*stack->rxbufstat)[idxf] == EC_BUF_TX &&\n                      (*stack->txbuflength)[idxf] >= (int)(ETH_HEADERSIZE + l + 2) &&\n                      (*stack->txbuflength)[idxf] <= port->tempinbufs)")
    start = text.index("   /* use ppoll to reduce busy_polling */")
    end = text.index("   do\n   {\n      poll_err = ppoll", start)
    text = text[:start] + "   int poll_err;\n" + text[end:]
    text = replace_once(text, "      poll_err = ppoll(fdsp, pollcnt, &timeout_spec, NULL);", """      uint64_t now;
      ec_io_result result = ec_transport_now(&port->stack.transport, &now);
      if (result != EC_IO_OK) return EC_NOFRAME;
      uint64_t end_us = (uint64_t)timer->stop_time.tv_sec * 1000000 +
                        ((uint64_t)timer->stop_time.tv_nsec + 999) / 1000;
      uint64_t wake = end_us;
      if (wake > now && wake - now > 50) wake = now + 50;
      result = ec_transport_wait(&port->stack.transport, wake);
      if (result == EC_IO_ERROR || result == EC_IO_INVALID) return EC_NOFRAME;
      poll_err = 0;""")
    path.write_text(text)
    print(f"Prepared single-port SOEM copy: {output}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    prepare(args.source.resolve(), args.output.resolve())
