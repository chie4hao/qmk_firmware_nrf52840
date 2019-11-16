/* Copyright 2015-2017 Jack Humbert
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
#include "muse.h"
#include "ble_service.h"
#include "rgb_matrix.h"
#include "i2c_master.h"

// extern keymap_config_t keymap_config;
extern rgb_config_t rgb_matrix_config;

enum qs75_layers {
    _DEFAULT,
    _OTHER,
    _SLASH,
    _SCOLON,
    _SPACE,
    _MOUSE,
    _BLUE
};

enum planck_keycodes { DISC = SAFE_RANGE, ADVW, ADVS, SEL0, SEL1, SEL2, DELB, SLEEP, REBOOT };

#define LCTLESC MT(MOD_LCTL, KC_ESC)
#define LTPSC LT(_SPACE, KC_SPC)
#define MOUSEL MO(_MOUSE)
#define OTHERL TG(_OTHER)
#define SCOLONL LT(_SCOLON, KC_SCLN)
#define SLASHL LT(_SLASH, KC_SLSH)
#define CLTQUOT MT(MOD_RCTL, KC_QUOT)
#define BLUETOG MO(_BLUE)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_DEFAULT] = LAYOUT(
        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_RCBR,
        LCTLESC, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, SCOLONL, CLTQUOT,
        KC_LCBR, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, SLASHL, KC_ENTER,
        KC_CAPS, KC_NLCK, RGB_MOD, KC_LALT, KC_LGUI, KC_LSFT, LTPSC, MOUSEL, BLUETOG, KC_RGUI, KC_RALT, OTHERL

                        // SLEEP,LT(_OTHER, KC_W), OUT_USB, OUT_BT, ADVW, KC_E, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L,
                        // KC_A, SEL0, KC_C, KC_D, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L,
                        // KC_A, KC_B, KC_C, KC_D, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L,
                        // KC_A, KC_B, KC_C, KC_D, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L
                        ),
    [_OTHER]   = LAYOUT(
        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_UP, KC_O, KC_P, KC_RCBR,
        KC_ESC, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_LEFT, KC_DOWN, KC_RIGHT, KC_SCLN, CLTQUOT,
        KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, SLASHL, KC_ENTER,
        KC_CAPS, KC_NLCK, RGB_MOD, KC_LALT, KC_LGUI, KC_LSFT, KC_SPC, MOUSEL, BLUETOG, KC_RGUI, KC_RALT, OTHERL
                    //   SEL1, KC_NO,DISC, ADVS, RESET, DELB, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L,
                    //   KC_A, SEL2, KC_C, KC_D, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L,
                    //   KC_A, KC_B, KC_C, KC_D, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L,
                    //   KC_A, KC_B, KC_C, KC_D, KC_E, KC_F, KC_G, KC_H, KC_I, KC_J, KC_K, KC_L
                      ),
    [_SLASH]   = LAYOUT(
        _______, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, _______, _______,
        _______, KC_MPRV, KC_MPLY, KC_MNXT, KC_MSTP, KC_F10, _______, KC_APP, _______, _______, _______, _______,
        _______, KC_VOLD, KC_VOLU, KC_MUTE, KC_F11, KC_F12, KC_INS, KC_PSCR, KC_SLCK, KC_PAUS, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______),
    [_SCOLON] = LAYOUT(
        _______, _______, _______, _______, _______, _______, _______, KC_BSPC, KC_DEL, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, _______, _______,
        _______, _______, _______, _______, _______, _______, KC_HOME, KC_PGDN, KC_PGUP, KC_END, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______

                       ),
    [_SPACE]  = LAYOUT(
        KC_GRV, KC_EXLM, KC_AT, KC_HASH, KC_DLR, KC_PERC, KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, _______,
        _______, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_BSLS,
        _______, KC_TILD, KC_PIPE, KC_LBRC, KC_RBRC, KC_UNDS, KC_PLUS, KC_MINS, KC_EQL, KC_DOT, KC_SLSH, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______),

    [_MOUSE] = LAYOUT(
        _______, RGB_TOG, RGB_MOD, RGB_HUI, RGB_HUD, RGB_SAI, _______, KC_WH_D, KC_MS_U, KC_WH_U, _______, _______,
        _______, RGB_VAI, RGB_VAD, RGB_SPI, RGB_SPD, _______, KC_BTN2, KC_MS_L, KC_MS_D, KC_MS_R, KC_BTN1, _______,
        _______, _______, _______, _______, _______, _______, KC_HOME, KC_ACL0, KC_ACL1, KC_ACL2, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______),

    [_BLUE] = LAYOUT(
        RESET,   OUT_USB, OUT_BT,  ADVS,    ADVW,    REBOOT,  _______, _______, _______, _______, _______, _______,
        SEL0,    SEL1,    SEL2,    DISC,    DELB   , SLEEP,   _______, _______, _______, _______, _______, _______,
        MAGIC_TOGGLE_NKRO, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
    )
};

void keyboard_post_init_user() {
    #ifdef IS31FL3737
    // Shutdown IS31FL3737 if rgb disabled
    if (!rgb_matrix_config.enable) {
        i2c_stop();
    }
    #endif
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    #ifdef POWER_SAVE_TIMEOUT
        if (record->event.pressed) {
            reset_power_save_counter();
        }
    #endif
    switch (keycode) {
        case DISC:
            if (record->event.pressed) {
                ble_disconnect();
            }
            return false;

        case ADVW:
            if (record->event.pressed) {
                advertising_without_whitelist();
            }
            return false;

        case ADVS:
            if (record->event.pressed) {
                advertising_start(false);
            }
            return false;

        case SEL0:
            if (record->event.pressed) {
                restart_advertising_id(0);
            }
            return false;

        case SEL1:
            if (record->event.pressed) {
                restart_advertising_id(1);
            }
            return false;

        case SEL2:
            if (record->event.pressed) {
                restart_advertising_id(2);
            }
            return false;

        case DELB:
            if (record->event.pressed) {
                advertising_start(true);
            }
            return false;

        case REBOOT:
            if (record->event.pressed) {
                NVIC_SystemReset();
            }

        case RGB_TOG:
            if (record->event.pressed) {
                if (rgb_matrix_config.enable) {
                    i2c_stop();
                } else {
                    i2c_start();
                }
            }
            return true;

        case SLEEP:
            if (!record->event.pressed) {
                deep_sleep_mode_enter();
            }
            return false;
    }
    return true;
}
