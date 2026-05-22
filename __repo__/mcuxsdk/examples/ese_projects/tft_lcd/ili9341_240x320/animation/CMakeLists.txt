cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(tft_lcd_ili9341_240x320_animation LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_projects/tft_lcd/ili9341_240x320/animation)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${board_root}/${board}/ese_projects/tft_lcd/_common/resources/animations.c
            ${board_root}/${board}/ese_projects/tft_lcd/_common/resources/animations.h
            ${board_root}/${board}/ese_projects/tft_lcd/_common/resources/bitmaps.c
            ${board_root}/${board}/ese_projects/tft_lcd/_common/resources/bitmaps.h
            ${board_root}/${board}/ese_projects/tft_lcd/_common/resources/fonts.c
            ${board_root}/${board}/ese_projects/tft_lcd/_common/resources/fonts.h
            ${board_root}/${board}/ese_projects/tft_lcd/ili9341_240x320/_common/tft_lcd/tft_lcd.c
            ${board_root}/${board}/ese_projects/tft_lcd/ili9341_240x320/_common/tft_lcd/tft_lcd.h
            ${board_root}/${board}/ese_projects/tft_lcd/ili9341_240x320/_common/tft_lcd/lpspi_master.c
            ${board_root}/${board}/ese_projects/tft_lcd/ili9341_240x320/_common/tft_lcd/lpspi_master.h
            ${board_root}/${board}/ese_common/board/board.h
            ${board_root}/${board}/ese_common/leds/leds.c
            ${board_root}/${board}/ese_common/leds/leds.h
            ${board_root}/${board}/ese_common/serial/serial.c
            ${board_root}/${board}/ese_common/serial/serial.h
            ${board_root}/${board}/ese_common/serial/retarget.c
            examples/ese_projects/tft_lcd/ili9341_240x320/animation/main.c
            examples/ese_common/fifo/fifo.c
            examples/ese_common/fifo/fifo.h

            # Tool files, not part of the build, but needed to be copied
            examples/ese_projects/tft_lcd/tools/convert.py
            examples/ese_projects/tft_lcd/tools/made_by_ese.bmp
            examples/ese_projects/tft_lcd/tools/nxp.bmp
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES ${board_root}/${board}/ese_common/board
             ${board_root}/${board}/ese_common/leds
             ${board_root}/${board}/ese_common/serial
             ${board_root}/${board}/ese_projects/tft_lcd/_common/resources
             ${board_root}/${board}/ese_projects/tft_lcd/ili9341_240x320/_common/tft_lcd
             examples/ese_common/fifo
)

include(${SdkRootDirPath}/${board_root}/${board}/ese_projects/tft_lcd/ili9341_240x320/animation/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
