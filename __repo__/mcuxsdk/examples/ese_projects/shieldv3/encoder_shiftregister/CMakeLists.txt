cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(shieldv3_encoder_shiftregister LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_projects/shieldv3/encoder_shiftregister)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${board_root}/${board}/ese_projects/shieldv3/encoder_shiftregister/main.c
            ${board_root}/${board}/ese_projects/shieldv3/encoder_shiftregister/encoder.c
            ${board_root}/${board}/ese_projects/shieldv3/encoder_shiftregister/encoder.h
            ${board_root}/${board}/ese_projects/shieldv3/encoder_shiftregister/hc595.c
            ${board_root}/${board}/ese_projects/shieldv3/encoder_shiftregister/hc595.h
            ${board_root}/${board}/ese_projects/shieldv3/encoder_shiftregister/lpspi1_master.c
            ${board_root}/${board}/ese_projects/shieldv3/encoder_shiftregister/lpspi1_master.h
            ${board_root}/${board}/ese_common/board/board.h
            ${board_root}/${board}/ese_common/serial/serial.c
            ${board_root}/${board}/ese_common/serial/serial.h
            ${board_root}/${board}/ese_common/serial/retarget.c
            examples/ese_common/fifo/fifo.c
            examples/ese_common/fifo/fifo.h
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES ${board_root}/${board}/ese_common/board
             ${board_root}/${board}/ese_common/serial
             examples/ese_common/fifo
)

include(${SdkRootDirPath}/${board_root}/${board}/ese_projects/shieldv3/encoder_shiftregister/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
