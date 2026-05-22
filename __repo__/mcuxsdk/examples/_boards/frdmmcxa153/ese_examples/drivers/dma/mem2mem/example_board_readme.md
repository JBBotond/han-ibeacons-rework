# DMA - MEM2MEM

## Overview
Copy data from memory-to-memory by using the DMA controller.

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
- Observe the output in the terminal application indicating whether the data was copied successfully from the source buffer to the destination buffer.
- This is also indicated by the RGB LED on the board:
    - Green RGB LED blinks: data copied successfully from src to dst buffer.
    - Red RGB LED blinks: data not copied successfully from src to dst buffer.

