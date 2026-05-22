
# Copyright 2026 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.22.0)

cmake_path(APPEND conf_file_path ${CMAKE_CURRENT_LIST_DIR} ${board} prj.conf)
set(CONF_FILE ${conf_file_path} ${CONF_FILE})
include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(display_game LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_projects/tft_lcd/ili9341_240x320/animation)


include(${board}/board_files.cmake)
include(${SdkRootDirPath}/CMakeLists.txt)


mcux_add_source(
    SOURCES board/board.h
            fifo/fifo.c
            fifo/fifo.h
            leds/leds.c
            leds/leds.h
            main.c
            resources/animations.c
            resources/animations.h
            resources/bitmaps.c
            resources/bitmaps.h
            resources/fonts.c
            resources/fonts.h
            serial/retarget.c
            serial/serial.c
            serial/serial.h
            tft_lcd/lpspi_master.c
            tft_lcd/lpspi_master.h
            tft_lcd/tft_lcd.c
            tft_lcd/tft_lcd.h
            tools/convert.py
            simonsaid/start.c
            simonsaid/start.h
)

mcux_add_include(
    INCLUDES board
             fifo
             leds
             resources
             serial
             tft_lcd
             simonsaid
)
mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)


mcux_add_include(INCLUDES tools .)
