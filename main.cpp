#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "sounds.h"
#include "pwm_audio.h"

int main() {
    stdio_init_all();
    
    // Initialize PWM audio system
    pwm_audio_init();
    
    // Initialize button system
    buttons_init();

    printf("PWM Audio System Ready (GPIO Pin 15)\n");
    printf("Button-triggered sound system initialized\n");
    printf("Buttons GPIO 0-11 connected to sounds 01-12\n\n");
    
    printf("Available sounds:\n");
    printf("Button 0 (GPIO 0)  -> Sound 01: %zu bytes\n", sound_01_size);
    printf("Button 1 (GPIO 1)  -> Sound 02: %zu bytes\n", sound_02_size);
    printf("Button 2 (GPIO 2)  -> Sound 03: %zu bytes\n", sound_03_size);
    printf("Button 3 (GPIO 3)  -> Sound 04: %zu bytes\n", sound_04_size);
    printf("Button 4 (GPIO 4)  -> Sound 05: %zu bytes\n", sound_05_size);
    printf("Button 5 (GPIO 5)  -> Sound 06: %zu bytes\n", sound_06_size);
    printf("Button 6 (GPIO 6)  -> Sound 07: %zu bytes\n", sound_07_size);
    printf("Button 7 (GPIO 7)  -> Sound 08: %zu bytes\n", sound_08_size);
    printf("Button 8 (GPIO 8)  -> Sound 09: %zu bytes\n", sound_09_size);
    printf("Button 9 (GPIO 9)  -> Sound 10: %zu bytes\n", sound_10_size);
    printf("Button 10 (GPIO 10) -> Sound 11: %zu bytes\n", sound_11_size);
    printf("Button 11 (GPIO 11) -> Sound 12: %zu bytes\n", sound_12_size);
    
    // Map sounds to buttons
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
    
    printf("\nSystem ready! Press any button to play the corresponding sound.\n");
    printf("Buttons are pulled low - connect them to 3.3V to trigger.\n\n");
    
    // Main loop - wait for interrupts (more power efficient)
    while (1) {
        __wfi(); // Wait for interrupt - CPU sleeps until button press or timer interrupt
    }

    return 0;
}
