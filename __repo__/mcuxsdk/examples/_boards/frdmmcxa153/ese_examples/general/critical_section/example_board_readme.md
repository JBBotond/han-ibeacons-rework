# General - Critical section

## Overview
Critical section example project

## Details
Do you want to know more details about this example? Check the [details](https://ese.han.nl/~harends/frdm-mcxa153-site/index.html).

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
- Open the VCOM (115200-8n1) in a terminal application of your choice.
- Press SW3. Try to get the timing right and trigger the race condition (the buffer printing different values on a single SW3 click).

