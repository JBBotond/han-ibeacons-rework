cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(usb_device_cdc_vcom LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_examples/drivers/usb/device/cdc/vcom)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${board_root}/${board}/ese_examples/drivers/usb/device/cdc/vcom/main.c
            ${board_root}/${board}/ese_examples/drivers/usb/device/cdc/vcom/main.h
            ${board_root}/${board}/ese_examples/drivers/usb/device/cdc/vcom/usb_device_descriptor.c
            ${board_root}/${board}/ese_examples/drivers/usb/device/cdc/vcom/usb_device_descriptor.h
            ${board_root}/${board}/ese_common/board/board.h
            ${board_root}/${board}/ese_common/leds/leds.c
            ${board_root}/${board}/ese_common/leds/leds.h
            ${board_root}/${board}/ese_common/switches/switches.c
            ${board_root}/${board}/ese_common/switches/switches.h
            ${board_root}/${board}/ese_common/serial/serial.c
            ${board_root}/${board}/ese_common/serial/serial.h
            ${board_root}/${board}/ese_common/serial/retarget.c
            examples/ese_common/fifo/fifo.c
            examples/ese_common/fifo/fifo.h
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES ${board_root}/${board}/ese_examples/drivers/usb/device/cdc/vcom
             ${board_root}/${board}/ese_examples/drivers/usb/device/cdc/vcom/serial
             ${board_root}/${board}/ese_common/board
             ${board_root}/${board}/ese_common/leds
             ${board_root}/${board}/ese_common/switches
             examples/ese_common/fifo
)

include(${SdkRootDirPath}/${board_root}/${board}/ese_examples/drivers/usb/usb_device/cdc/vcom/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
