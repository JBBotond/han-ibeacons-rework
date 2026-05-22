cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(lpi2c_p3t1755_polling LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_examples/drivers/lpi2c/p3t1755/polling)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${board_root}/${board}/ese_common/board/board.h
            ${board_root}/${board}/ese_examples/drivers/lpi2c/p3t1755/polling/lpi2c.c
            ${board_root}/${board}/ese_examples/drivers/lpi2c/p3t1755/polling/lpi2c.h
            ${board_root}/${board}/ese_examples/drivers/lpi2c/p3t1755/polling/p3t1755.c
            ${board_root}/${board}/ese_examples/drivers/lpi2c/p3t1755/polling/p3t1755.h
            ${board_root}/${board}/ese_common/serial/serial.c
            ${board_root}/${board}/ese_common/serial/serial.h
            ${board_root}/${board}/ese_common/serial/retarget.c
            examples/ese_examples/drivers/lpi2c/p3t1755/polling/main.c
            examples/ese_common/fifo/fifo.c
            examples/ese_common/fifo/fifo.h
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES ${board_root}/${board}/ese_common/board
             ${board_root}/${board}/ese_examples/drivers/lpi2c/p3t1755/polling
             ${board_root}/${board}/ese_common/serial
             examples/ese_common/fifo
)

include(${SdkRootDirPath}/${board_root}/${board}/ese_examples/drivers/lpi2c/p3t1755/polling/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
