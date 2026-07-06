// ============================================================
// Custom "welcome" RGB Matrix boot animation.
//
// TO REMOVE THIS ENTIRELY: just delete this file (or comment out
// the WELCOME_ANIMATION_ACTIVE define in config.h). Nothing else
// needs to change - the whole file compiles to nothing when the
// define is missing.
// ============================================================
#ifdef WELCOME_ANIMATION_ACTIVE

RGB_MATRIX_EFFECT(welcome_animation)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// ---------------- Tunables ----------------
#define WA_SWEEP_STEP_MS    12   // ms per LED during the "one by one" sweep
#define WA_CHECKER_STEP_MS  120  // ms per checkerboard flip
#define WA_CHECKER_FLASHES  12    // number of on/off flips (3 full blinks)
#define WA_LETTER_STEP_MS   220  // ms each letter stays lit
#define WA_LETTER_GAP_MS    80   // ms of "off" between letters

// LED indices for the letters, in the order they light up.
// These come from g_led_config's "Key Matrix to LED Index" table
// in rgb.c - update if you ever change the PCB/layout.
//   A = 42   Z = 63   A = 42   R = 24
static const uint8_t wa_letters[] = {42, 63, 42, 24};
#define WA_LETTER_COUNT (sizeof(wa_letters) / sizeof(wa_letters[0]))

static uint32_t wa_start_time = 0;
static bool     wa_started    = false;

static bool welcome_animation(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    if (params->init || !wa_started) {
        wa_start_time = timer_read32();
        wa_started    = true;
        rgb_matrix_set_color_all(0, 0, 0);
    }

    uint32_t elapsed = timer_elapsed32(wa_start_time);

    uint8_t  led_count   = RGB_MATRIX_LED_COUNT;
    uint32_t sweep_dur   = (uint32_t)led_count * WA_SWEEP_STEP_MS;
    uint32_t checker_dur = (uint32_t)WA_CHECKER_FLASHES * WA_CHECKER_STEP_MS;
    uint32_t letters_dur = (uint32_t)WA_LETTER_COUNT * (WA_LETTER_STEP_MS + WA_LETTER_GAP_MS);
    uint32_t total_dur   = sweep_dur + checker_dur + letters_dur;

    HSV hsv = rgb_matrix_config.hsv;
    RGB rgb = hsv_to_rgb(hsv);

    if (elapsed < sweep_dur) {
        // ---- Phase 1: light LEDs one by one, first to last ----
        uint32_t lit_up_to = elapsed / WA_SWEEP_STEP_MS;
        for (uint8_t i = led_min; i < led_max; i++) {
            if (i <= lit_up_to) {
                rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
            } else {
                rgb_matrix_set_color(i, 0, 0, 0);
            }
        }
    } else if (elapsed < sweep_dur + checker_dur) {
        // ---- Phase 2: checkerboard flash, like an LED test ----
        uint32_t phase_elapsed = elapsed - sweep_dur;
        uint32_t flash_index   = phase_elapsed / WA_CHECKER_STEP_MS;
        bool     even_on       = (flash_index % 2) == 0;

        for (uint8_t i = led_min; i < led_max; i++) {
            bool is_even = (i % 2) == 0;
            if (is_even == even_on) {
                rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
            } else {
                rgb_matrix_set_color(i, 0, 0, 0);
            }
        }
    } else if (elapsed < total_dur) {
        // ---- Phase 3: A - Z - A - R, one at a time ----
        uint32_t phase_elapsed = elapsed - sweep_dur - checker_dur;
        uint32_t step_dur      = WA_LETTER_STEP_MS + WA_LETTER_GAP_MS;
        uint32_t letter_index  = phase_elapsed / step_dur;
        uint32_t within_step   = phase_elapsed % step_dur;

        for (uint8_t i = led_min; i < led_max; i++) {
            rgb_matrix_set_color(i, 0, 0, 0);
        }

        if (letter_index < WA_LETTER_COUNT && within_step < WA_LETTER_STEP_MS) {
            uint8_t led = wa_letters[letter_index];
            if (led >= led_min && led < led_max) {
                rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
            }
        }
    } else {
        // ---- Done: hand off to your normal effect ----
        wa_started = false;
        rgb_matrix_mode_noeeprom(WA_FOLLOWUP_MODE);
        return true;
    }

    return led_max < RGB_MATRIX_LED_COUNT;
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif // WELCOME_ANIMATION_ACTIVE














