Overview
========
Project to check the address of I2C devices connected to the FRDM-MCXA153 board.

Hardware requirements
=====================
- FRDM-MCXA153 board
- Type-C USB cable
- I2C device(s) connected to the board

Board settings
==============
- Default

Preparation
===========
- Connect the FRDM-MCXA153 board and I2C devices(s) to

    |   FRDM-MCXA153   | Device(s) |
    |:----------------:|:---------:|
    | P3_27/LPI2C0_SCL | SCL       |
    | P3_28/LPI2C0_SDA | SDA       |

- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Run or debug the application.

Result
======
- Open the VCOM (115200-8n1) in a terminal application of your choice.
- The addresses of the connected I2C devices are displayed in the terminal application.
