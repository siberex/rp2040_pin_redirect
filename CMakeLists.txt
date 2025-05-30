# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if (WIN32)
    set(USERHOME $ENV{USERPROFILE})
else ()
    set(USERHOME $ENV{HOME})
endif ()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif ()
# ====================================================================================

cmake_minimum_required(VERSION 3.20)

set(PROJECT pin_redirect)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be done before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.1.1")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.1.1 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# It is necessary to include the pico sdk cmake before defining the project in CMake:
project(${PROJECT} C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable
add_executable(${PROJECT}
        example.cpp
)

# Targeting CLK_SYS=18MHz
# Note: USB will NOT work with that clock speed
#target_compile_definitions(${PROJECT} PRIVATE
#        PLL_SYS_REFDIV=2
#        PLL_SYS_VCO_FREQ_HZ=882000000
#        PLL_SYS_POSTDIV1=7
#        PLL_SYS_POSTDIV2=7
#        SYS_CLK_HZ=18000000
#)

# Targeting CLK_SYS=48MHz
target_compile_definitions(${PROJECT} PRIVATE
        PLL_SYS_REFDIV=1
        PLL_SYS_VCO_FREQ_HZ=1440000000
        PLL_SYS_POSTDIV1=6
        PLL_SYS_POSTDIV2=5
        SYS_CLK_HZ=48000000
)

# Targeting CLK_SYS=200 MHz
#target_compile_definitions(${PROJECT} PRIVATE
#        PLL_SYS_REFDIV=1
#        PLL_SYS_VCO_FREQ_HZ=1200000000
#        PLL_SYS_POSTDIV1=6
#        PLL_SYS_POSTDIV2=1
#        SYS_CLK_HZ=200000000
#)

target_compile_options(${PROJECT} PRIVATE
        -Wall
        -Wextra
        -Wformat
        # Disable compiler-flavored ABI-compatibility warnings
        -Wno-psabi
)

pico_set_program_name(${PROJECT} "${PROJECT}")
pico_set_program_version(${PROJECT} "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT} 0)
# Note: USB serial console adds significant overhead (TinyUSB init and stuff)
pico_enable_stdio_usb(${PROJECT} 1)
# See Segger RTT readme section on how to use RTT
pico_enable_stdio_rtt(${PROJECT} 1)

add_subdirectory(src)

# Add the standard include files to the build
target_include_directories(${PROJECT} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        src
)

# Add the standard library to the build
target_link_libraries(${PROJECT}
        # Pico SDK
        pico_stdlib
        pico_time
        hardware_pio
        hardware_clocks
        # Vendored libs
        gpio_redirect
)

# Picotool processing (signing/encrypting/etc) and creating hex/bin/map/uf2 files in addition to ELF
pico_add_extra_outputs(${PROJECT})

# "make flash" to flash the firmware with openocd (connect the board with SWD connector first)
add_custom_target(flash
        openocd -f ${CMAKE_CURRENT_LIST_DIR}/openocd.cfg -c "program ${PROJECT}.elf verify reset exit"
        COMMENT "openocd: program verify reset"
        DEPENDS ${PROJECT}
)

# "make gdb" to launch gdb server for both cores and RTT console
# Connect to serial console: telnet 127.0.0.1 9090
# Connect with gdb (core0): gdb "${TARGET}.elf" -ex "target extended-remote localhost:3333"
add_custom_target(gdb
        openocd -f ${CMAKE_CURRENT_LIST_DIR}/openocd_gdb.cfg
        COMMENT "openocd: GDB server with RTT"
        DEPENDS ${PROJECT}
)

# "make flash_usb" to flash the firmware with picotool (connect the board in BOOTSEL mode first)
add_custom_target(flash_usb
        picotool load -f -x ${PROJECT}.uf2
        COMMENT "picotool load"
        DEPENDS ${PROJECT}
)
