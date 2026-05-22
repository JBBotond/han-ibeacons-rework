cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(frdmmcxa153_assignments_oled LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_examples/assignments/oled)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${board_root}/${board}/ese_examples/assignments/oled/animations.c
            ${board_root}/${board}/ese_examples/assignments/oled/animations.h
            ${board_root}/${board}/ese_examples/assignments/oled/bitmaps.c
            ${board_root}/${board}/ese_examples/assignments/oled/bitmaps.h
            ${board_root}/${board}/ese_examples/assignments/oled/fonts.c
            ${board_root}/${board}/ese_examples/assignments/oled/fonts.h
            ${board_root}/${board}/ese_examples/assignments/oled/lpi2c.c
            ${board_root}/${board}/ese_examples/assignments/oled/lpi2c.h
            ${board_root}/${board}/ese_examples/assignments/oled/oled.c
            ${board_root}/${board}/ese_examples/assignments/oled/oled.h
            ${board_root}/${board}/ese_common/board/board.h
            ${board_root}/${board}/ese_common/serial/serial.c
            ${board_root}/${board}/ese_common/serial/serial.h
            ${board_root}/${board}/ese_common/serial/retarget.c
            examples/ese_examples/assignments/oled/main.c
            examples/ese_common/fifo/fifo.c
            examples/ese_common/fifo/fifo.h
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES ${board_root}/${board}/ese_common/board
             ${board_root}/${board}/ese_common/serial
             ${board_root}/${board}/ese_examples/assignments/oled
             examples/ese_examples/assignments/oled
             examples/ese_common/fifo
)

include(${SdkRootDirPath}/${board_root}/${board}/ese_examples/assignments/oled/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
