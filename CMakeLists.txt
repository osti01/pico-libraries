cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(project_pico_libraries C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()


add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

add_executable(pico-libraries pico-libraries.cpp)

# pull in common dependencies
target_link_libraries(pico-libraries pico_stdlib)


target_link_libraries(pico-libraries pico_stdlib hardware_spi pico_multicore hardware_pll)

pico_set_binary_type(pico-libraries no_flash)



# create map/bin/hex file etc.
pico_add_extra_outputs(pico-libraries)

