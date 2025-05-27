#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "hardware/xosc.h"
#include "sounds.h"
#include "pwm_audio.h"

int main() {
    // Skip USB/stdio initialization for power saving
    // stdio_init_all();
    
    // Reduce system clock for power saving (48MHz instead of 125MHz)
    set_sys_clock_khz(48000, true);

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
        
        // If no audio playing, enter deeper power saving
        if (!pwm_audio_is_playing()) {
            // Scale down clock even further when idle
            set_sys_clock_khz(12000, true);
            __wfi();
            // Restore clock when woken
            set_sys_clock_khz(48000, true);
        } else {
            __wfi(); // Light sleep while playing
        }
    }

    return 0;
}
