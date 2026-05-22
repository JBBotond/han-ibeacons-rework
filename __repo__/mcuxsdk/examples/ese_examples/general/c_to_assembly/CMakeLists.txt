cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(c_to_assembly LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_examples/general/c_to_assembly)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${board_root}/${board}/ese_examples/general/c_to_assembly/${multicore_foldername}/main.c
            ${board_root}/${board}/ese_examples/general/c_to_assembly/${multicore_foldername}/multiply.s
)

include(${SdkRootDirPath}/${board_root}/${board}/ese_examples/general/c_to_assembly/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
