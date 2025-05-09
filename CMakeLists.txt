# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)

project(alarme_industria C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(alarme_industria alarme_industria.c lib/ssd1306.c)

pico_set_program_name(alarme_industria "alarme_industria")
pico_set_program_version(alarme_industria "0.1")

# Generate PIO header
pico_generate_pio_header(alarme_industria ${CMAKE_CURRENT_LIST_DIR}/lib/animacao_matriz.pio)


# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(alarme_industria 1)
pico_enable_stdio_usb(alarme_industria 1)

# Add the standard library to the build
target_link_libraries(alarme_industria
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(alarme_industria PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(alarme_industria 
        hardware_i2c
        hardware_pio
        hardware_clocks
        hardware_adc
        hardware_pwm
        )

pico_add_extra_outputs(alarme_industria)

