# EtherCAT Frame Handling Experiments

## Experiment 1: Frame Construction and Parsing

Constructs an EtherCAT-formatted Ethernet frame using C.
Checks EtherType 0x88A4, EtherCAT header fields, command
type, data length, and Working Counter.

Result: PASS

## Experiment 2: Working Counter Simulation

Simulates two EtherCAT slaves processing a Broadcast Read.

Test cases:
- Both slaves respond: WKC = 2
- One slave responds: WKC = 1
- Neither slave responds: WKC = 0

Result: All simulation checks passed.

## Environment

Ubuntu 24.04 (WSL2)
GCC 13.3.0

## Compilation

gcc -std=c11 -Wall -Wextra -Werror frame_demo.c -o frame_demo
gcc -std=c11 -Wall -Wextra -Werror ethercat_wkc_demo.c -o wkc_demo

## Execution

./frame_demo
./wkc_demo

## Limitations

These are software-only experiments.
No physical or virtual EtherCAT slaves were used.
No Ethernet frames were transmitted.
