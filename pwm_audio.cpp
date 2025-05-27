#include "pwm_audio.h"

#include <stdint.h>

#include <cstdio>

#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"

static audio_player_t player{};
static dma_channel_config dma_cfg{};
static int dma_chan = 0;

typedef struct {
    size_t index = 0;
    const uint16_t* data = 0;
    size_t size = 0;
    bool pressed = false;
} button_sound_t;

static button_sound_t button_sounds[NUM_BUTTONS] = {0};

static void dma_irq_handler() {
    if (dma_channel_get_irq0_status(dma_chan)) {
        dma_channel_acknowledge_irq0(dma_chan);
        pwm_audio_stop();
    }
}

void gpio_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL && gpio <= LAST_BUTTON_PIN) {
        int button_index = gpio - FIRST_BUTTON_PIN;
        if (!pwm_audio_is_playing()) {
            if (button_sounds[button_index].data && button_sounds[button_index].size > 0) {
                button_sounds[button_index].pressed = true;
            }
        }
    }
}

void pwm_audio_init(void) {
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(AUDIO_PIN);
    uint chan_num = pwm_gpio_to_channel(AUDIO_PIN);
    pwm_config config = pwm_get_default_config();
    // For 22kHz with 16-bit resolution: need faster PWM clock
    // 125MHz / (2839 wrap * 22050Hz) = ~2.0 divider  
    pwm_config_set_clkdiv(&config, 2.0f);
    pwm_config_set_wrap(&config, 2838);
    pwm_init(slice_num, &config, true);
    pwm_set_chan_level(pwm_gpio_to_slice_num(AUDIO_PIN), chan_num, 0);
    pwm_set_enabled(pwm_gpio_to_slice_num(AUDIO_PIN), true);

    // --------------------------------

    dma_chan = dma_claim_unused_channel(true);
    dma_cfg = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_cfg, true);
    channel_config_set_write_increment(&dma_cfg, false);
    channel_config_set_dreq(&dma_cfg, pwm_get_dreq(slice_num));

    // --------------------------------

    player.data = nullptr;
    player.size = 0;
    player.position = 0;
    player.playing = false;
    player.loop = false;

    // --------------------------------

    dma_channel_acknowledge_irq0(dma_chan);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);
}

void pwm_audio_play(const uint16_t* data, size_t size, bool loop) {
    if (player.playing) {
        return;
    }

    printf("pwm_audio_play\n");
    player.data = data;
    player.size = size;
    player.position = 0;
    player.playing = true;
    player.loop = loop;

    // --------------------------------

    uint slice_num = pwm_gpio_to_slice_num(AUDIO_PIN);

    uint chan_num = pwm_gpio_to_channel(AUDIO_PIN);
    volatile void* write_addr = (chan_num == PWM_CHAN_A) ? (void*)&pwm_hw->slice[slice_num].cc
                                                         : (void*)&((uint16_t*)&pwm_hw->slice[slice_num].cc)[1];

    printf("slice_num %d chan_num %d\n", slice_num, chan_num);
    
    dma_channel_configure(dma_chan, &dma_cfg,
                          write_addr,     // Write address
                          player.data,    // Read address (preprocessed PWM data)
                          player.size,    // Number of 16-bit samples
                          true);          // Start now

    dma_channel_set_irq0_enabled(dma_chan, true);
    gpio_put(AUDIO_OFF_PIN, true);
}

void pwm_audio_stop(void) {
    if (player.playing) {
        player.playing = false;
        // --------------------------------
        dma_channel_abort(dma_chan);
        gpio_put(AUDIO_OFF_PIN, false);
    }
}

bool pwm_audio_is_playing(void) {
    return player.playing;
}

size_t pwm_audio_get_position(void) {
    return player.position;
}

void buttons_init(void) {
    gpio_init(AUDIO_OFF_PIN);
    gpio_set_dir(AUDIO_OFF_PIN, GPIO_OUT);
    gpio_put(AUDIO_OFF_PIN, false);

    for (int pin = FIRST_BUTTON_PIN; pin <= LAST_BUTTON_PIN; pin++) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
        gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    }
}

void buttons_set_sound_data(int button_index, const uint16_t* data, size_t size) {
    if (button_index >= 0 && button_index < NUM_BUTTONS) {
        button_sounds[button_index].data = data;
        button_sounds[button_index].size = size;
        button_sounds[button_index].index = button_index;
    }
}

void button_check() {
    for (auto& b : button_sounds) {
        if (b.pressed) {
            b.pressed = false;
            printf("Playind sound %d!\n", b.index);
            pwm_audio_play(b.data, b.size);
        }
    }
}