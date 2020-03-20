/*
Copyright 2012 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONFIG_H
#define CONFIG_H

#include "config_common.h"

/* USB Device descriptor parameter */
#define VENDOR_ID       0xFEED
#define PRODUCT_ID      0xC475
#define DEVICE_VER      0x0001
#define MANUFACTURER    CHIE_4
#define PRODUCT         QS75
#define DESCRIPTION     A 5x15 ortholinear keyboard

/* key matrix */
#define MATRIX_ROWS 15
#define MATRIX_COLS 5

#define MATRIX_ROW_PINS { B2, B10, A1, B13, C15, B0, C14, A0, C13, B11, B1, B15, A10, A7, A6 }
#define MATRIX_COL_PINS { B8, B9, A8, A9, B12 }

/* Set 0 if debouncing isn't needed */
#define DEBOUNCE 2

#define MUSIC_MAP

#ifdef AUDIO_ENABLE
#define STARTUP_SONG SONG(PLANCK_SOUND)
#endif

#define RGB_MATRIX_KEYPRESSES
#define RGB_MATRIX_FRAMEBUFFER_EFFECTS
#define RGB_MATRIX_LED_PROCESS_LIMIT 10
#define RGB_MATRIX_LED_FLUSH_LIMIT 30
#define DRIVER_ADDR_1 0b1010000
#define DRIVER_ADDR_2 0b1011010

#define DRIVER_COUNT 2
#define DRIVER_1_LED_TOTAL 40
#define DRIVER_2_LED_TOTAL 35
#define DRIVER_LED_TOTAL DRIVER_1_LED_TOTAL + DRIVER_2_LED_TOTAL

#endif
