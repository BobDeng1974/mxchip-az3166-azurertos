cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)

# Set the project name
project(create_threads LANGUAGES C ASM)

# The linker output should be .elf so we can keep track of it
set(CMAKE_EXECUTABLE_SUFFIX ".elf")
# set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".elf")

# Create the executable
add_executable(${PROJECT_NAME}
    main.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/../tx_vector_table.s
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        threadx
        stm32f4xx
)

STM32_SET_TARGET_PROPERTIES(${PROJECT_NAME})
STM32_ADD_HEX_BIN_TARGETS(${PROJECT_NAME})
