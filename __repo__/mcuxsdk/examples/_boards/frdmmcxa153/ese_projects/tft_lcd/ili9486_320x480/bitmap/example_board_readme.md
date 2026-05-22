Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA153 board
- 3.5 inch TFT Display 320*480 pixels - Mega-Due Compatible - ILI9486
- Personal Computer

Board settings
============
No special settings are required.

Prepare the Demo
===============
1. Connect the FRDM-MCXA153 board and the TFT LCD module as follows

    |   FRDM-MCXA153  |   TFT LCD  |
    |:---------------:|:----------:|
    |  P2_0           |  LCD_DB00  |
    |  P2_1           |  LCD_DB01  |
    |  P2_2           |  LCD_DB02  |
    |  P2_3           |  LCD_DB03  |
    |  P2_4           |  LCD_DB04  |
    |  P2_5           |  LCD_DB05  |
    |  P2_6           |  LCD_DB06  |
    |  P2_7           |  LCD_DB07  |
    |  P1_4           |  LCD_DB08  |
    |  P1_5           |  LCD_DB09  |
    |  P1_6           |  LCD_DB10  |
    |  P1_7           |  LCD_DB11  |
    |  P1_8           |  LCD_DB12  |
    |  P1_9           |  LCD_DB13  |
    |  P1_10          |  LCD_DB14  |
    |  P1_11          |  LCD_DB15  |
    |  P3_6           |  LCD_RS    |
    |  P3_7           |  LCD_WR    |
    |  P3_8           |  LCD_CS    |
    |  P3_9           |  LCD_RST   |
    |  P1_0           |  SPI_MOSI  |
    |  P1_1           |  SPI_CLK   |
    |  P1_2           |  SPI_MISO  |
    |  P1_3           |  SD_CS     |
    |  5V             |  5V        |
    |  GND            |  GND       |

2. Connect the type-C USB cable between the host PC and the MCU-Link port (J15) on the target board.
3. Build the application.
4. Run or debug the application.

Running the demo
===============
- The TFT LCD module displays bitmaps and responds to SW2 input.

