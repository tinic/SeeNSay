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
    
    PWMAudio& audio = PWMAudio::instance();
    audio.set_button_sound(0, sound_01_data, sound_01_size);
    audio.set_button_sound(1, sound_02_data, sound_02_size);
    audio.set_button_sound(2, sound_03_data, sound_03_size);
    audio.set_button_sound(3, sound_04_data, sound_04_size);
    audio.set_button_sound(4, sound_05_data, sound_05_size);
    audio.set_button_sound(5, sound_06_data, sound_06_size);
    audio.set_button_sound(6, sound_07_data, sound_07_size);
    audio.set_button_sound(7, sound_08_data, sound_08_size);
    audio.set_button_sound(8, sound_09_data, sound_09_size);
    audio.set_button_sound(9, sound_10_data, sound_10_size);
    audio.set_button_sound(10, sound_11_data, sound_11_size);
    audio.set_button_sound(11, sound_12_data, sound_12_size);

    while (1) {
        audio.update();
        __wfi();
    }

    return 0;
}
