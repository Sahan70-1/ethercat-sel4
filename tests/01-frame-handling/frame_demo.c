
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FRAME_SIZE 60
#define ETH_HEADER 14
#define ECAT_HEADER 2
#define DATAGRAM_HEADER 10
#define PAYLOAD_SIZE 2
#define WKC_SIZE 2

// Read a 16-bit little-endian value
static uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

int main(void)
{
    uint8_t frame[FRAME_SIZE] = {0};

    const uint8_t source_mac[6] = {
        0x02, 0x00, 0x00, 0x00, 0x00, 0x01
    };

    const uint16_t datagrams_size =
        DATAGRAM_HEADER + PAYLOAD_SIZE + WKC_SIZE;

    const uint16_t ethercat_header =
        (uint16_t)(0x1000 | datagrams_size);

    // Ethernet header
    memset(frame, 0xFF, 6);
    memcpy(frame + 6, source_mac, 6);

    frame[12] = 0x88;
    frame[13] = 0xA4;

    // EtherCAT header
    frame[14] = (uint8_t)ethercat_header;
    frame[15] = (uint8_t)(ethercat_header >> 8);

    // EtherCAT datagram header
    frame[16] = 0x07;           // BRD command
    frame[17] = 0x01;           // Datagram index

    // Bytes 18-21: address = 0
    frame[22] = PAYLOAD_SIZE;   // Data length = 2

    // Bytes 24-25: IRQ = 0
    // Bytes 26-27: placeholder data = 0
    // Bytes 28-29: WKC = 0
    // Remaining bytes: Ethernet padding

    // Parse the constructed frame
    uint16_t ethertype =
        (uint16_t)((frame[12] << 8) | frame[13]);

    uint16_t ec_header = read_le16(frame + ETH_HEADER);

    uint16_t reported_ec_length =
        (uint16_t)(ec_header & 0x07FF);

    uint16_t data_length =
        (uint16_t)(read_le16(frame + 22) & 0x07FF);

    uint16_t wkc = read_le16(
        frame + ETH_HEADER + ECAT_HEADER +
        DATAGRAM_HEADER + data_length
    );

    printf("EtherType: 0x%04X\n", ethertype);
    printf("EtherCAT type: %u\n", ec_header >> 12);
    printf("EtherCAT datagrams length: %u bytes\n",
           reported_ec_length);
    printf("Command: 0x%02X (BRD: broadcast read)\n",
           frame[16]);
    printf("Datagram data length: %u bytes\n",
           data_length);
    printf("Working counter: %u\n", wkc);

    int valid =
        ethertype == 0x88A4 &&
        (ec_header >> 12) == 1 &&
        reported_ec_length ==
            DATAGRAM_HEADER + data_length + WKC_SIZE &&
        FRAME_SIZE >=
            ETH_HEADER + ECAT_HEADER + reported_ec_length;

    printf("Local format checks: %s\n",
           valid ? "PASS" : "FAIL");

    printf("Constructed %d-byte Ethernet buffer "
           "(not transmitted)\n", FRAME_SIZE);

    return valid ? 0 : 1;
}
