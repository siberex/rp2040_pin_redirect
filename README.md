# GPIO redirect for RP2040 and Pico boards

Redirect single GPIO pin with PIO.

Should work without any additional setup on Pico/Pico2 boards and any other boards based on Raspberry RP2040 and RP2350 MCUs.

Uses [side-set](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf#page=332&zoom=100,153,745) output with minimal possible cycle count (will trigger as fast as board could possibly can).


# Usage

Vendor-in to your project:

```bash
mkdir -p vendor
git submodule add https://github.com/siberex/rp2040_pin_redirect.git vendor/rp2040_pin_redirect
```

CMakeLists.txt:

```cmake
# Assuming Pico SDK set up earlier

add_subdirectory(vendor/rp2040_pin_redirect/src)
target_link_libraries(${PROJECT}
        gpio_redirect
)
```

In your code:

```c++
// Pico SDK
#include "pico/stdlib.h"
#include "hardware/pio.h"
// gpio_redirect generated header
#include "redirect.pio.h"

int main() {
    // Initialize PIO to redirect any input from GPIO 0 to GPIO 1
    constexpr uint pin_RedirectFrom = 0;
    constexpr uint pin_RedirectTo = 1;
    const int offset = pio_add_program(pio0, &gpio_redirect_program);
    gpio_redirect_program_init(pio0, 0, offset, pin_RedirectFrom, pin_RedirectTo);

    while (true) {
        tight_loop_contents();
    }
} // main
```