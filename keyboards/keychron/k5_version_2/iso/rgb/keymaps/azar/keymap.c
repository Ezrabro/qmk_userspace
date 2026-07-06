/* Copyright 2025 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include "features/calculator.h"

const uint8_t calc_bit_leds[] = {
21,  22,  23,  24,  25,  26,  27,  28,  29,  30,
};

enum layers {
    BASE,   // Hardware "Mac" side  → QWERTY (Windows)
    COLEMAK_BASE,   // Hardware "Win" side  → Colemak-DH Wide ISO
    FN,
};

enum custom_keycodes {
    CALC_TOGG = SAFE_RANGE,
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    // -------------------------------------------------------------------------
    // MAC_BASE = QWERTY Windows (physical switch on "Mac" side)
    // -------------------------------------------------------------------------
    [BASE] = LAYOUT_109_iso(
        KC_ESC,             KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,  KC_F13,  KC_F14, KC_F15,  KC_F16,   KC_F17,   KC_F18,   KC_F19,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC, KC_F20,   KC_F21,  KC_F22,  KC_NUM,   KC_PSLS,  KC_PAST,  KC_PMNS,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,           KC_DEL,   KC_HOME,   KC_END,  KC_P7,    KC_P8,    KC_P9,    KC_PPLS,
        LT(0, KC_ENT),   KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_NUHS,  KC_ENT,                                KC_P4,    KC_P5,    KC_P6,
        KC_LSFT,  KC_NUBS,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,           KC_UP,              KC_P1,    KC_P2,    KC_P3,    KC_PENT,
        KC_LCTL,  KC_LWIN,  KC_LALT,                                KC_SPC,                                 KC_RALT,  KC_RWIN, MO(FN),KC_RCTL, KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_P0,              KC_PDOT          ),

    // -------------------------------------------------------------------------
    // COLEMAK_BASE = Colemak-DH Wide ISO (physical switch on "Win" side)
    //
    // Layout from image (alpha keys only, rest unchanged):
    //   Tab row:  Q  W  F  P  B  [  J  L  U  Y  ;  /
    //   Home row: A  R  S  T  G  ]  M  N  E  I  O  '
    //   Bot row:  Z  X  C  D  V  \  #  K  H  ,  .
    //
    // ISO note: KC_NUHS = # (the key between left shift and Z on ISO)
    //           KC_NUBS = \ (the key left of Z, or the extra ISO key)
    // -------------------------------------------------------------------------
    [COLEMAK_BASE] = LAYOUT_109_iso(
        KC_ESC,             KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,  KC_F13,  KC_F14, KC_F15,  KC_F16,   KC_F17,   KC_F18,   KC_F19,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC, KC_F20,   KC_F21,  KC_F22,  KC_NUM,   KC_PSLS,  KC_PAST,  KC_PMNS,
        KC_TAB,   KC_Q,     KC_W,     KC_F,     KC_P,     KC_B,     KC_LBRC,  KC_J,     KC_L,     KC_U,     KC_Y,     KC_SCLN,  KC_SLSH,           KC_DEL,   KC_HOME,   KC_END,  KC_P7,    KC_P8,    KC_P9,    KC_PPLS,
        LT(0, KC_ENT),  KC_A,     KC_R,     KC_S,     KC_T,     KC_G,     KC_RBRC,  KC_M,     KC_N,     KC_E,     KC_I,  KC_O,  KC_QUOT,  KC_ENT,                                KC_P4,    KC_P5,    KC_P6,
        KC_LSFT,  KC_Z,     KC_X,     KC_C,     KC_D,     KC_V,     KC_NUBS,  KC_NUHS,  KC_K,     KC_H,     KC_COMM,  KC_DOT,       KC_RSFT,           KC_UP,              KC_P1,    KC_P2,    KC_P3,    KC_PENT,
        KC_LCTL,  KC_LWIN,  KC_LALT,                                KC_SPC,                                 KC_RALT,  KC_RWIN, MO(FN),KC_RCTL, KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_P0,              KC_PDOT          ),


    // -------------------------------------------------------------------------
    // FN = cool Fn layer 
    // -------------------------------------------------------------------------
    [FN] = LAYOUT_109_iso(
        QK_BOOT,            KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  UG_VALD,  UG_VALU,  KC_MPRV,  KC_MNXT,  KC_MPLY,  KC_MUTE,  KC_VOLD,  KC_VOLU, _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  BT_HST1,  BT_HST2,  BT_HST3,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______, _______,  _______,  _______,  CALC_TOGG,  _______,  _______,  _______,
        UG_TOGG,  UG_NEXT,  UG_VALU,  UG_HUEU,  UG_SATU,  UG_SPDU,  _______,  _______,  _______,  _______,  _______,  _______,  _______,           _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______,  UG_PREV,  UG_VALD,  UG_HUED,  UG_SATD,  UG_SPDD,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                               _______,  _______,  _______,
        _______,  _______,  _______,  _______,  _______,  _______,  BAT_LVL,  _______,  _______,  _______,  _______,  _______,            _______,           _______,            _______,  _______,  _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,  _______, _______,  _______,  _______,  _______,            _______           ),

};

// clang-format on
bool backspace_mode = false;
uint16_t backspace_timer = 0;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // calc
    if (keycode == CALC_TOGG) {
        if (record->event.pressed) calculator_toggle();
        return false;
    }

    if (calculator_is_active()) {
        return calculator_process_record(keycode, record);
    }

    switch (keycode) {
        case LT(0, KC_ENT):

            // If backspace mode timed out, disable it
            if (backspace_mode &&
                timer_elapsed(backspace_timer) > 1000) {
                backspace_mode = false;
            }

            // Tap
            if (record->tap.count && record->event.pressed) {

                if (backspace_mode) {
                    tap_code(KC_BSPC);
                    backspace_timer = timer_read(); // Reset timeout
                    return false;
                }

                return true; // Normal Enter behavior
            }

            // Hold
            if (!record->tap.count) {

                if (record->event.pressed) {
                    register_code(KC_BSPC);

                    // Enable backspace mode
                    backspace_mode = true;
                    backspace_timer = timer_read();
                } else {
                    unregister_code(KC_BSPC);
                }

                return false;
            }

            break;

    }
    if (!process_record_keychron_common(keycode, record)) {
        return false;
    }
    return true;
}


bool rgb_matrix_indicators_user(void) {
    if (calculator_is_active()) {
        calculator_update_rgb();
        return false;
    }
    if (host_keyboard_led_state().caps_lock || is_caps_word_on()) {
        rgb_matrix_set_color(CAPS_LOCK_INDEX, 255, 255, 255);
    }

    return true;
}


bool dip_switch_update_kb(uint8_t index, bool active) {
    if (index == 0) {
        default_layer_set(1UL << (active ? BASE : COLEMAK_BASE));
    }
    return dip_switch_update_user(index, active);
}

void keyboard_post_init_user(void) {
    calculator_init(calc_bit_leds, ARRAY_SIZE(calc_bit_leds), 20, 37);
}

