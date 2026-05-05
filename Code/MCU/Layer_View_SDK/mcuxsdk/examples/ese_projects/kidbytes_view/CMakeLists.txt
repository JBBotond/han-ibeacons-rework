cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(frdmmcxa153_kidbytes_view
    LANGUAGES C CXX ASM
    PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_projects/kidbytes_view)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES
        # TFT LCD low-level driver (ILI9341)
        ${board_root}/${board}/ese_projects/tft_lcd/ili9341_240x320/_common/tft_lcd/tft_lcd.c
        ${board_root}/${board}/ese_projects/tft_lcd/ili9341_240x320/_common/tft_lcd/tft_lcd.h
        ${board_root}/${board}/ese_projects/tft_lcd/ili9341_240x320/_common/tft_lcd/lpspi_master.c
        ${board_root}/${board}/ese_projects/tft_lcd/ili9341_240x320/_common/tft_lcd/lpspi_master.h

        # Shared resources (fonts)
        ${board_root}/${board}/ese_projects/tft_lcd/_common/resources/fonts.c
        ${board_root}/${board}/ese_projects/tft_lcd/_common/resources/fonts.h

        # HAL header
        examples/ese_projects/kidbytes_view/hal/display.h

        # display_drv (Layer 4)
        examples/ese_projects/kidbytes_view/drivers/display_drv.c
        examples/ese_projects/kidbytes_view/drivers/display_drv.h

        # Board support
        ${board_root}/${board}/ese_common/board/board.h
        ${board_root}/${board}/ese_common/serial/serial.c
        ${board_root}/${board}/ese_common/serial/serial.h
        ${board_root}/${board}/ese_common/serial/retarget.c
        ${board_root}/${board}/ese_common/leds/leds.c
        ${board_root}/${board}/ese_common/leds/leds.h
        ${board_root}/${board}/ese_common/switches/switches.c
        ${board_root}/${board}/ese_common/switches/switches.h

        # Application
        examples/ese_projects/kidbytes_view/main.c
        examples/ese_projects/kidbytes_view/v_screen.c
        examples/ese_projects/kidbytes_view/v_screen.h
        examples/ese_projects/kidbytes_view/v_icons.c
        examples/ese_projects/kidbytes_view/v_icons.h
        examples/ese_projects/kidbytes_view/v_fb.c
        examples/ese_projects/kidbytes_view/v_fb.h

        # FIFO (required by serial)
        examples/ese_common/fifo/fifo.c
        examples/ese_common/fifo/fifo.h
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES
        ${board_root}/${board}/ese_common/board
        ${board_root}/${board}/ese_common/serial
        ${board_root}/${board}/ese_common/leds
        ${board_root}/${board}/ese_common/switches
        ${board_root}/${board}/ese_projects/tft_lcd/_common/resources
        ${board_root}/${board}/ese_projects/tft_lcd/ili9341_240x320/_common/tft_lcd
        examples/ese_projects/kidbytes_view/hal
        examples/ese_projects/kidbytes_view/drivers
        examples/ese_projects/kidbytes_view
        examples/ese_common/fifo
)

include(${SdkRootDirPath}/${board_root}/${board}/ese_projects/kidbytes_view/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
