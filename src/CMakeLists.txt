if (NOT TARGET gpio_redirect)
    add_library("gpio_redirect" INTERFACE)

    # PIO program to Redirect GPIO
    # Compile .pio file with pioasm and put resulting header in the same directory
    pico_generate_pio_header("gpio_redirect" ${CMAKE_CURRENT_LIST_DIR}/redirect.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR})
    pico_generate_pio_header("gpio_redirect" ${CMAKE_CURRENT_LIST_DIR}/redirect_basic.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR})
    pico_generate_pio_header("gpio_redirect" ${CMAKE_CURRENT_LIST_DIR}/squarewave.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR})

    # Pull in required Pico-SDK libraries
    target_link_libraries("gpio_redirect" INTERFACE
            hardware_pio
    )
endif ()