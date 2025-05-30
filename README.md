# GPIO redirect for RP2040 and Pico boards

Redirect single GPIO pin with PIO.

Should work without any additional setup on Pico/Pico2 boards and any other boards based on Raspberry RP2040 and RP2350 MCUs.

Uses `INPUT_SYNC_BYPASS` and [side-set](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf#page=332&zoom=100,153,745) output with minimal possible cycle count (will trigger as fast as the board is capable of).


# Usage

Vendor-in to your project:

```bash
mkdir -p vendor
git submodule add https://github.com/siberex/rp2040_pin_redirect.git vendor/rp2040_pin_redirect
```

Edit your `CMakeLists.txt`:

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
// gpio_redirect pioasm-generated header
#include "redirect.pio.h"

int main() {
    // Initialize PIO to redirect any input from GPIO 0 to GPIO 1
    constexpr unsigned int pin_RedirectFrom = 0;
    constexpr unsigned int pin_RedirectTo   = 1;
    // Using state machine #3 in PIO bank #1
    const int offset = pio_add_program(pio0, &gpio_redirect_program);
    gpio_redirect_program_init(pio1, 3, offset, pin_RedirectFrom, pin_RedirectTo);

    while (true) {
        tight_loop_contents();
    }
}
```

Note: Hi-Z input will not change the output state (it will keep the last value, either 0 or 1).


# Clock comparisons

There is alternative [redirect_basic.pio](./src/redirect_basic.pio) implementation which uses the same instruction count, but without a side-set.

You can run two state machines from the same input pin.

With high-enough-resolution oscilloscope you can compare outputs on the native clock speed (200MHz for RP2040 with `PICO_USE_FASTEST_SUPPORTED_CLOCK=1`).

With low-res scope you can reduce RP2040 system clock [using](https://github.com/raspberrypi/pico-examples/blob/84e8d489ca321a4be90ee49e36dc29e5c645da08/clocks/hello_48MHz/hello_48MHz.c#L49) `clock_configure()`. 

Check out [CMakeLists.txt](./CMakeLists.txt) for the example binary: it provides 18MHz, 48MHz and 200MHz system clock configurations.

See also: `python3 "$PICO_SDK_PATH/src/rp2_common/hardware_clocks/scripts/vcocalc.py" 18`

```c++
#include "redirect.pio.h"
#include "redirect_basic.pio.h"
int main() {
    // Initialize PIO to redirect any input from GPIO 0 to GPIOs 1 and 2
    constexpr unsigned int pin_RedirectFrom     = 0;
    constexpr unsigned int pin_RedirectTo       = 1;
    constexpr unsigned int pin_RedirectToBasic  = 2;
    // Using state machine #3 in PIO bank #1
    const int offset = pio_add_program(pio0, &gpio_redirect_program);
    gpio_redirect_program_init(pio1, 3, offset, pin_RedirectFrom, pin_RedirectTo);
    
    // Different PIO bank just for the sake of example, state machine #1
    // PIO state machines can share input pins. But only same-bank SMs can share output pins
    const int offsetBasic = pio_add_program(pio0, &gpio_redirect_basic_program);
    gpio_redirect_basic_program_init(pio1, 1, offsetBasic, pin_RedirectFrom, pin_RedirectToBasic);
    
    while (true) {
        tight_loop_contents();
    }
}
```


# Build example

```bash
mkdir -p build && cd build
cmake -DPICO_BOARD=pico -DCMAKE_BUILD_TYPE=Release --fresh ..
# cmake -DPICO_BOARD=pimoroni_tiny2350 -DCMAKE_BUILD_TYPE=Release --fresh ..
make
# Flash with openocd:
make flash
# Flash with picotool:
make flash_usb
```
