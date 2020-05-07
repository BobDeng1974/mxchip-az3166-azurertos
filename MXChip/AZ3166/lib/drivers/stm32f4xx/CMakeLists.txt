cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)
# Set the project name
project(stm32f4xx LANGUAGES C ASM)

find_package(CMSIS REQUIRED)
find_package(STM32HAL REQUIRED COMPONENTS gpio tim uart)

add_library(${PROJECT_NAME}
    STATIC
        ${CMSIS_SOURCES}
        ${STM32HAL_SOURCES}
)
# add_library(lib::stm32f4xx ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMSIS_INCLUDE_DIRS}
        ${STM32HAL_INCLUDE_DIR}
)

# TODO: Set the chip definitions, make it simpiler
if(NOT STM32_CHIP_TYPE)
    if(NOT STM32_CHIP)
        message(WARNING "Neither STM32_CHIP_TYPE nor STM32_CHIP selected, you'll have to use STM32_set_CHIP_DEFINITIONS directly")
    else()
        STM32_GET_CHIP_TYPE(${STM32_CHIP} STM32_CHIP_TYPE)
    endif()
endif()
STM32_SET_CHIP_DEFINITIONS(${PROJECT_NAME} ${STM32_CHIP_TYPE})
