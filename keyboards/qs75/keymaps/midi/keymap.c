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

extern keymap_config_t keymap_config;

enum planck_layers {
  _DEFAULT,
  _OTHER,
  _SLASH,
  _SCOLON,
  _SPACE,
  _MOUSE,
};

enum planck_keycodes {
  QWERTY = SAFE_RANGE,
  COLEMAK,
  DVORAK,
  PLOVER,
  BACKLIT,
  EXT_PLV
};

#define LCTLESC MT(MOD_LCTL, KC_ESC)
#define LTPSC LT(_SPACE, KC_SPC)
#define MOUSEL MO(_MOUSE)
#define OTHERL TG(_OTHER)
#define SCOLONL LT(_SCOLON, KC_SCLN)
#define SLASHL LT(_SLASH, KC_SLSH)
#define CLTQUOT MT(MOD_RCTL, KC_QUOT)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

/* Qwerty
 * ,-----------------------------------------------------------------------------------.
 * | Tab  |   Q  |   W  |   E  |   R  |   T  |   Y  |   U  |   I  |   O  |   P  | Bksp |
 * |------+------+------+------+------+-------------+------+------+------+------+------|
 * | Esc  |   A  |   S  |   D  |   F  |   G  |   H  |   J  |   K  |   L  |   ;  |  "   |
 * |------+------+------+------+------+------|------+------+------+------+------+------|
 * | Shift|   Z  |   X  |   C  |   V  |   B  |   N  |   M  |   ,  |   .  |   /  |Enter |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | Brite| Ctrl | Alt  | GUI  |Lower |    Space    |Raise | Left | Down |  Up  |Right |
 * `-----------------------------------------------------------------------------------'
 */
[_DEFAULT] = LAYOUT_ortho_5x15(
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_PSLS, KC_PAST, KC_PMNS, KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,
    KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_P7,   KC_P8,   KC_P9,   KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_RCBR,
    LCTLESC, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_P4,   KC_P5,   KC_P6,   KC_H,    KC_J,    KC_K ,   KC_L,    SCOLONL, CLTQUOT,
    KC_LCBR, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_P1,   KC_P2,   KC_P3,   KC_N,    KC_M,    KC_COMM, KC_DOT,  SLASHL,  KC_SFTENT,
    KC_CAPS, KC_NLCK, RGB_MOD, KC_LALT, KC_LGUI, KC_LSFT, MU_MOD,  MU_TOG,  KC_SPC,  LTPSC,   MOUSEL,  CK_TOGG, KC_RGUI, KC_RALT, OTHERL
),

[_OTHER] = LAYOUT_ortho_5x15(
    MI_SUS , MI_G,    MI_B,    MI_Ds_1, MI_G_1,  MI_B_1,  MI_Ds_2, MI_G_2,  MI_B_2,  MI_Ds_3, MI_G_3,  MI_B_3,  MI_Ds_4, MI_G_4,  MI_B_4,
    MI_OCTU, MI_Fs,   MI_As,   MI_D_1,  MI_Fs_1, MI_As_1, MI_D_2,  MI_Fs_2, MI_As_2, MI_D_3,  MI_Fs_3, MI_As_3, MI_D_4,  MI_Fs_4, MI_As_4,
    MI_OCTD, MI_F,    MI_A,    MI_Cs_1, MI_F_1,  MI_A_1,  MI_Cs_2, MI_F_2,  MI_A_2,  MI_Cs_3, MI_F_3,  MI_A_3,  MI_Cs_4,  MI_F_4,  MI_A_4,
    MI_VELU, MI_E,    MI_Gs,   MI_C_1,  MI_E_1,  MI_Gs_1, MI_C_2,  MI_E_2,  MI_Gs_2, MI_C_3,  MI_E_3,  MI_Gs_3, MI_C_4,  MI_E_4,  MI_Gs_4,
    MI_VELD, MI_Ds,   MI_G,    MI_B,    MI_Ds_1, MI_G_1,  MI_B_1,  MI_Ds_2, MI_G_2,  MI_B_2,  MI_Ds_3, MI_G_3,  MI_B_3,  MI_Ds_4, OTHERL
),

[_SLASH] = LAYOUT_ortho_5x15(
                              RESET,   DEBUG,   _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
                              _______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   _______, _______, _______, KC_F6,   KC_F7,   KC_F8,   KC_F9,   _______, _______,
                              _______, KC_MPRV, KC_MPLY, KC_MNXT, KC_MSTP, KC_F10,  _______, _______, _______, _______, KC_APP,  _______, _______, _______, _______,
                              _______, KC_VOLD, KC_VOLU, KC_MUTE, KC_F11,  KC_F12,  _______, _______, _______, KC_INS,  KC_PSCR, KC_SLCK, KC_PAUS, _______, _______,
                              _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
                              ),

[_SCOLON] = LAYOUT_ortho_5x15(
                              _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
                              _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_BSPC, KC_DEL,  _______, _______, _______,
                              _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, _______, _______,
                              _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_HOME, KC_PGDN, KC_PGUP, KC_END,  _______, _______,
                              _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
                              ),

[_SPACE] = LAYOUT_ortho_5x15(
                               _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
                               KC_GRV,  KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC, _______, _______, _______, KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, _______,
                               _______, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    _______, _______, _______, KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSLS,
                               _______, KC_TILD, KC_PIPE, KC_LBRC, KC_RBRC, KC_UNDS, _______, _______, _______, KC_PLUS, KC_MINS, KC_EQL,  KC_DOT,  KC_SLSH, _______,
                               _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
                               ),

[_MOUSE] = LAYOUT_ortho_5x15(
                               _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
                               _______, RGB_TOG, RGB_MOD, RGB_HUI, RGB_HUD, RGB_SAI, RGB_SAD, _______, _______, _______, KC_WH_D, KC_MS_U, KC_WH_U, _______, _______,
                               _______, RGB_VAI, RGB_VAD, RGB_SPI, RGB_SPD, _______, _______, _______, _______, KC_BTN2, KC_MS_L, KC_MS_D, KC_MS_R, KC_BTN1, _______,
                               _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_HOME, KC_ACL0, KC_ACL1, KC_ACL2, _______, _______,
                               _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
                               )

/* Colemak
 * ,-----------------------------------------------------------------------------------.
 * | Tab  |   Q  |   W  |   F  |   P  |   G  |   J  |   L  |   U  |   Y  |   ;  | Bksp |
 * |------+------+------+------+------+-------------+------+------+------+------+------|
 * | Esc  |   A  |   R  |   S  |   T  |   D  |   H  |   N  |   E  |   I  |   O  |  "   |
 * |------+------+------+------+------+------|------+------+------+------+------+------|
 * | Shift|   Z  |   X  |   C  |   V  |   B  |   K  |   M  |   ,  |   .  |   /  |Enter |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | Brite| Ctrl | Alt  | GUI  |Lower |    Space    |Raise | Left | Down |  Up  |Right |
 * `-----------------------------------------------------------------------------------'
 */
/* [_COLEMAK] = LAYOUT_planck_grid( */
/*     KC_TAB,  KC_Q,    KC_W,    KC_F,    KC_P,    KC_G,    KC_J,    KC_L,    KC_U,    KC_Y,    KC_SCLN, KC_BSPC, */
/*     KC_ESC,  KC_A,    KC_R,    KC_S,    KC_T,    KC_D,    KC_H,    KC_N,    KC_E,    KC_I,    KC_O,    KC_QUOT, */
/*     KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_K,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_ENT , */
/*     BACKLIT, KC_LCTL, KC_LALT, KC_LGUI, LOWER,   KC_SPC,  KC_SPC,  RAISE,   KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT */
/* ), */

/* Dvorak
 * ,-----------------------------------------------------------------------------------.
 * | Tab  |   "  |   ,  |   .  |   P  |   Y  |   F  |   G  |   C  |   R  |   L  | Bksp |
 * |------+------+------+------+------+-------------+------+------+------+------+------|
 * | Esc  |   A  |   O  |   E  |   U  |   I  |   D  |   H  |   T  |   N  |   S  |  /   |
 * |------+------+------+------+------+------|------+------+------+------+------+------|
 * | Shift|   ;  |   Q  |   J  |   K  |   X  |   B  |   M  |   W  |   V  |   Z  |Enter |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | Brite| Ctrl | Alt  | GUI  |Lower |    Space    |Raise | Left | Down |  Up  |Right |
 * `-----------------------------------------------------------------------------------'
 */
/* [_DVORAK] = LAYOUT_planck_grid( */
/*     KC_TAB,  KC_QUOT, KC_COMM, KC_DOT,  KC_P,    KC_Y,    KC_F,    KC_G,    KC_C,    KC_R,    KC_L,    KC_BSPC, */
/*     KC_ESC,  KC_A,    KC_O,    KC_E,    KC_U,    KC_I,    KC_D,    KC_H,    KC_T,    KC_N,    KC_S,    KC_SLSH, */
/*     KC_LSFT, KC_SCLN, KC_Q,    KC_J,    KC_K,    KC_X,    KC_B,    KC_M,    KC_W,    KC_V,    KC_Z,    KC_ENT , */
/*     BACKLIT, KC_LCTL, KC_LALT, KC_LGUI, LOWER,   KC_SPC,  KC_SPC,  RAISE,   KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT */
/* ), */

/* Lower
 * ,-----------------------------------------------------------------------------------.
 * |   ~  |   !  |   @  |   #  |   $  |   %  |   ^  |   &  |   *  |   (  |   )  | Bksp |
 * |------+------+------+------+------+-------------+------+------+------+------+------|
 * | Del  |  F1  |  F2  |  F3  |  F4  |  F5  |  F6  |   _  |   +  |   {  |   }  |  |   |
 * |------+------+------+------+------+------|------+------+------+------+------+------|
 * |      |  F7  |  F8  |  F9  |  F10 |  F11 |  F12 |ISO ~ |ISO | | Home | End  |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |             |      | Next | Vol- | Vol+ | Play |
 * `-----------------------------------------------------------------------------------'
 */
/* [_LOWER] = LAYOUT_planck_grid( */
/*     KC_TILD, KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC, KC_CIRC, KC_AMPR,    KC_ASTR,    KC_LPRN, KC_RPRN, KC_BSPC, */
/*     KC_DEL,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_UNDS,    KC_PLUS,    KC_LCBR, KC_RCBR, KC_PIPE, */
/*     _______, KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  S(KC_NUHS), S(KC_NUBS), KC_HOME, KC_END,  _______, */
/*     _______, _______, _______, _______, _______, _______, _______, _______,    KC_MNXT,    KC_VOLD, KC_VOLU, KC_MPLY */
/* ), */

/* Raise
 * ,-----------------------------------------------------------------------------------.
 * |   `  |   1  |   2  |   3  |   4  |   5  |   6  |   7  |   8  |   9  |   0  | Bksp |
 * |------+------+------+------+------+-------------+------+------+------+------+------|
 * | Del  |  F1  |  F2  |  F3  |  F4  |  F5  |  F6  |   -  |   =  |   [  |   ]  |  \   |
 * |------+------+------+------+------+------|------+------+------+------+------+------|
 * |      |  F7  |  F8  |  F9  |  F10 |  F11 |  F12 |ISO # |ISO / |Pg Up |Pg Dn |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |             |      | Next | Vol- | Vol+ | Play |
 * `-----------------------------------------------------------------------------------'
 */
/* [_RAISE] = LAYOUT_planck_grid( */
/*     KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC, */
/*     KC_DEL,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_MINS, KC_EQL,  KC_LBRC, KC_RBRC, KC_BSLS, */
/*     _______, KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_NUHS, KC_NUBS, KC_PGUP, KC_PGDN, _______, */
/*     _______, _______, _______, _______, _______, _______, _______, _______, KC_MNXT, KC_VOLD, KC_VOLU, KC_MPLY */
/* ), */

/* Plover layer (http://opensteno.org)
 * ,-----------------------------------------------------------------------------------.
 * |   #  |   #  |   #  |   #  |   #  |   #  |   #  |   #  |   #  |   #  |   #  |   #  |
 * |------+------+------+------+------+-------------+------+------+------+------+------|
 * |      |   S  |   T  |   P  |   H  |   *  |   *  |   F  |   P  |   L  |   T  |   D  |
 * |------+------+------+------+------+------|------+------+------+------+------+------|
 * |      |   S  |   K  |   W  |   R  |   *  |   *  |   R  |   B  |   G  |   S  |   Z  |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * | Exit |      |      |   A  |   O  |             |   E  |   U  |      |      |      |
 * `-----------------------------------------------------------------------------------'
 */
/* [_PLOVER] = LAYOUT_planck_grid( */
/*     KC_1,    KC_1,    KC_1,    KC_1,    KC_1,    KC_1,    KC_1,    KC_1,    KC_1,    KC_1,    KC_1,    KC_1   , */
/*     XXXXXXX, KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, */
/*     XXXXXXX, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, */
/*     EXT_PLV, XXXXXXX, XXXXXXX, KC_C,    KC_V,    XXXXXXX, XXXXXXX, KC_N,    KC_M,    XXXXXXX, XXXXXXX, XXXXXXX */
/* ), */

/* Adjust (Lower + Raise)
 * ,-----------------------------------------------------------------------------------.
 * |      | Reset|      |      |      |      |      |      |      |      |      |  Del |
 * |------+------+------+------+------+-------------+------+------+------+------+------|
 * |      |      |      |Aud on|Audoff|AGnorm|AGswap|Qwerty|Colemk|Dvorak|Plover|      |
 * |------+------+------+------+------+------|------+------+------+------+------+------|
 * |      |Voice-|Voice+|Mus on|Musoff|MIDIon|MIDIof|      |      |      |      |      |
 * |------+------+------+------+------+------+------+------+------+------+------+------|
 * |      |      |      |      |      |             |      |      |      |      |      |
 * `-----------------------------------------------------------------------------------'
 */
/* [_ADJUST] = LAYOUT_planck_grid( */
/*     _______, RESET,   DEBUG,   RGB_TOG, RGB_MOD, RGB_HUI, RGB_HUD, RGB_SAI, RGB_SAD,  RGB_VAI, RGB_VAD, KC_DEL , */
/*     _______, _______, MU_MOD,  AU_ON,   AU_OFF,  AG_NORM, AG_SWAP, QWERTY,  COLEMAK,  DVORAK,  PLOVER,  _______, */
/*     _______, MUV_DE,  MUV_IN,  MU_ON,   MU_OFF,  MI_ON,   MI_OFF,  TERM_ON, TERM_OFF, _______, _______, _______, */
/*     _______, _______, _______, _______, _______, _______, _______, _______, _______,  _______, _______, _______ */
/* ) */

};

/* #ifdef AUDIO_ENABLE */
/*   float plover_song[][2]     = SONG(PLOVER_SOUND); */
/*   float plover_gb_song[][2]  = SONG(PLOVER_GOODBYE_SOUND); */
/* #endif */

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
/*     case QWERTY: */
/*       if (record->event.pressed) { */
/*         print("mode just switched to qwerty and this is a huge string\n"); */
/*         set_single_persistent_default_layer(_QWERTY); */
/*       } */
/*       return false; */
/*       break; */
/*     case COLEMAK: */
/*       if (record->event.pressed) { */
/*         set_single_persistent_default_layer(_COLEMAK); */
/*       } */
/*       return false; */
/*       break; */
/*     case DVORAK: */
/*       if (record->event.pressed) { */
/*         set_single_persistent_default_layer(_DVORAK); */
/*       } */
/*       return false; */
/*       break; */
/*     case BACKLIT: */
/*       if (record->event.pressed) { */
/*         register_code(KC_RSFT); */
/*         #ifdef BACKLIGHT_ENABLE */
/*           backlight_step(); */
/*         #endif */
/*         #ifdef KEYBOARD_planck_rev5 */
/*           PORTE &= ~(1<<6); */
/*         #endif */
/*       } else { */
/*         unregister_code(KC_RSFT); */
/*         #ifdef KEYBOARD_planck_rev5 */
/*           PORTE |= (1<<6); */
/*         #endif */
/*       } */
/*       return false; */
/*       break; */
    /* case PLOVER: */
    /*   if (record->event.pressed) { */
    /*     #ifdef AUDIO_ENABLE */
    /*       stop_all_notes(); */
    /*       PLAY_SONG(plover_song); */
    /*     #endif */
    /*     layer_off(_RAISE); */
    /*     layer_off(_LOWER); */
    /*     layer_off(_ADJUST); */
    /*     layer_on(_PLOVER); */
    /*     if (!eeconfig_is_enabled()) { */
    /*         eeconfig_init(); */
    /*     } */
    /*     keymap_config.raw = eeconfig_read_keymap(); */
    /*     keymap_config.nkro = 1; */
    /*     eeconfig_update_keymap(keymap_config.raw); */
    /*   } */
    /*   return false; */
    /*   break; */
/*     case EXT_PLV: */
/*       if (record->event.pressed) { */
/*         #ifdef AUDIO_ENABLE */
/*           PLAY_SONG(plover_gb_song); */
/*         #endif */
/*         layer_off(_PLOVER); */
/*       } */
/*       return false; */
/*       break; */
  }
  return true;
}

/* bool music_mask_user(uint16_t keycode) { */
/*   switch (keycode) { */
/*     case LCTLESC: */
/*     case LTPSC: */
/*     case MOUSEL: */
/*     case OTHERL: */
/*     case SCOLONL: */
/*     case SLASHL: */
/*     case CLTQUOT: */
/*       return false; */
/*     default: */
/*       return true; */
/*   } */
/* } */
