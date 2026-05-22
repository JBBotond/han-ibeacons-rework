# LPSPI Master Polling

## Overview
LPSPI as a polling based master example project

## Details
Do you want to know more details about this example? Check the [details](https://ese.han.nl/~harends/frdm-mcxa153-site/index.html).

## Hardware requirements
- FRDM-MCXA153 board
- Jumper wire
- Type-C USB cable

## Board settings
- Default

## Preparation
- Connect a jumper wires from P1_0/LPSPI0_SDO to P1_2/LPSPI0_SDI
- Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
- Build the application.
- Run or debug the application.

## Result
- On succesfull transmission
    ```txt
    Test 1: MATCH
    tx_data: 48 65 6C 6C 6F 20 57 6F 72 6C 64 21 00
    rx_data: 48 65 6C 6C 6F 20 57 6F 72 6C 64 21 00
    Test 2: MATCH
    tx_data: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
    rx_data: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
    ```
