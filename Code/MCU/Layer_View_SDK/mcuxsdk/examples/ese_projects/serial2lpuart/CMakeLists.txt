cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(frdmmcxa153_serial2lpuart LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_projects/serial2lpuart)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${board_root}/${board}/ese_projects/serial2lpuart/main.c
            ${board_root}/${board}/ese_projects/serial2lpuart/lpuart2.c
            ${board_root}/${board}/ese_projects/serial2lpuart/lpuart2.h
            ${board_root}/${board}/ese_common/board/board.h
            ${board_root}/${board}/ese_common/leds/leds.c
            ${board_root}/${board}/ese_common/leds/leds.h
            ${board_root}/${board}/ese_common/serial/serial.c
            ${board_root}/${board}/ese_common/serial/serial.h
            ${board_root}/${board}/ese_common/serial/retarget.c
            examples/ese_common/fifo/fifo.c
            examples/ese_common/fifo/fifo.h
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES ${board_root}/${board}/ese_common/board
             ${board_root}/${board}/ese_common/leds
             ${board_root}/${board}/ese_common/serial
             examples/ese_common/fifo
)

include(${SdkRootDirPath}/${board_root}/${board}/ese_projects/serial2lpuart/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
