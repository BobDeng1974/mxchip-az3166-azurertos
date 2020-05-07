cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)
# Set the project name
project(mqtt_iot_hub)

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

# set(NX_USER_FILE "nx_user.h")

# Create the executable
add_executable(${PROJECT_NAME}
    main.c
    mqtt_iothub.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/board_init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/wifi_init.c
)

# add_dependencies(mqtt_iot_hub theadx netxduo stm32f4xx cyw43362)

# Preprocessor marcos
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        USE_COM_PORT
        # TX_ENABLE_EXECUTION_CHANGE_NOTIFY
        WIFI_SSID=Liki
        WIFI_PASSWORD=19860206
        WIFI_SECURITY=WICED_SECURITY_WPA2_MIXED_PSK
        WIFI_COUNTRY=WICED_COUNTRY_CHINA
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../common
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        threadx
        netxduo
        stm32f4xx
        cyw43362
)

STM32_SET_TARGET_PROPERTIES(${PROJECT_NAME})
