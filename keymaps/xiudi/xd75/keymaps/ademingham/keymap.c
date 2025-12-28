/* Copyright 2017 Wunder
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
#include "rgblight.h"

static uint16_t play_next_timer = 0;
static uint16_t mute_prev_timer = 0;

#define _BASE 0
#define _FN 1
#define _WINLOCK 2
#define _NUMPAD 3
#define _SYSTEM 4

// System Layer Colour
#define SYSTEM_H 43
#define SYSTEM_S 255
#define SYSTEM_V 255

static uint8_t saved_h, saved_s, saved_v;

// Underglow state change logic
static bool system_layer_active = false;
static bool system_colour_dirty = false;

enum custom_keycodes {
    LAYER0 = SAFE_RANGE,
    LAYER1,
    LAYER2,
    LAYER3,
    LAYER4,
    TAP_HOLD_PLAY_NEXT,
    TAP_HOLD_MUTE_PREV,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* QWERTY
     * .--------------------------------------------------------------------------------------------------------------------------------------.
     * | ESC    | 1      | 2      | 3      | 4      | 5      | 6      | 7      | 8      | 9      | 0      | BACKSP | `      | -      | =      |
     * |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+-----------------|
     * | TAB    | Q      | W      | E      | R      | T      | Y      | U      | I      | O      | P      | '      | Num7   | Num8   | Num9   |
     * |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+-----------------+--------|
     * | CAP LK | A      | S      | D      | F      | G      | H      | J      | K      | L      | ;      | ENTER  | Num4   | Num5   | Num6   |
     * |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------------------------+--------|
     * | LSHIFT | Z      | X      | C      | V      | B      | N      | M      | ,      | .      | /      | RSHIFT | Num1   | Num2   | Num3   |
     * |--------+--------+--------+--------+--------+-----------------+--------+--------+--------+--------+-----------------+--------+--------|
     * | LCTRL  | LGUI   | LALT   | SPACE           | SPACE           | FN     | MENU   | RALT   | WinLock| RCTRL  | Num0   | NumDot | NumEnt |
     * '--------------------------------------------------------------------------------------------------------------------------------------'
    */

    [_BASE] = LAYOUT(
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_BSPC, KC_GRV, KC_MINS, KC_EQL,
        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_QUOT, KC_P7, KC_P8, KC_P9,
        KC_CAPS, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_ENT, KC_P4, KC_P5, KC_P6,
        KC_LEFT_SHIFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, KC_P1, KC_P2, KC_P3,
        KC_LCTL, KC_LGUI, KC_LALT, KC_SPC, KC_NO, KC_SPC, KC_NO, MO(1), KC_APP, KC_RALT, TG(2), TG(3), KC_P0, KC_PDOT, KC_PENT
    ),

    [_FN] = LAYOUT(
        KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, KC_PSCR, KC_SCRL, KC_PAUS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_INS, KC_HOME, KC_PGUP,
        KC_TRNS, KC_LCBR, KC_RCBR, KC_LPRN, KC_RPRN, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_TRNS, KC_DEL, KC_END, KC_PGDN,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_LBRC, KC_RBRC, KC_TRNS, KC_TRNS, TAP_HOLD_MUTE_PREV, KC_VOLD, KC_VOLU, TAP_HOLD_PLAY_NEXT, KC_TRNS, KC_NO, KC_UP, KC_NO,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, TG(4), KC_TRNS, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN, KC_RGHT
    ),

    [_WINLOCK] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_NO, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),

    [_NUMPAD] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_PSCR, KC_SCRL, KC_PAUS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_INS, KC_HOME, KC_PGUP,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_DEL, KC_END, KC_PGDN,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO, KC_UP, KC_NO,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN, KC_RGHT
    ),

    [_SYSTEM] = LAYOUT(
        QK_RBT, QK_BOOT, EE_CLR, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, UG_VALU, UG_SATU, UG_HUEU, KC_NO, KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, UG_PREV, LM_SPDD, UG_SPDU, UG_NEXT, UG_TOGG, KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, UG_VALD, UG_SATD, UG_HUED, KC_NO, KC_NO, KC_NO, KC_NO,
        TO(0),KC_NO,  KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, TG(4), KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO
    )
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    // Detect underglow changes while in SYSTEM layer
    if (system_layer_active && record->event.pressed) {
        switch (keycode) {
            case UG_VALU:
            case UG_VALD:
            case UG_SATU:
            case UG_SATD:
            case UG_HUEU:
            case UG_HUED:
            case UG_TOGG:
            case UG_NEXT:
            case UG_PREV:
            case UG_SPDU:
            case LM_SPDD:
                system_colour_dirty = true;
                break;
        }
    }

    switch (keycode) {

        case TAP_HOLD_PLAY_NEXT:
            if (record->event.pressed) {
                play_next_timer = timer_read();
            } else {
                if (timer_elapsed(play_next_timer) < 200) {
                    tap_code(KC_MPLY);
                } else {
                    tap_code(KC_MNXT);
                }
            }
            return false;

        case TAP_HOLD_MUTE_PREV:
            if (record->event.pressed) {
                mute_prev_timer = timer_read();
            } else {
                if (timer_elapsed(mute_prev_timer) < 200) {
                    tap_code(KC_MUTE);
                } else {
                    tap_code(KC_MPRV);
                }
            }
            return false;

        case QK_BOOT:
        case QK_RBT:
            rgblight_sethsv(0, 255, 255);  // red before reset
            return true;
    }

    return true;
}

layer_state_t layer_state_set_user(layer_state_t state) {
    bool system_now = IS_LAYER_ON_STATE(state, _SYSTEM);

    // Entering SYSTEM
    if (system_now && !system_layer_active) {
        system_layer_active = true;
        system_colour_dirty = false;

        // Save the current base colour BEFORE overriding it
        saved_h = rgblight_get_hue();
        saved_s = rgblight_get_sat();
        saved_v = rgblight_get_val();

        // Apply SYSTEM colour
        rgblight_sethsv(SYSTEM_H, SYSTEM_S, SYSTEM_V);
    }

    // Exiting SYSTEM
    else if (!system_now && system_layer_active) {
        system_layer_active = false;

        if (system_colour_dirty) {
            // User changed RGB → new base colour is whatever is active now
            saved_h = rgblight_get_hue();
            saved_s = rgblight_get_sat();
            saved_v = rgblight_get_val();
        }

        // Restore base colour
        rgblight_sethsv(saved_h, saved_s, saved_v);
    }

    return state;
}
