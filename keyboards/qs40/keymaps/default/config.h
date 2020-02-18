#ifndef CONFIG_H
#define CONFIG_H

#include "config_common.h"
/* key matrix */
#define MATRIX_ROWS 4
#define MATRIX_COLS 12

// NRF52840 pin map: ((port << 5) | (pin & 0x1F))
#define MATRIX_ROW_PINS \
    { 26, 41, 8, 3 }
#define MATRIX_COL_PINS \
    { 6, 45, 7, 38, 12, 36, 34, 32, 24, 13, 20, 22 }
#define DEBOUNCE 20
#define BLUETOOTH_ENABLE


#define PERMISSVIE_HOLD
#define IGNORE_MOD_TAP_INTERRUPT
#define TAPPING_FORCE_HOLD
#define TAPPING_TERM 150
#define ONESHOT_TIMEOUT 120

#define RGB_MATRIX_KEYPRESSES
#define RGB_MATRIX_FRAMEBUFFER_EFFECTS
#define RGB_MATRIX_LED_PROCESS_LIMIT 20
#define RGB_MATRIX_LED_FLUSH_LIMIT 26
#define DRIVER_ADDR_1 0b1010000
#define DRIVER_ADDR_2 0b1010000

#define DRIVER_COUNT 1
#define DRIVER_1_LED_TOTAL 48
#define DRIVER_LED_TOTAL DRIVER_1_LED_TOTAL

#define DISABLE_RGB_MATRIX_ALPHAS_MODS
#define DISABLE_RGB_MATRIX_GRADIENT_UP_DOWN
#define DISABLE_RGB_MATRIX_BREATHING
#define DISABLE_RGB_MATRIX_BAND_SAT
#define DISABLE_RGB_MATRIX_BAND_PINWHEEL_SAT
#define DISABLE_RGB_MATRIX_BAND_SPIRAL_SAT
#define DISABLE_RGB_MATRIX_RAINDROPS
#define DISABLE_RGB_MATRIX_JELLYBEAN_RAINDROPS
#define DISABLE_RGB_MATRIX_TYPING_HEATMAP
#define DISABLE_RGB_MATRIX_DIGITAL_RAIN
#define DISABLE_RGB_MATRIX_SOLID_REACTIVE
#define DISABLE_RGB_MATRIX_SOLID_REACTIVE_SIMPLE
#define DISABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE
#define DISABLE_RGB_MATRIX_SOLID_REACTIVE_CROSS
#define DISABLE_RGB_MATRIX_SOLID_REACTIVE_NEXUS
#define DISABLE_RGB_MATRIX_SPLASH
#define DISABLE_RGB_MATRIX_SOLID_SPLASH

#define KEYBOARD_SCAN_INTERVAL 6
// The pin number for SCL pin
#define I2C1_SCL 15
// The pin number for SDA pin
#define I2C1_SDA 17
// Enter sleep modes after 600 seconds
#define POWER_SAVE_TIMEOUT 600
// #define BLE_NKRO
// Force NKRO
// #define FORCE_NKRO
#define NKRO_EPSIZE 22
// Enable watchdog
// #define KBD_WDT_ENABLE
#define MAX_ENDPOINTS 8

/*
 * MIDI options
 */

/* Prevent use of disabled MIDI features in the keymap */
#define MIDI_ENABLE_STRICT 1

/* enable basic MIDI features:
   - MIDI notes can be sent when in Music mode is on
*/

#define MIDI_BASIC

/* enable advanced MIDI features:
   - MIDI notes can be added to the keymap
   - Octave shift and transpose
   - Virtual sustain, portamento, and modulation wheel
   - etc.
*/
#define MIDI_ADVANCED

/* override number of MIDI tone keycodes (each octave adds 12 keycodes and allocates 12 bytes) */
#define MIDI_TONE_KEYCODE_OCTAVES 6

#endif
