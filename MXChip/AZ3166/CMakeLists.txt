cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)

# See https://cmake.org/cmake/help/latest/policy/CMP0079.html for more info
cmake_policy(SET CMP0079 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/arm_none_eabi_stm32.cmake")

# Flags for Azure RTOS components
set(THREADX_ARCH "cortex_m4")
set(THREADX_TOOLCHAIN "gnu")

# Project name, version and languages
project(threadX_demo
    VERSION 6.0.0
    LANGUAGES C ASM
)

# Include helper modules
include(FeatureSummary)
include(utilities)

# Provide a custom tx_user.h file
# set(TX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/apps/common/tx_user.h")
set(NX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/apps/common/nx_user.h")
# set(FX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/apps/common/fx_user.h")
# set(UX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/apps/common/ux_user.h")
# set(GX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/apps/common/gx_user.h")

# Pick up the required Azure RTOS components
add_azrtos_component_dir(threadx)
add_azrtos_component_dir(filex)
add_azrtos_component_dir(netx-duo)

# Drivers
add_subdirectory(lib/drivers/stm32f4xx)
add_subdirectory(lib/drivers/cyw43362)

# Add samples sub directories
add_subdirectory(apps)

# Apply the linker scripts to the app target
# if(EXISTS ${LINKER_SCRIPT})
#     message(STATUS "Using linker script: ${LINKER_SCRIPT}")
#     target_link_options(${PROJECT_NAME} INTERFACE -T ${LINKER_SCRIPT})
# endif()

# Dump a report of features turned on/off across all included libraries
feature_summary(WHAT ALL)