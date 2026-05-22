cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(usb_device_hid_generic LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_examples/drivers/usb/device/hid/generic)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${board_root}/${board}/ese_examples/drivers/usb/device/hid/generic/main.c
            ${board_root}/${board}/ese_examples/drivers/usb/device/hid/generic/main.h
            ${board_root}/${board}/ese_examples/drivers/usb/device/hid/generic/usb_device_descriptor.c
            ${board_root}/${board}/ese_examples/drivers/usb/device/hid/generic/usb_device_descriptor.h
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

            # Tool files, not part of the build, but needed to be copied
            ${board_root}/${board}/ese_examples/drivers/usb/device/hid/generic/test.py
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES ${board_root}/${board}/ese_examples/drivers/usb/device/hid/generic
             ${board_root}/${board}/ese_examples/drivers/usb/device/hid/generic/serial
             ${board_root}/${board}/ese_common/board
             ${board_root}/${board}/ese_common/leds
             ${board_root}/${board}/ese_common/switches
             examples/ese_common/fifo
)

include(${SdkRootDirPath}/${board_root}/${board}/ese_examples/drivers/usb/usb_device/hid/generic/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
