# CRC - CRC-32

## Overview
CRC-32 example project

## Hardware requirements
- FRDM-MCXA153 board
- Type-C USB cable

## Board settings
- Default

## Preparation
- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Run or debug the application.

## Result
- The serial console (115200-8n1) will print the CRC-32 calculation results.
- The green RGB LED is blinking, indicating all CRC-32 calculations are correct.
- By making a change to any of the data values in the arrays (data0, data1 and/or data2), the red RGB LED starts blinking, indicating a CRC-32 calculations is incorrect.