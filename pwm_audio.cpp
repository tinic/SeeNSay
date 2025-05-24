#include "pwm_audio.h"

#include <stdint.h>

#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"

static audio_player_t player = {0};
static struct repeating_timer timer{};

typedef struct {
    const unsigned char* data = 0;
    size_t size = 0;
} button_sound_t;

static button_sound_t button_sounds[NUM_BUTTONS] = {0};

void gpio_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_RISE && gpio <= LAST_BUTTON_PIN) {
        int button_index = gpio - FIRST_BUTTON_PIN;
        if (!pwm_audio_is_playing()) {
            if (button_sounds[button_index].data && button_sounds[button_index].size > 0) {
                pwm_audio_play(button_sounds[button_index].data, button_sounds[button_index].size);
            }
        }
    }
}

bool pwm_audio_timer_callback(struct repeating_timer* /*t*/) {
    if (!player.playing || player.position >= player.size) {
        player.playing = false;
        return true;
    }

    uint8_t sample = player.data[player.position];
    pwm_set_gpio_level(AUDIO_PIN, sample);

    player.position++;

    return true;
}

void pwm_audio_init(void) {
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(AUDIO_PIN);

    pwm_config config = pwm_get_default_config();

    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_config_set_wrap(&config, PWM_WRAP);

    pwm_init(slice_num, &config, true);

    pwm_set_gpio_level(AUDIO_PIN, 127);

    player.data = NULL;
    player.size = 0;
    player.position = 0;
    player.playing = false;
}

void pwm_audio_play(const unsigned char* data, size_t size) {
    pwm_audio_stop();

    player.data = data;
    player.size = size;
    player.position = 0;
    player.playing = true;

    int64_t period_us = 1000000 / SAMPLE_RATE;
    add_repeating_timer_us(-period_us, pwm_audio_timer_callback, NULL, &timer);
}

void pwm_audio_stop(void) {
    if (player.playing) {
        cancel_repeating_timer(&timer);
        player.playing = false;
        pwm_set_gpio_level(AUDIO_PIN, 127);
    }
}

bool pwm_audio_is_playing(void) {
    return player.playing && player.position < player.size;
}

size_t pwm_audio_get_position(void) {
    return player.position;
}

void buttons_init(void) {
    for (int pin = FIRST_BUTTON_PIN; pin <= LAST_BUTTON_PIN; pin++) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_down(pin);
        gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    }
}

void buttons_set_sound_data(int button_index, const unsigned char* data, size_t size) {
    if (button_index >= 0 && button_index < NUM_BUTTONS) {
        button_sounds[button_index].data = data;
        button_sounds[button_index].size = size;
    }
}
