# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13...3.27)


if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.gitmodules")
    execute_process(COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

set(PICO_SDK_FETCH_FROM_GIT on)
set(CMAKE_CXX_STANDARD 23)

include(pico-sdk/pico_sdk_init.cmake)

project(seensay)

pico_sdk_init()

# Create directory for generated sound headers
set(GENERATED_SOUNDS_DIR "${CMAKE_BINARY_DIR}/generated_sounds")
file(MAKE_DIRECTORY ${GENERATED_SOUNDS_DIR})

# Custom command to convert sounds to headers
add_custom_command(
    OUTPUT ${GENERATED_SOUNDS_DIR}/sounds.h
    COMMAND python3 ${CMAKE_SOURCE_DIR}/convert_sounds.py ${CMAKE_SOURCE_DIR}/sounds ${GENERATED_SOUNDS_DIR}
    DEPENDS ${CMAKE_SOURCE_DIR}/convert_sounds.py
    COMMENT "Converting MP3 files to PCM headers"
    VERBATIM
)

# Create a custom target for sound conversion
add_custom_target(convert_sounds DEPENDS ${GENERATED_SOUNDS_DIR}/sounds.h)

add_executable(${CMAKE_PROJECT_NAME}
    main.cpp
    seensay.cpp
    ${GENERATED_SOUNDS_DIR}/sounds.h
)

# Add generated directory to include path
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${GENERATED_SOUNDS_DIR})

# Make sure sounds are converted before building main executable
add_dependencies(${CMAKE_PROJECT_NAME} convert_sounds)

target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC
    -Os
    -Wall
    -Wextra
)

target_link_libraries(${CMAKE_PROJECT_NAME} 
    pico_stdlib
    hardware_pwm
    hardware_timer
    hardware_irq
    hardware_dma
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)
pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 0)

pico_add_uf2_output(${CMAKE_PROJECT_NAME})
pico_generate_pio_header(${CMAKE_PROJECT_NAME})
pico_add_extra_outputs(${CMAKE_PROJECT_NAME})

execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_SHORT_SHA OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${GIT_EXECUTABLE} rev-list HEAD --count WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_REV_COUNT OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${GIT_EXECUTABLE} show -s --format=%ad HEAD --date=iso-strict WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_COMMIT_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${GIT_EXECUTABLE} show -s --format=%as HEAD --date=iso-strict WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_COMMIT_DATE_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file("${PROJECT_SOURCE_DIR}/version.h.in" "${CMAKE_BINARY_DIR}/version.h" @ONLY)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/version.h)

set(CMAKE_SIZE arm-none-eabi-size)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} ${CMAKE_PROJECT_NAME}.elf
    USES_TERMINAL)
