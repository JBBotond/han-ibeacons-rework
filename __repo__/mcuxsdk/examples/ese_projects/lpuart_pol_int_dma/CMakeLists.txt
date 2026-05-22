cmake_minimum_required(VERSION 3.22.0)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(lpuart_pol_int_dma LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/ese_projects/lpuart_pol_int_dma)

include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES ${board_root}/${board}/ese_projects/lpuart_pol_int_dma/main.c
            ${board_root}/${board}/ese_projects/lpuart_pol_int_dma/benchmark.c
            ${board_root}/${board}/ese_projects/lpuart_pol_int_dma/benchmark.h
            ${board_root}/${board}/ese_projects/lpuart_pol_int_dma/gpio_output.c
            ${board_root}/${board}/ese_projects/lpuart_pol_int_dma/gpio_output.h
            ${board_root}/${board}/ese_projects/lpuart_pol_int_dma/lpuart0_dma.c
            ${board_root}/${board}/ese_projects/lpuart_pol_int_dma/lpuart0_dma.h
            ${board_root}/${board}/ese_projects/lpuart_pol_int_dma/lpuart0_interrupt.c
            ${board_root}/${board}/ese_projects/lpuart_pol_int_dma/lpuart0_interrupt.h
            ${board_root}/${board}/ese_projects/lpuart_pol_int_dma/lpuart0_polling.c
            ${board_root}/${board}/ese_projects/lpuart_pol_int_dma/lpuart0_polling.h
            ${board_root}/${board}/ese_projects/lpuart_pol_int_dma/retarget.c
)

include(${SdkRootDirPath}/${board_root}/${board}/ese_projects/lpuart_pol_int_dma/reconfig.cmake OPTIONAL)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
