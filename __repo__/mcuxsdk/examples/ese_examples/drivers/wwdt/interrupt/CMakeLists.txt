cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(wwdt_interrupt LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_examples/drivers/wwdt/interrupt)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${board_root}/${board}/ese_common/board/board.h
            ${board_root}/${board}/ese_common/leds/leds.c
            ${board_root}/${board}/ese_common/leds/leds.h
            ${board_root}/${board}/ese_common/switches/switches.c
            ${board_root}/${board}/ese_common/switches/switches.h
            ${board_root}/${board}/ese_common/serial/serial.c
            ${board_root}/${board}/ese_common/serial/serial.h
            ${board_root}/${board}/ese_common/serial/retarget.c
            ${board_root}/${board}/ese_examples/drivers/wwdt/interrupt/${core_id}/wwdt.c
            ${board_root}/${board}/ese_examples/drivers/wwdt/interrupt/${core_id}/wwdt.h
            examples/ese_examples/drivers/wwdt/interrupt/main.c
            examples/ese_common/fifo/fifo.c
            examples/ese_common/fifo/fifo.h
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES ${board_root}/${board}/ese_common/board
             ${board_root}/${board}/ese_examples/drivers/wwdt/interrupt
             ${board_root}/${board}/ese_common/leds
             ${board_root}/${board}/ese_common/switches
             ${board_root}/${board}/ese_common/serial
             ${board_root}/${board}/ese_examples/drivers/wwdt/interrupt/${core_id}
             examples/ese_common/fifo
)

include(${SdkRootDirPath}/${board_root}/${board}/ese_examples/drivers/wwdt/interrupt/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
