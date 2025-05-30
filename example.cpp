/**
 * rp2040_pin_redirect
 *
 * Author: Stephen Jingle <sib.li>
 * Created: 28 May 2025
 */

#include <cassert>
#include <iostream>

// Pico SDK
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// gpio_redirect generated header
#include "redirect.pio.h"
#include "redirect_basic.pio.h"


#define ASSERT(exp, msg) assert((void(msg), exp))


namespace config {
    struct pioMap_t {
        PIO pio;
        uint sm;
    };
    inline const pioMap_t           g_pio_Neopixel          = {pio0, 0};    // other PIO programs
    inline const pioMap_t           g_pio_Redirect          = {pio1, 0};
    // Alternative output on the different PIO bank (just for the sake of example)
    inline const pioMap_t           g_pio_RedirectBasic     = {pio0, 1};
} // config


void handleTerminate() {
    std::cerr << "Execution terminated" << std::endl;
    stdio_deinit_all();
}

[[noreturn]] int main() {
    std::set_terminate(handleTerminate);
    std::ostream::sync_with_stdio(false);

    // Enable stdio drivers (UART/RTT/USB serial)
    // Toggle appropriate pico_enable_stdio_*() in CMakeLists.txt
    stdio_init_all();

    // Initialize PIO to redirect any input from GPIO 0 to GPIO 1
    constexpr unsigned int pin_RedirectFrom = 0;
    constexpr unsigned int pin_RedirectTo   = 1;
    const int offset = pio_add_program(config::g_pio_Redirect.pio, &gpio_redirect_program);
    ASSERT(offset >= 0, "Error mapping PIO bank for gpio_redirect program");
    gpio_redirect_program_init(
        config::g_pio_Redirect.pio,
        config::g_pio_Redirect.sm,
        offset,
        pin_RedirectFrom,
        pin_RedirectTo
    );

    // Optional part, just for the sake of comparison:
    // Initialize naive PIO to redirect any input from GPIO 0 to GPIO 2
    // PIO state machines can share input pins. But only same-bank SMs can share output pins
    constexpr unsigned int pin_RedirectBasicTo   = 2;
    const int offsetBasic = pio_add_program(config::g_pio_RedirectBasic.pio, &gpio_redirect_basic_program);
    ASSERT(offsetBasic >= 0, "Error mapping PIO bank for gpio_redirect_basic program");
    gpio_redirect_basic_program_init(
        config::g_pio_RedirectBasic.pio,
        config::g_pio_RedirectBasic.sm,
        offsetBasic,
        pin_RedirectFrom, // same input pin
        pin_RedirectBasicTo
    );

    sleep_ms(1000);
    std::cout << "GPIO redirect initialized" << std::endl;
    std::cout << "pin" << pin_RedirectFrom << " -> pin" << pin_RedirectTo << " (side-set)" << std::endl;
    std::cout << "pin" << pin_RedirectFrom << " -> pin" << pin_RedirectBasicTo << " (basic)" << std::endl;
    const uint clk_sys_measured = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS) * 1000;
    std::cout << "CLK_SYS=" << clk_sys_measured << std::endl;

    while (true) {
        tight_loop_contents();
    }
} // main