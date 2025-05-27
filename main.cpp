#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "hardware/xosc.h"
#include "sounds.h"
#include "pwm_audio.h"

int main() {
    set_sys_clock_48mhz();

    stdio_init_all();
    
    buttons_init();

    buttons_set_sound_data(0, sound_01_data, sound_01_size);
    buttons_set_sound_data(1, sound_02_data, sound_02_size);
    buttons_set_sound_data(2, sound_03_data, sound_03_size);
    buttons_set_sound_data(3, sound_04_data, sound_04_size);
    buttons_set_sound_data(4, sound_05_data, sound_05_size);
    buttons_set_sound_data(5, sound_06_data, sound_06_size);
    buttons_set_sound_data(6, sound_07_data, sound_07_size);
    buttons_set_sound_data(7, sound_08_data, sound_08_size);
    buttons_set_sound_data(8, sound_09_data, sound_09_size);
    buttons_set_sound_data(9, sound_10_data, sound_10_size);
    buttons_set_sound_data(10, sound_11_data, sound_11_size);
    buttons_set_sound_data(11, sound_12_data, sound_12_size);

    pwm_audio_init();

#if 0
    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (1) {
        static size_t counter = 
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
        gpio_put(LED_PIN, 1);
        sleep_ms(1000);
    }
#endif  // #if 0

    while (1) {
        button_check();
        __wfi();
    }

    return 0;
}
