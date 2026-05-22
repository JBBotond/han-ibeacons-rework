cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(shieldv3_neopixel_flexpwm LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_projects/shieldv3/neopixel/flexpwm)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${board_root}/${board}/ese_projects/shieldv3/neopixel/flexpwm/main.c
            ${board_root}/${board}/ese_projects/shieldv3/neopixel/flexpwm/neopixel_flexpwm.c
            ${board_root}/${board}/ese_projects/shieldv3/neopixel/flexpwm/neopixel_flexpwm.h
)

include(${SdkRootDirPath}/${board_root}/${board}/ese_projects/shieldv3/neopixel/flexpwm/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
