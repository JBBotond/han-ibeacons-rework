# WWDT - Reset

## Overview
WWDT reset example project

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
- The WWDT must be reset by calling the function `wwdt_feed()` every 1200ms. In normal operation, this function is called every 1000ms.
- Press SW2 or SW3. This will introduce a blocking delay of 1500ms in the main loop.
- The microcontroller will reset as soon as 1200ms have passed since the last `wwdt_feed()` call.
- 4.092ms before the reset occurs, a WWDT Warning interrupt will occur.

In the example below, the WWDT warning interrupt occurs at around 5197ms, which is 1197ms after the last `wwdt_feed()` call at 4000ms. The WWDT reset occurs at around 5200ms, which is approximately 4ms after the warning interrupt. After the reset, the program starts again from the beginning

```TXT
WWDT - Reset example
Debug build Feb 28 2026 09:19:33
[    1000] WWDT reset
[    2000] WWDT reset
[    3000] WWDT reset
[    4000] WWDT reset
[    4518] SW pressed
[    4518] Start delay of 1500ms
[    5197] WWDT Warning interrupt
WWDT - Reset example
Debug build Feb 28 2026 09:19:33
[    1000] WWDT reset
[    2000] WWDT reset
[    3000] WWDT reset
[    4000] WWDT reset
[    5000] WWDT reset
[    6000] WWDT reset
[    7000] WWDT reset
[    8000] WWDT reset
```
