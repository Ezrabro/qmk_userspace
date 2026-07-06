#include "calculator.h"
#include <limits.h>

static const uint8_t *calc_leds   = NULL;
static uint8_t        calc_nleds  = 0;
static uint8_t        calc_sign   = 255;
static uint8_t calc_numlock_led = 255;

static bool active       = false;
static long accumulator  = 0;
static long current_num  = 0;
static bool negative     = false;
static char pending_op   = 0;     // '+', '-', '*', '/', or 0
static bool show_result  = false;
static bool overflow     = false;

void calculator_init(const uint8_t *led_indices, uint8_t num_leds, uint8_t sign_led, uint8_t numlock_led) {
    calc_leds        = led_indices;
    calc_nleds       = num_leds;
    calc_sign        = sign_led;
    calc_numlock_led = numlock_led;
}

bool calculator_is_active(void) { return active; }

static void calculator_reset(void) {
    accumulator = 0;
    current_num = 0;
    negative    = false;
    pending_op  = 0;
    show_result = false;
    overflow    = false;
}

void calculator_toggle(void) {
    active = !active;
    calculator_reset();
}

static void apply_pending(void) {
    long value = negative ? -current_num : current_num;
    switch (pending_op) {
        case 0:   accumulator = value; break;
        case '+': accumulator += value; break;
        case '-': accumulator -= value; break;
        case '*': accumulator *= value; break;
        case '/':
            if (value == 0) overflow = true;
            else accumulator /= value;
            break;
    }
    current_num = 0;
    negative    = false;
}

bool calculator_process_record(uint16_t keycode, keyrecord_t *record) {
    if (!active) return true;

    // Swallow key-up events for everything we handle on key-down
    if (!record->event.pressed) {
        switch (keycode) {
            case KC_P1 ... KC_P0:
            case KC_PPLS: case KC_PMNS: case KC_PAST: case KC_PSLS:
            case KC_PEQL: case KC_PENT: case KC_PDOT: case KC_BSPC:
                return false;
            default:
                return true;
        }
    }

    switch (keycode) {
        case KC_P1 ... KC_P0: { // KC_P1..KC_P9 then KC_P0, contiguous in QMK
            uint8_t digit = (keycode == KC_P0) ? 0 : (keycode - KC_P1 + 1);
            if (show_result) calculator_reset(); // digit after '=' starts fresh
            if (current_num <= (LONG_MAX - digit) / 10) {
                current_num = current_num * 10 + digit;
            } else {
                overflow = true;
            }
            return false;
        }

        case KC_PDOT: // repurposed as +/- toggle (no decimal support)
            negative = !negative;
            return false;

        case KC_BSPC:
            current_num /= 10;
            return false;

        case KC_PPLS: case KC_PMNS: case KC_PAST: case KC_PSLS:
            apply_pending();
            pending_op  = (keycode == KC_PPLS) ? '+' :
                          (keycode == KC_PMNS) ? '-' :
                          (keycode == KC_PAST) ? '*' : '/';
            show_result = false;
            return false;

        case KC_PEQL: case KC_PENT:
            apply_pending();
            pending_op  = 0;
            show_result = true;
            return false;

        default:
            return true; // anything else on the layer behaves normally
    }
}

void calculator_update_rgb(void) {
    if (!active || calc_leds == NULL) return;

    rgb_matrix_set_color_all(RGB_OFF);

    bool blink = (timer_read() / 250) % 2; // 4Hz blink, shared by numlock + overflow

    // Numlock indicator: always blinks yellow while calc mode is on
    if (calc_numlock_led != 255 && blink) {
        rgb_matrix_set_color(calc_numlock_led, 255, 255, 0);
    }

    long value = show_result ? accumulator : (negative ? -current_num : current_num);
    bool neg = value < 0;
    unsigned long uval = neg ? (unsigned long)(-value) : (unsigned long)value;

    if (overflow || uval >= (1UL << calc_nleds)) {
        if (blink) {
            for (uint8_t i = 0; i < calc_nleds; i++) rgb_matrix_set_color(calc_leds[i], RGB_RED);
        }
        return;
    }

    for (uint8_t i = 0; i < calc_nleds; i++) {
        bool bit_on = (uval >> i) & 1;
        if (bit_on) {
            rgb_matrix_set_color(calc_leds[i], 0, show_result ? 120 : 255, show_result ? 255 : 0);
        }
    }

    if (calc_sign != 255 && neg) {
        rgb_matrix_set_color(calc_sign, RGB_RED);
    }
}