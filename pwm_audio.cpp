#include "pwm_audio.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

static audio_player_t player = {0};
static struct repeating_timer timer;

// Button-to-sound mapping
typedef struct {
    const unsigned char* data;
    size_t size;
} button_sound_t;

static button_sound_t button_sounds[NUM_BUTTONS] = {0};

// GPIO interrupt callback for button presses
void gpio_callback(uint gpio, uint32_t events) {
    // Check if it's a rising edge (button press) on one of our button pins
    if (events & GPIO_IRQ_EDGE_RISE && gpio >= FIRST_BUTTON_PIN && gpio <= LAST_BUTTON_PIN) {
        int button_index = gpio - FIRST_BUTTON_PIN;
        
        // Play the corresponding sound if available
        if (button_sounds[button_index].data && button_sounds[button_index].size > 0) {
            pwm_audio_play(button_sounds[button_index].data, button_sounds[button_index].size);
        }
    }
}

// Timer callback function - called at sample rate frequency
bool pwm_audio_timer_callback(struct repeating_timer *t) {
    if (!player.playing || player.position >= player.size) {
        // Stop playback when we reach the end
        player.playing = false;
        return true;
    }
    
    // Get current sample and update PWM duty cycle
    uint8_t sample = player.data[player.position];
    pwm_set_gpio_level(AUDIO_PIN, sample);
    
    // Advance to next sample
    player.position++;
    
    return true; // Continue repeating
}

void pwm_audio_init(void) {
    // Initialize GPIO for PWM
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    
    // Get PWM slice number
    uint slice_num = pwm_gpio_to_slice_num(AUDIO_PIN);
    
    // Configure PWM
    pwm_config config = pwm_get_default_config();
    
    // Set PWM frequency
    // System clock is 125MHz, we want PWM frequency high enough to avoid audible artifacts
    // PWM freq = 125MHz / (wrap + 1) / div
    // For 8-bit resolution (wrap = 255), we need div = 125MHz / 256 / desired_freq
    // Let's aim for ~488kHz PWM frequency: div = 125MHz / 256 / 488kHz ≈ 1.0
    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_config_set_wrap(&config, PWM_WRAP);
    
    // Initialize PWM slice
    pwm_init(slice_num, &config, true);
    
    // Set initial PWM level to middle (127 for 8-bit)
    pwm_set_gpio_level(AUDIO_PIN, 127);
    
    // Initialize player state
    player.data = NULL;
    player.size = 0;
    player.position = 0;
    player.playing = false;
}

void pwm_audio_play(const unsigned char* data, size_t size) {
    // Stop current playback if any
    pwm_audio_stop();
    
    // Set up new playback
    player.data = data;
    player.size = size;
    player.position = 0;
    player.playing = true;
    
    // Start timer at sample rate
    // Timer period = 1 / sample_rate seconds = 1000000 / sample_rate microseconds
    int64_t period_us = 1000000 / SAMPLE_RATE;
    add_repeating_timer_us(-period_us, pwm_audio_timer_callback, NULL, &timer);
}

void pwm_audio_stop(void) {
    if (player.playing) {
        cancel_repeating_timer(&timer);
        player.playing = false;
        // Set PWM to middle level when stopped
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
    // Initialize all button GPIO pins
    for (int pin = FIRST_BUTTON_PIN; pin <= LAST_BUTTON_PIN; pin++) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_down(pin);  // Pull down since buttons connect to high
        
        // Enable interrupt on rising edge (button press)
        gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    }
}

void buttons_set_sound_data(int button_index, const unsigned char* data, size_t size) {
    if (button_index >= 0 && button_index < NUM_BUTTONS) {
        button_sounds[button_index].data = data;
        button_sounds[button_index].size = size;
    }
}