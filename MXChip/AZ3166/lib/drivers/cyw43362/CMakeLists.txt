cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)
# Set the project name
project(cyw43362)

# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CYPRESS_SRC)

set(CYPRESS_SRC
    43362A2_bin.c
    host_cm4.c
    platform.c
    platform_resource.c
    wiced_crypto.c
    wwd_ap.c
    wwd_ap_common.c
    wwd_buffer.c
    wwd_bus_common.c
    wwd_bus_protocol.c
    wwd_chip_specific_functions.c
    wwd_debug.c
    wwd_eapol.c
    wwd_internal.c
    wwd_management.c
    wwd_network.c
    wwd_platform.c
    wwd_platform_separate_mcu.c
    wwd_resources.c
    wwd_rtos.c
    wwd_SDIO.c
    wwd_sdpcm.c
    wwd_thread.c
    wwd_wifi.c
    wwd_wifi_chip_common.c
    wwd_wifi_sleep.c
    F4xx_lib/peripherals/platform_gpio.c
    F4xx_lib/peripherals/platform_mcu_powersave.c
    # Old ST Lib
    F4xx_lib/peripherals/libraries/src/stm32f4xx_exti.c
    F4xx_lib/peripherals/libraries/src/stm32f4xx_gpio.c
    F4xx_lib/peripherals/libraries/src/stm32f4xx_rcc.c
    F4xx_lib/peripherals/libraries/src/stm32f4xx_sdio.c
    F4xx_lib/peripherals/libraries/src/stm32f4xx_syscfg.c
)

# Add a library with the above sources
add_library(${PROJECT_NAME}
    STATIC
        ${CYPRESS_SRC}
)
# add_library(lib::cyw43362 ALIAS ${PROJECT_NAME})

add_dependencies(cyw43362 threadx filex netxduo)

# Preprocessor marcos
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        WICED_CUSTOM_NX_USER_H
        # TX_ENABLE_EXECUTION_CHANGE_NOTIFY
        wifi_firmware_image=resources_firmware_DIR_43362_DIR_43362A2_bin
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/internal
        ${CMAKE_CURRENT_SOURCE_DIR}/internal/bus_protocols
        ${CMAKE_CURRENT_SOURCE_DIR}/network
        ${CMAKE_CURRENT_SOURCE_DIR}/platform
        ${CMAKE_CURRENT_SOURCE_DIR}/RTOS
        ${CMAKE_CURRENT_SOURCE_DIR}/F4xx_lib/peripherals
        ${CMAKE_CURRENT_SOURCE_DIR}/F4xx_lib/peripherals/libraries
        ${CMAKE_CURRENT_SOURCE_DIR}/F4xx_lib/peripherals/libraries/inc
    PRIVATE
        ${CMSIS_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        threadx
        netxduo
)
