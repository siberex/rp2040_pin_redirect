/**
 * rp2040_pin_redirect
 *
 * Author: Stephen Jingle <sib.li>
 * Created: 28 May 2025
 */

#include <cassert>

// Pico SDK
#include "pico/stdlib.h"
#include "hardware/pio.h"

// gpio_redirect generated header
#include "redirect.pio.h"

#define ASSERT(exp, msg) assert((void(msg), exp))

namespace config {
    struct pioMap_t {
        PIO pio;
        uint sm;
    };
    inline const pioMap_t           g_pio_Neopixel          = {pio0, 0};    // other PIO programs
    inline const pioMap_t           g_pio_Redirect          = {pio1, 0};
} // config

[[noreturn]] int main() {
    // Initialize PIO to redirect any input from GPIO 0 to GPIO 1
    constexpr uint pin_RedirectFrom = 0;
    constexpr uint pin_RedirectTo = 1;
    const int offset = pio_add_program(config::g_pio_Redirect.pio, &gpio_redirect_program);
    ASSERT(offset >= 0, "Error mapping PIO bank for gpio_redirect program");
    gpio_redirect_program_init(config::g_pio_Redirect.pio, config::g_pio_Redirect.sm, offset, pin_RedirectFrom, pin_RedirectTo);

    while (true) {
        tight_loop_contents();
    }
} // main