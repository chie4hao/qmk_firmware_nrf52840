#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "nrf.h"
#include "app_util_platform.h"
#include "nrf_drv_usbd.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_drv_power.h"

#include "app_timer.h"
#include "app_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd_hid_generic.h"
#include "app_usbd_audio.h"
#include "app_error.h"
#include "boards.h"

#include "nrf_log.h"
#include "host_driver.h"
#include "LUFA/Drivers/USB/USB.h"

/**
 * @brief HID generic class interface number.
 * */
#define HID_KBD_INTERFACE 0
#define HID_KBD2_INTERFACE 1
#define HID_MOUSE_INTERFACE 2
#define HID_EXTRA_INTERFACE 3

/**
 * @brief User event handler.
 * */
void hid_user_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_hid_user_event_t event);

void hid_kbd_user_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_hid_user_event_t event);

#define APP_USBD_HID_KBD_REPORT_DSC()                                        \
    {                                                                        \
        0x05, 0x01,     /* USAGE_PAGE (Generic Desktop)                   */ \
            0x09, 0x06, /* USAGE (Keyboard)                               */ \
            0xa1, 0x01, /* COLLECTION (Application)                       */ \
            0x05, 0x07, /*   USAGE_PAGE (Keyboard)                        */ \
            0x19, 0xe0, /*   USAGE_MINIMUM (Keyboard LeftControl)         */ \
            0x29, 0xe7, /*   USAGE_MAXIMUM (Keyboard Right GUI)           */ \
            0x15, 0x00, /*   LOGICAL_MINIMUM (0)                          */ \
            0x25, 0x01, /*   LOGICAL_MAXIMUM (1)                          */ \
            0x75, 0x01, /*   REPORT_SIZE (1)                              */ \
            0x95, 0x08, /*   REPORT_COUNT (8)                             */ \
            0x81, 0x02, /*   INPUT (Data,Var,Abs)                         */ \
            0x95, 0x01, /*   REPORT_COUNT (1)                             */ \
            0x75, 0x08, /*   REPORT_SIZE (8)                              */ \
            0x81, 0x03, /*   INPUT (Cnst,Var,Abs)                         */ \
            0x95, 0x05, /*   REPORT_COUNT (5)                             */ \
            0x75, 0x01, /*   REPORT_SIZE (1)                              */ \
            0x05, 0x08, /*   USAGE_PAGE (LEDs)                            */ \
            0x19, 0x01, /*   USAGE_MINIMUM (Num Lock)                     */ \
            0x29, 0x05, /*   USAGE_MAXIMUM (Kana)                         */ \
            0x91, 0x02, /*   OUTPUT (Data,Var,Abs)                        */ \
            0x95, 0x01, /*   REPORT_COUNT (1)                             */ \
            0x75, 0x03, /*   REPORT_SIZE (3)                              */ \
            0x91, 0x03, /*   OUTPUT (Cnst,Var,Abs)                        */ \
            0x95, 0x06, /*   REPORT_COUNT (6)                             */ \
            0x75, 0x08, /*   REPORT_SIZE (8)                              */ \
            0x15, 0x00, /*   LOGICAL_MINIMUM (0)                          */ \
            0x25, 0x65, /*   LOGICAL_MAXIMUM (101)                        */ \
            0x05, 0x07, /*   USAGE_PAGE (Keyboard)                        */ \
            0x19, 0x00, /*   USAGE_MINIMUM (Reserved (no event indicated))*/ \
            0x29, 0x65, /*   USAGE_MAXIMUM (Keyboard Application)         */ \
            0x81, 0x00, /*   INPUT (Data,Ary,Abs)                         */ \
            0xc0        /* END_COLLECTION                                 */ \
    }
APP_USBD_HID_GENERIC_SUBCLASS_REPORT_DESC(kbd_desc, APP_USBD_HID_KBD_REPORT_DSC());

#ifdef NKRO_ENABLE
#    define APP_USBD_HID_KBD2_REPORT_DSC()                                                            \
        {                                                                                             \
            0x05, 0x01,     /* USAGE_PAGE (Generic Desktop)                   */                      \
                0x09, 0x06, /* USAGE (Keyboard)                               */                      \
                0xa1, 0x01, /* COLLECTION (Application)                       */                      \
                                                                                                      \
                0x05, 0x07, /*   USAGE_PAGE (Keyboard)                        */                      \
                0x19, 0xe0, /*   USAGE_MINIMUM (Keyboard LeftControl)         */                      \
                0x29, 0xe7, /*   USAGE_MAXIMUM (Keyboard Right GUI)           */                      \
                0x15, 0x00, /*   LOGICAL_MINIMUM (0)                          */                      \
                0x25, 0x01, /*   LOGICAL_MAXIMUM (1)                          */                      \
                0x75, 0x01, /*   REPORT_SIZE (1)                              */                      \
                0x95, 0x08, /*   REPORT_COUNT (8)                             */                      \
                0x81, 0x02, /*   INPUT (Data,Var,Abs)                         */                      \
                                                                                                      \
                0x95, 0x05, /*   REPORT_COUNT (5)                             */                      \
                0x75, 0x01, /*   REPORT_SIZE (1)                              */                      \
                0x05, 0x08, /*   USAGE_PAGE (LEDs)                            */                      \
                0x19, 0x01, /*   USAGE_MINIMUM (Num Lock)                     */                      \
                0x29, 0x05, /*   USAGE_MAXIMUM (Kana)                         */                      \
                0x91, 0x02, /*   OUTPUT (Data,Var,Abs)                        */                      \
                0x95, 0x01, /*   REPORT_COUNT (1)                             */                      \
                0x75, 0x03, /*   REPORT_SIZE (3)                              */                      \
                0x91, 0x03, /*   OUTPUT (Cnst,Var,Abs)                        */                      \
                                                                                                      \
                0x95, (NKRO_EPSIZE - 1) * 8,     /*   REPORT_COUNT (6)                             */ \
                0x75, 0x01,                      /*   REPORT_SIZE (8)                              */ \
                0x15, 0x00,                      /*   LOGICAL_MINIMUM (0)                          */ \
                0x25, 0x01,                      /*   LOGICAL_MAXIMUM (101)                        */ \
                0x05, 0x07,                      /*   USAGE_PAGE (Keyboard)                        */ \
                0x19, 0x00,                      /*   USAGE_MINIMUM (Reserved (no event indicated))*/ \
                0x29, (NKRO_EPSIZE - 1) * 8 - 1, /*   USAGE_MAXIMUM (Keyboard Application)         */ \
                0x81, 0x02,                      /*   INPUT (Data,Ary,Abs)                         */ \
                0xc0                             /* END_COLLECTION                                 */ \
        }
APP_USBD_HID_GENERIC_SUBCLASS_REPORT_DESC(kbd2_desc, APP_USBD_HID_KBD2_REPORT_DSC());
#endif

#ifdef MOUSE_ENABLE
#    define APP_USBD_HID_MOUSE_REPORT_DSC(bcnt)                                                                                                               \
        {                                                                                                                                                     \
            0x05, 0x01,     /* Usage Page (Generic Desktop),       */                                                                                         \
                0x09, 0x02, /* Usage (Mouse),                      */                                                                                         \
                0xA1, 0x01, /*  Collection (Application),          */                                                                                         \
                0x09, 0x01, /*   Usage (Pointer),                  */                                                                                         \
                0xA1, 0x00, /*  Collection (Physical),             */                                                                                         \
                                                                                                                                                              \
                0x05, 0x09,         /*     Usage Page (Buttons),           */                                                                                 \
                0x19, 0x01,         /*     Usage Minimum (01),             */                                                                                 \
                0x29, bcnt,         /*     Usage Maximum (bcnt),           */                                                                                 \
                0x15, 0x00,         /*     Logical Minimum (0),            */                                                                                 \
                0x25, 0x01,         /*     Logical Maximum (1),            */                                                                                 \
                0x75, 0x01,         /*     Report Size (1),                */                                                                                 \
                0x95, bcnt,         /*     Report Count (bcnt),            */                                                                                 \
                0x81, 0x02,         /*     Input (Data, Variable, Absolute)*/                                                                                 \
                0x75, (8 - (bcnt)), /*     Report Size (8-(bcnt)),         */                                                                                 \
                0x95, 0x01,         /*     Report Count (1),               */                                                                                 \
                0x81, 0x03,         /*     Input (Constant,Var,Abs),               */                                                                         \
                                                                                                                                                              \
                0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08, 0x95, 0x02, 0x81, 0x06,                                               \
                                                                                                                                                              \
                0x09, 0x38, 0x15, 0x81, 0x25, 0x7f, 0x35, 0x00, 0x45, 0x00, 0x75, 0x08, 0x95, 0x01, 0x81, 0x06,                                               \
                                                                                                                                                              \
                0x05, 0x0c, 0x0a, 0x38, 0x02, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08, 0x95, 0x01, 0x81, 0x06, 0xC0, /*  End Collection,                        */ \
                0xC0,                                                                                           /* End Collection                          */ \
        }
APP_USBD_HID_GENERIC_SUBCLASS_REPORT_DESC(mouse_desc, APP_USBD_HID_MOUSE_REPORT_DSC(5));
#endif

#ifdef EXTRAKEY_ENABLE
#    define APP_USBD_EXTRA_HID_REPORT_DSC()                                                                                                                                         \
        {                                                                                                                                                                           \
            0x05, 0x01, 0x09, 0x80, 0xa1, 0x01, 0x85, REPORT_ID_SYSTEM, 0x15, 0x01, 0x25, 0xb7, 0x19, 0x01, 0x29, 0xb7, 0x75, 0x10, 0x95, 0x01, 0x81, 0x00, 0xc0,                   \
                                                                                                                                                                                    \
                0x05, 0x0c, 0x09, 0x01, 0xa1, 0x01, 0x85, REPORT_ID_CONSUMER, 0x15, 0x01, 0x26, 0x9c, 0x02, 0x19, 0x01, 0x2a, 0x9c, 0x02, 0x75, 0x10, 0x95, 0x01, 0x81, 0x00, 0xc0, \
        }
APP_USBD_HID_GENERIC_SUBCLASS_REPORT_DESC(extrakey_desc, APP_USBD_EXTRA_HID_REPORT_DSC());
#endif

extern uint8_t keyboard_protocol;
host_driver_t *nrf5_usb_driver(void);
void           usb_keyboard_init(void);
