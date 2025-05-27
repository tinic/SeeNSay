#ifndef PWM_AUDIO_H
#define PWM_AUDIO_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// Audio configuration
#define AUDIO_PIN 15         // GPIO pin for PWM audio output
#define AUDIO_OFF_PIN 16     // GPIO pin for controlling on-off state of amplifier
#define SAMPLE_RATE 22050    // 22kHz sample rate to match our PCM data

// Button configuration
#define NUM_BUTTONS 12       // Number of button/sound pairs
#define FIRST_BUTTON_PIN 0   // First button GPIO pin
#define LAST_BUTTON_PIN 11   // Last button GPIO pin

// Audio player state
typedef struct {
    const uint16_t* data = 0;
    size_t size = 0;
    size_t position = 0;
    bool playing = false;
    bool loop = false;
} audio_player_t;

// Initialize PWM audio system
void pwm_audio_init(void);

// Start playing a sound
void pwm_audio_play(const uint16_t* data, size_t size, bool loop = false);

// Stop audio playback
void pwm_audio_stop(void);

// Check if audio is currently playing
bool pwm_audio_is_playing(void);

// Get current playback position (in samples)
size_t pwm_audio_get_position(void);

// Button handling functions
void buttons_init(void);
void buttons_set_sound_data(int button_index, const uint16_t* data, size_t size);
void button_check(void);

#endif // PWM_AUDIO_H
