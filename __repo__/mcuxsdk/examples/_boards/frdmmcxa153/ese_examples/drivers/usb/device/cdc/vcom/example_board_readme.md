Overview
========
USB Device CDC VCOM example demonstrates how to implement a USB CDC VCOM device using the NXP MCUXpresso SDK.

Hardware requirements
=====================
- FRDM-MCXA153 board
- 2x Type-C USB cable

Board settings
==============
- Default

Preparation
===========
- Connect a type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Connect a type-C USB cable between the host PC and the MCU-USB port (J8) on the target board.
- Build the application.
- Run the application.

Result
======
- Open *two* the VCOMs (115200-8n1) in a terminal application of your choice.
    - COMn - MCU-Link VCom Port. Shows application log information.
    - COMm - USB Serial Device.
- All data sent by USB Serial Device will be echoed back in both COM ports.
- Data sent by the MCU-Link VCom Port will not be echoed back.
- Type `red=on` to turn on the red LED, `red=off` to turn it off.
- Type `green=on` to turn on the green LED, `green=off` to turn it off.
- Type `blue=on` to turn on the blue LED, `blue=off` to turn it off.
