idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES driver
    )

idf_component_get_property(tusb_lib espressif__tinyusb COMPONENT_LIB)
target_include_directories(${tusb_lib} main/tusb_config.h)