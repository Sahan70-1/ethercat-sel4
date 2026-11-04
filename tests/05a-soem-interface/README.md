# Experiment 5A — SOEM interface investigation

Read the [source audit](../../docs/experiment-05a-soem-network-audit.md) first.
This is the EtherCAT research track; it is separate from seL4 Experiment 05.

## Reproduce the host checks

Requires Linux, Git, a C compiler, Make, CMake >= 3.28, and pthread/rt libraries.
Keep upstream dependencies outside this repository. On Ubuntu/WSL, install
`build-essential git cmake` and check `cmake --version`; use a current CMake
in a Python virtual environment if the distro version is too old.

```sh
git clone https://github.com/OpenEtherCATsociety/SOEM.git ../SOEM-05a
git -C ../SOEM-05a checkout --detach 49fc34923b973c17700592319159af22157350c9
bash tests/05a-soem-interface/verify.sh ../SOEM-05a
```

Run from the project root. The script uses a fresh temporary output directory
by default; an optional second argument selects a fresh output directory.
It compiles and executes the two existing frame demos, builds SOEM and
`simple_ng`, and checks its usage/adapter-list path. No root permission is
needed. It sends no frames. A successful build is not successful discovery.

For source review, inspect `oshw/linux/nicdrv.c`, `oshw/linux/nicdrv.h`,
`osal/linux/osal.c`, `oshw/linux/oshw.c`, `cmake/Linux.cmake`, and callers in
`src/ec_base.c` and `src/ec_main.c` in that pinned checkout.

[Recorded validation](../../results/05a-soem-interface/README.md).
Next practical step: reproduce these host checks on WSL and compare the full
SOEM commit ID. Experiment 5B will introduce and test the backend seam separately.
