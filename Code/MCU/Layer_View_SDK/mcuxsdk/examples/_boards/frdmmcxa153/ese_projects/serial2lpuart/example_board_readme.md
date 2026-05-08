Overview
========
Demonstration project that forwards data from LPUART2 to serial (LPUART0).
The data is not interpreted, it is forwarded in both directions.

Hardware requirements
=====================
- FRDM-MCXA153 board
- Type-C USB cable
- Device with a serial interface, such as a HM-10 CC2541 Bluetooth 4.0 module or an ATGM336H GPS module

Board settings
==============
- Default

Preparation
===========
- Connect the FRDM-MCXA153 board and the module as follows

    |   FRDM-MCXA153   | Device   |
    |:----------------:|:--------:|
    | 3V3              | VCC      |
    | GND              | GND      |
    | P1_4/LPUART2_RDX | TXD      |
    | P1_5/LPUART2_TXD | RXD      |

- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Configure LPUART2 baudrate for your GPS module (should normally be (9600-8n1)).
- Build the application.
- Run or debug the application.

Result
======
- Open the VCOM (115200-8n1) in a terminal application of your choice.
- Data typed in the terminal application is sent to the module.
- Data from the module is visible in the terminal application.
