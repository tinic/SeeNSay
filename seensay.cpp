#include "seensay.h"

#include <stdint.h>

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"

SeeNSay& SeeNSay::instance() {
    static SeeNSay seensay;
    if (!seensay.initialized) {
        seensay.initialized = true;
        seensay.init();
    }
    return seensay;
}

void SeeNSay::dma_irq_handler() {
    SeeNSay& audio = SeeNSay::instance();
    if (dma_channel_get_irq0_status(audio.dma_chan)) {
        dma_channel_acknowledge_irq0(audio.dma_chan);
        audio.stop();
    }
}

void SeeNSay::gpio_irq_handler(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL && gpio <= last_button_pin) {
        size_t bi = gpio - first_button_pin;
        SeeNSay& audio = SeeNSay::instance();
        if (!audio.is_playing()) {
            if (audio.button_sounds[bi].data && audio.button_sounds[bi].size > 0) {
                audio.button_sounds[bi].pressed = true;
            }
        }
    }
}

void SeeNSay::init() {
    btn_gpio_init();
    pwm_dma_init();
}

void SeeNSay::btn_gpio_init() {
    gpio_init(audio_off_pin);
    gpio_set_dir(audio_off_pin, GPIO_OUT);
    gpio_put(audio_off_pin, false);

    for (uint pin = first_button_pin; pin <= last_button_pin; pin++) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
        gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    }

    for (uint pin = last_button_pin + 1; pin < 29; pin++) {
        if (pin != audio_pin && pin != audio_off_pin) {
            gpio_init(pin);
            gpio_set_dir(pin, GPIO_IN);
            gpio_disable_pulls(pin);
            gpio_set_input_enabled(pin, false);
        }
    }
}

void SeeNSay::pwm_dma_init() {
    gpio_set_function(audio_pin, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(audio_pin);
    uint chan_num = pwm_gpio_to_channel(audio_pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, static_cast<float>(clock_get_hz(clk_sys)) /
                                       static_cast<float>(sample_rate * (pwm_wrap + 1)));
    pwm_config_set_wrap(&config, pwm_wrap);
    pwm_init(slice_num, &config, true);
    pwm_set_chan_level(pwm_gpio_to_slice_num(audio_pin), chan_num, 0);
    pwm_set_enabled(pwm_gpio_to_slice_num(audio_pin), true);

    dma_chan = dma_claim_unused_channel(true);
    dma_cfg = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_cfg, true);
    channel_config_set_write_increment(&dma_cfg, false);
    channel_config_set_dreq(&dma_cfg, pwm_get_dreq(slice_num));

    dma_channel_acknowledge_irq0(dma_chan);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);
}

void SeeNSay::play(const uint16_t* data, size_t size) {
    if (audio_playing) {
        return;
    }

    audio_size = size;
    audio_position = 0;
    audio_playing = true;

    uint slice_num = pwm_gpio_to_slice_num(audio_pin);
    uint chan_num = pwm_gpio_to_channel(audio_pin);
    volatile void* write_addr =
        (chan_num == PWM_CHAN_A) ? (void*)&pwm_hw->slice[slice_num].cc
                                 : (void*)&((uint16_t*)&pwm_hw->slice[slice_num].cc)[1];
    pwm_set_enabled(slice_num, true);
    dma_channel_configure(dma_chan, &dma_cfg, write_addr, data, size, true);
    dma_channel_set_irq0_enabled(dma_chan, true);

    gpio_put(audio_off_pin, true);
}

void SeeNSay::stop() {
    if (audio_playing) {
        uint32_t remaining = dma_channel_hw_addr(dma_chan)->transfer_count;
        if (remaining > audio_size) {
            audio_position = 0;
        } else {
            audio_position = static_cast<intptr_t>(audio_size - remaining);
        }
        audio_playing = false;

        dma_channel_abort(dma_chan);
        pwm_set_enabled(pwm_gpio_to_slice_num(audio_pin), false);

        gpio_put(audio_off_pin, false);
    }
}

bool SeeNSay::is_playing() const {
    return audio_playing;
}

size_t SeeNSay::get_position() const {
    if (!audio_playing) {
        return audio_position < 0 ? 0 : static_cast<size_t>(audio_position);
    }

    uint32_t remaining = dma_channel_hw_addr(dma_chan)->transfer_count;
    if (remaining > audio_size) {
        return 0;
    }
    
    return audio_size - remaining;
}

void SeeNSay::set_button_sound(uint bi, const uint16_t* data, size_t size) {
    if (bi < num_buttons) {
        button_sounds.at(bi).data = data;
        button_sounds.at(bi).size = size;
        button_sounds.at(bi).index = bi;
    }
}

void SeeNSay::update() {
    for (auto& b : button_sounds) {
        if (b.pressed) {
            b.pressed = false;
            play(b.data, b.size);
        }
    }
}
