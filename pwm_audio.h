#ifndef PWM_AUDIO_H
#define PWM_AUDIO_H

#include <stddef.h>
#include <stdbool.h>

// Audio configuration
#define AUDIO_PIN 15         // GPIO pin for PWM audio output (changed from 0 to avoid button conflict)
#define SAMPLE_RATE 22050    // 22kHz sample rate to match our PCM data
#define PWM_WRAP 255         // 8-bit PWM resolution

// Button configuration
#define NUM_BUTTONS 12       // Number of button/sound pairs
#define FIRST_BUTTON_PIN 0   // First button GPIO pin
#define LAST_BUTTON_PIN 11   // Last button GPIO pin

// Audio player state
typedef struct {
    const unsigned char* data;
    size_t size;
    size_t position;
    bool playing;
} audio_player_t;

// Initialize PWM audio system
void pwm_audio_init(void);

// Start playing a sound
void pwm_audio_play(const unsigned char* data, size_t size);

// Stop audio playback
void pwm_audio_stop(void);

// Check if audio is currently playing
bool pwm_audio_is_playing(void);

// Get current playback position (in samples)
size_t pwm_audio_get_position(void);

// Button handling functions
void buttons_init(void);
void buttons_set_sound_data(int button_index, const unsigned char* data, size_t size);

#endif // PWM_AUDIO_H