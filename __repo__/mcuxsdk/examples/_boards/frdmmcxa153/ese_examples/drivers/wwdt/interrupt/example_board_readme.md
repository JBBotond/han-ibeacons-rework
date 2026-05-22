# WWDT - Interrupt

## Overview
WWDT interrupt example project

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
- An interrupt will be generated as soon as 1200ms have passed since the last `wwdt_feed()` call.
- When main finishes the 1500ms delay, "WWDT restart" is printed. The WWDT will then be restarted and normal operation will resume.

In the example below, the WWDT Timeout interrupt occurs at around 5200ms, which is 1200ms after the last `wwdt_feed()` call at 4000ms.

```TXT
WWDT - Interrupt example
Debug build Feb 28 2026 09:44:34
[    1000] WWDT reset
[    2000] WWDT reset
[    3000] WWDT reset
[    4000] WWDT reset
[    4595] SW pressed
[    4595] Start delay of 1500ms
[    5202] WWDT Timeout interrupt
[    6095] Delay finished
[    6095] WWDT restart
[    6095] WWDT reset
[    7095] WWDT reset
[    8095] WWDT reset
```
