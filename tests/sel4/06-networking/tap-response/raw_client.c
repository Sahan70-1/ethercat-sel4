#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <os/sddf.h>
#include <sddf/util/printf.h>
#include <sddf/network/config.h>
#include <sddf/network/queue.h>
#include <sddf/network/constants.h>
#include <sddf/serial/config.h>
#include <sddf/serial/queue.h>
#include <sddf/timer/config.h>
#include <sddf/benchmark/config.h>

__attribute__((section(".net_client_config")))
net_client_config_t net_config;

__attribute__((section(".serial_client_config")))
serial_client_config_t serial_config;

__attribute__((section(".timer_client_config")))
timer_client_config_t timer_config;

__attribute__((section(".benchmark_client_config")))
benchmark_client_config_t benchmark_config;

static net_queue_handle_t tx_queue;
static net_queue_handle_t rx_queue;
static serial_queue_handle_t serial_queue;

#define FRAME_LENGTH 60
#define ETHERCAT_ETHERTYPE 0x88A4

static void send_test_frame(void)
{
    net_buff_desc_t desc;

    if (net_queue_full_active(&tx_queue)) {
        sddf_printf("RAW_ECAT: TX active queue full\n");
        return;
    }

    if (net_dequeue_free(&tx_queue, &desc) != 0) {
        sddf_printf("RAW_ECAT: no free TX buffer\n");
        return;
    }

    uint8_t *frame =
        (uint8_t *)net_config.tx_data.vaddr + desc.io_or_offset;

    memset(frame, 0, FRAME_LENGTH);

    /* Destination MAC: broadcast */
    memset(frame, 0xff, 6);

    /* Source MAC: assigned to this client */
    memcpy(frame + 6, net_config.mac_addr.addr, 6);

    /* EtherType: 0x88A4, in network byte order */
    frame[12] = (ETHERCAT_ETHERTYPE >> 8) & 0xff;
    frame[13] = ETHERCAT_ETHERTYPE & 0xff;

    /*
     * EtherCAT frame header:
     * type = 1, datagram length = 14 bytes.
     * Little-endian representation of 0x100E.
     */
    frame[14] = 0x0e;
    frame[15] = 0x10;

    /* EtherCAT BRD datagram */
    frame[16] = 0x07;  /* BRD command */
    frame[17] = 0x01;  /* Datagram index */

    frame[18] = 0x00;  /* ADP, low byte */
    frame[19] = 0x00;  /* ADP, high byte */

    frame[20] = 0x30;  /* ADO = 0x0130, AL Status */
    frame[21] = 0x01;

    frame[22] = 0x02;  /* Read two bytes */
    frame[23] = 0x00;  /* No additional datagrams */

    frame[24] = 0x00;  /* IRQ */
    frame[25] = 0x00;

    frame[26] = 0x00;  /* Read data initially zero */
    frame[27] = 0x00;

    frame[28] = 0x00;  /* Initial working counter */
    frame[29] = 0x00;

    desc.len = FRAME_LENGTH;
    desc.oid = 0;

    if (net_enqueue_active(&tx_queue, desc) != 0) {
        sddf_printf("RAW_ECAT: TX queue full\n");
        /* This path should not occur with the expected queue setup. */

        return;
    }

    sddf_printf("RAW_ECAT: queued EtherCAT BRD AL-status frame\n");

    /* Wake up the transmit virtualiser. */
    sddf_notify(net_config.tx.id);
}

static void process_rx(void)
{
    bool reprocess = true;
    bool returned_buffers = false;

    while (reprocess) {
        while (!net_queue_empty_active(&rx_queue)) {
            net_buff_desc_t desc;

            if (net_dequeue_active(&rx_queue, &desc) != 0) {
                break;
            }

            if (desc.len >= 14 &&
                desc.io_or_offset <=
                    (uint64_t)rx_queue.capacity * NET_BUFFER_SIZE -
                    NET_BUFFER_SIZE) {

                uint8_t *frame =
                    (uint8_t *)net_config.rx_data.vaddr +
                    desc.io_or_offset;

                if (frame[12] == 0x88 && frame[13] == 0xA4) {
                    static const uint8_t responder_mac[6] = {
                        0x02, 0xec, 0xa7, 0x00, 0x00, 0x01
                    };
                    static const uint8_t expected_datagram[10] = {
                        0x07, 0x01, 0x00, 0x00, 0x30,
                        0x01, 0x02, 0x00, 0x00, 0x00
                    };

                    if (desc.len < 30) {
                        sddf_printf("ECAT_RX: FAIL short frame\n");
                    } else if (
                        memcmp(frame, net_config.mac_addr.addr, 6) != 0 ||
                        memcmp(frame + 6, responder_mac, 6) != 0 ||
                        frame[14] != 0x0e ||
                        frame[15] != 0x10 ||
                        memcmp(frame + 16, expected_datagram, 10) != 0
                    ) {
                        sddf_printf("ECAT_RX: REJECT unexpected response\n");
                    } else {
                        uint16_t al_status =
                            (uint16_t)frame[26] |
                            ((uint16_t)frame[27] << 8);
                        uint16_t wkc =
                            (uint16_t)frame[28] |
                            ((uint16_t)frame[29] << 8);

                        sddf_printf(
                            "ECAT_RX: AL Status=%u WKC=%u\n",
                            (unsigned)al_status,
                            (unsigned)wkc
                        );

                        if (al_status == 2 && wkc == 1) {
                            sddf_printf(
                                "ECAT_RX: PASS simulated response validated\n"
                            );
                        } else {
                            sddf_printf(
                                "ECAT_RX: FAIL unexpected status or WKC\n"
                            );
                        }
                    }
                }
            }

            if (net_enqueue_free(&rx_queue, desc) == 0) {
                returned_buffers = true;
            }
        }

        net_request_signal_active(&rx_queue);
        reprocess = false;

        if (!net_queue_empty_active(&rx_queue)) {
            net_cancel_signal_active(&rx_queue);
            reprocess = true;
        }
    }

    if (returned_buffers && net_require_signal_free(&rx_queue)) {
        net_cancel_signal_free(&rx_queue);
        sddf_notify(net_config.rx.id);
    }
}

void init(void)
{
    serial_queue_init(
        &serial_queue,
        serial_config.tx.queue.vaddr,
        serial_config.tx.data.size,
        serial_config.tx.data.vaddr
    );

    serial_putchar_init(serial_config.tx.id, &serial_queue);

    net_queue_init(
        &tx_queue,
        net_config.tx.free_queue.vaddr,
        net_config.tx.active_queue.vaddr,
        net_config.tx.num_buffers
    );

    net_queue_init(
        &rx_queue,
        net_config.rx.free_queue.vaddr,
        net_config.rx.active_queue.vaddr,
        net_config.rx.num_buffers
    );

    net_buffers_init(&tx_queue, 0);

    sddf_printf("RAW_ECAT: client initialized\n");

    send_test_frame();

    process_rx();
}

void notified(sddf_channel ch)
{
    if (ch == net_config.rx.id) {
        process_rx();
    } else if (ch == net_config.tx.id) {
        /* TX completion notification; buffers can be reused. */
    }
}
