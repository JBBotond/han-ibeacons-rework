cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(shieldv3_oled_dma LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_projects/shieldv3/oled/dma)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${board_root}/${board}/ese_projects/shieldv3/oled/dma/main.c
            ${board_root}/${board}/ese_projects/shieldv3/oled/dma/bitmaps.c
            ${board_root}/${board}/ese_projects/shieldv3/oled/dma/bitmaps.h
            ${board_root}/${board}/ese_projects/shieldv3/oled/dma/fonts.c
            ${board_root}/${board}/ese_projects/shieldv3/oled/dma/fonts.h
            ${board_root}/${board}/ese_projects/shieldv3/oled/dma/lpi2c0_controller_dma.c
            ${board_root}/${board}/ese_projects/shieldv3/oled/dma/lpi2c0_controller_dma.h
            ${board_root}/${board}/ese_projects/shieldv3/oled/dma/ssd1306.c
            ${board_root}/${board}/ese_projects/shieldv3/oled/dma/ssd1306.h
)

include(${SdkRootDirPath}/${board_root}/${board}/ese_projects/shieldv3/oled/dma/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
