# LPCMP - Interrupt

## Overview
LPCMP interrupt example project

## Hardware requirements
- FRDM-MCXA153 board
- Type-C USB cable
- Potentiometer (any value between 10k - 100k Ohm), connected to P2_2/ADC0_A4/CMP0_IN0, 3V3, and GND.

## Board settings
- Default

## Preparation
- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Run or debug the application.

## Result
- Open a VCOM (115200-8n1) terminal to see the output.
- Every time the potentiometer sweeps from below 1.65V to above 1.65V, a message is printed and the green LED blinks.