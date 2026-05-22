cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(assignments_fatfs LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_examples/assignments/fatfs)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${board_root}/${board}/ese_common/board/board.h
            ${board_root}/${board}/ese_common/leds/leds.c
            ${board_root}/${board}/ese_common/leds/leds.h
            ${board_root}/${board}/ese_common/serial/serial.c
            ${board_root}/${board}/ese_common/serial/serial.h
            ${board_root}/${board}/ese_common/serial/retarget.c
            ${board_root}/${board}/ese_examples/assignments/fatfs/diskio.c
            ${board_root}/${board}/ese_examples/assignments/fatfs/diskio.h
            ${board_root}/${board}/ese_examples/assignments/fatfs/ff.c
            ${board_root}/${board}/ese_examples/assignments/fatfs/ff.h
            ${board_root}/${board}/ese_examples/assignments/fatfs/ffconf.h
            ${board_root}/${board}/ese_examples/assignments/fatfs/ffunicode.c
            ${board_root}/${board}/ese_examples/assignments/fatfs/lpspi_master.c
            ${board_root}/${board}/ese_examples/assignments/fatfs/lpspi_master.h
            examples/ese_examples/assignments/fatfs/main.c
            examples/ese_common/fifo/fifo.c
            examples/ese_common/fifo/fifo.h
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES ${board_root}/${board}/ese_common/board
             ${board_root}/${board}/ese_common/leds
             ${board_root}/${board}/ese_common/serial
             ${board_root}/${board}/ese_examples/assignments/fatfs
             examples/ese_examples/assignments/fatfs
             examples/ese_common/fifo
)

include(${SdkRootDirPath}/${board_root}/${board}/ese_examples/assignments/fatfs/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
