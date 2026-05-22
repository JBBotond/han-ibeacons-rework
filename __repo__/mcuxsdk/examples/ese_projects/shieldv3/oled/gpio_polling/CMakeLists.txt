cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(shieldv3_oled_polling LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_projects/shieldv3/oled/polling)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${board_root}/${board}/ese_projects/shieldv3/oled/polling/main.c
            ${board_root}/${board}/ese_projects/shieldv3/oled/polling/bitmaps.c
            ${board_root}/${board}/ese_projects/shieldv3/oled/polling/bitmaps.h
            ${board_root}/${board}/ese_projects/shieldv3/oled/polling/fonts.c
            ${board_root}/${board}/ese_projects/shieldv3/oled/polling/fonts.h
            ${board_root}/${board}/ese_projects/shieldv3/oled/polling/lpi2c0_controller_polling.c
            ${board_root}/${board}/ese_projects/shieldv3/oled/polling/lpi2c0_controller_polling.h
            ${board_root}/${board}/ese_projects/shieldv3/oled/polling/ssd1306.c
            ${board_root}/${board}/ese_projects/shieldv3/oled/polling/ssd1306.h
)

include(${SdkRootDirPath}/${board_root}/${board}/ese_projects/shieldv3/oled/polling/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
