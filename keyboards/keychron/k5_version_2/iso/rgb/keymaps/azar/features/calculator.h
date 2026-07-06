#pragma once
#include "quantum.h"

// Call once at startup with the LED indices that will represent bit0, bit1, bit2...
// sign_led = a dedicated LED to show negative numbers, or 255 if you don't want one.
void calculator_init(const uint8_t *led_indices, uint8_t num_leds, uint8_t sign_led, uint8_t numlock_led);

void calculator_toggle(void);
bool calculator_is_active(void);

// Return true to let QMK process the key normally, false if the calculator consumed it.
bool calculator_process_record(uint16_t keycode, keyrecord_t *record);

// Call every frame from rgb_matrix_indicators_user() while calculator_is_active()
void calculator_update_rgb(void);