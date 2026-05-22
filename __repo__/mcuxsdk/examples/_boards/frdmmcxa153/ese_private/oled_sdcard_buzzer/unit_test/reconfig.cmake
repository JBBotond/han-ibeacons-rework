add_compile_definitions(UNITY_INCLUDE_CONFIG_H)

mcux_add_armgcc_configuration(
    LD "-Xlinker --defsym=__stack_size__=0x0800 -Xlinker --defsym=__heap_size__=0x0"
)
