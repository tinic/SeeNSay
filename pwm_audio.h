#ifndef PWM_AUDIO_H
#define PWM_AUDIO_H

#include <stddef.h>
#include <stdint.h>
#include <array>
#include "hardware/dma.h"

class PWMAudio {
private:
    static constexpr size_t num_buttons = 12;
    static constexpr uint first_button_pin = 0;
    static constexpr uint last_button_pin = 11;

    static constexpr uint audio_pin = 15;
    static constexpr uint audio_off_pin = 16;

    static constexpr uint sample_rate = 22050;
    static constexpr uint pwm_resolution_bits = 10;
    static constexpr uint pwm_wrap = (1 << pwm_resolution_bits) + 64;
    
    struct button_sound {
        size_t index = 0;
        const uint16_t* data = nullptr;
        size_t size = 0;
        bool pressed = false;
    };

    size_t audio_size = 0;
    intptr_t audio_position = 0;
    bool audio_playing = false;

    dma_channel_config dma_cfg{};
    int dma_chan = 0;

    std::array<button_sound, num_buttons> button_sounds{};

    PWMAudio() {
        init();
    }
    ~PWMAudio() = default;

    PWMAudio(const PWMAudio&) = delete;
    PWMAudio& operator=(const PWMAudio&) = delete;

    void dma_irq_handler_impl();
    static void dma_irq_handler();
    static void gpio_callback(uint gpio, uint32_t events);

    void init();
    void btn_gpio_init();
    void pwm_dma_init();

    void play(const uint16_t* data, size_t size);
    void stop();
    bool is_playing() const;
    size_t get_position() const;

public:
    static PWMAudio& instance();
    void set_button_sound(size_t button_index, const uint16_t* data, size_t size);
    void update();
};

#endif // PWM_AUDIO_H
