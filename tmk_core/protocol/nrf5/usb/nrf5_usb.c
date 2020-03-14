#include <stdint.h>
#include "nrf5_usb.h"
#include "ble_service.h"
#include "outputselect.h"
#include "nrf_delay.h"

#ifdef NKRO_ENABLE
#    include "keycode_config.h"
extern keymap_config_t keymap_config;
#endif

static uint8_t keyboard_led_stats = 0;

static bool m_report_pending;

static const app_usbd_hid_subclass_desc_t *keyboard_hid_report_desc[] = {&kbd_desc};
APP_USBD_HID_GENERIC_GLOBAL_DEF(m_app_hid_kbd, HID_KBD_INTERFACE, hid_kbd_user_ev_handler, (NRF_DRV_USBD_EPIN6), keyboard_hid_report_desc, 1, 1, APP_USBD_HID_SUBCLASS_BOOT, APP_USBD_HID_PROTO_KEYBOARD);

#ifdef NKRO_ENABLE
static const app_usbd_hid_subclass_desc_t *keyboard2_hid_report_desc[] = {&kbd2_desc};
APP_USBD_HID_GENERIC_GLOBAL_DEF(m_app_hid_kbd2, HID_KBD2_INTERFACE, hid_kbd_user_ev_handler, (NRF_DRV_USBD_EPIN2), keyboard2_hid_report_desc, 1, 1, APP_USBD_HID_SUBCLASS_NONE, APP_USBD_HID_PROTO_GENERIC);
#endif

#ifdef MOUSE_ENABLE
static const app_usbd_hid_subclass_desc_t *mouse_hid_report_desc[] = {&mouse_desc};
APP_USBD_HID_GENERIC_GLOBAL_DEF(m_app_hid_mouse, HID_MOUSE_INTERFACE, hid_user_ev_handler, (NRF_DRV_USBD_EPIN3), mouse_hid_report_desc, 1, 0, APP_USBD_HID_SUBCLASS_NONE, APP_USBD_HID_PROTO_GENERIC);
#endif

#ifdef EXTRAKEY_ENABLE
static const app_usbd_hid_subclass_desc_t *extra_hid_report_desc[] = {&extrakey_desc};
APP_USBD_HID_GENERIC_GLOBAL_DEF(m_app_hid_extra, HID_EXTRA_INTERFACE, hid_user_ev_handler, (NRF_DRV_USBD_EPIN4), extra_hid_report_desc, 1, 0, APP_USBD_HID_SUBCLASS_NONE, APP_USBD_HID_PROTO_GENERIC);
#endif

#define REPORT_QUEUE_LEN 32

static uint8_t kbd_report_queue[REPORT_QUEUE_LEN][KEYBOARD_REPORT_SIZE];
#ifdef NKRO_ENABLE
static uint8_t kbd2_report_queue[REPORT_QUEUE_LEN][NKRO_EPSIZE];
#endif

static volatile uint8_t widx = 0;

static void hid_kbd_process_state(report_keyboard_t *report) {
    if (m_report_pending) return;
    ret_code_t ret;

#ifdef NKRO_ENABLE
    if (keyboard_protocol && keymap_config.nkro) {
        memcpy(kbd2_report_queue[widx], &report->nkro.mods, NKRO_EPSIZE);
        ret = app_usbd_hid_generic_in_report_set(&m_app_hid_kbd2, kbd2_report_queue[widx++], NKRO_EPSIZE);
        widx %= REPORT_QUEUE_LEN;
    } else
#endif
    {
        memcpy(kbd_report_queue[widx], report, KEYBOARD_REPORT_SIZE);

        // uint8_t asdf[8] = {};
        // memcpy(&asdf[0], &report->mods, 8);
        // NRF_LOG_INFO("%d %d %d %d", asdf[widx], asdf[widx], asdf[2], asdf[3]);
        // NRF_LOG_INFO("%d %d %d %d", asdf[4], asdf[5], asdf[6], asdf[7]);
        // ret = app_usbd_hid_generic_in_report_set(&m_app_hid_kbd, &report->mods, 8);
        ret = app_usbd_hid_generic_in_report_set(&m_app_hid_kbd, kbd_report_queue[widx++], KEYBOARD_REPORT_SIZE);

        widx %= REPORT_QUEUE_LEN;
    }
    APP_ERROR_CHECK(ret);
}

#ifdef MOUSE_ENABLE
static void hid_mouse_process_state(report_mouse_t *report) {
    if (m_report_pending) return;
    ret_code_t ret;

    ret = app_usbd_hid_generic_in_report_set(&m_app_hid_mouse, &report->buttons, 5);
    APP_ERROR_CHECK(ret);
}
#endif

#ifdef EXTRAKEY_ENABLE
static void hid_extra_process_state(uint8_t report_id, uint16_t data) {
    if (m_report_pending) return;
    ret_code_t ret;

    uint8_t report[3];
    report[0] = report_id;
    report[1] = data & 0xFF;
    report[2] = (data >> 8) & 0xFF;

    ret = app_usbd_hid_generic_in_report_set(&m_app_hid_extra, &report, 3);
    APP_ERROR_CHECK(ret);
}
#endif

static void kbd_status(void) {
    size_t         led_stats_size = 0;
    const uint8_t *p_buff;
#ifdef NKRO_ENABLE
    if (keyboard_protocol && keymap_config.nkro) {
        p_buff = app_usbd_hid_generic_out_report_get(&m_app_hid_kbd2, &led_stats_size);
    } else
#endif
    {
        p_buff = app_usbd_hid_generic_out_report_get(&m_app_hid_kbd, &led_stats_size);
    }
    keyboard_led_stats = *(p_buff + 1);
}

/**
 * @brief Class specific event handler.
 *
 * @param p_inst    Class instance.
 * @param event     Class specific event.
 * */
void hid_kbd_user_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_hid_user_event_t event) {
    switch (event) {
        case APP_USBD_HID_USER_EVT_OUT_REPORT_READY: {
            m_report_pending = false;
            kbd_status();
            break;
        }
        case APP_USBD_HID_USER_EVT_IN_REPORT_DONE: {
            m_report_pending = false;
            // hid_generic_mouse_process_state();
            // bsp_board_led_invert(LED_HID_REP_IN);
            break;
        }
        case APP_USBD_HID_USER_EVT_SET_BOOT_PROTO: {
            UNUSED_RETURN_VALUE(hid_generic_clear_buffer(p_inst));
            NRF_LOG_INFO("SET_BOOT_PROTO");
            break;
        }
        case APP_USBD_HID_USER_EVT_SET_REPORT_PROTO: {
            UNUSED_RETURN_VALUE(hid_generic_clear_buffer(p_inst));
            NRF_LOG_INFO("SET_REPORT_PROTO");
            break;
        }
        default:
            break;
    }
}

void hid_user_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_hid_user_event_t event) {
    switch (event) {
        case APP_USBD_HID_USER_EVT_OUT_REPORT_READY: {
            m_report_pending = false;
            /* No output report defined for this example.*/
            ASSERT(0);
            break;
        }
        case APP_USBD_HID_USER_EVT_IN_REPORT_DONE: {
            m_report_pending = false;
            // hid_generic_mouse_process_state();
            // bsp_board_led_invert(LED_HID_REP_IN);
            break;
        }
        case APP_USBD_HID_USER_EVT_SET_BOOT_PROTO: {
            UNUSED_RETURN_VALUE(hid_generic_clear_buffer(p_inst));
            NRF_LOG_INFO("SET_BOOT_PROTO");
            break;
        }
        case APP_USBD_HID_USER_EVT_SET_REPORT_PROTO: {
            UNUSED_RETURN_VALUE(hid_generic_clear_buffer(p_inst));
            NRF_LOG_INFO("SET_REPORT_PROTO");
            break;
        }
        default:
            break;
    }
}

/**
 * @brief USBD library specific event handler.
 *
 * @param event     USBD library event.
 * */
static void usbd_user_ev_handler(app_usbd_event_type_t event) {
    switch (event) {
        case APP_USBD_EVT_DRV_SOF:
            break;
        case APP_USBD_EVT_DRV_RESET:
            m_report_pending = false;
            break;
        case APP_USBD_EVT_DRV_SUSPEND:  // 3
            m_report_pending = false;
            app_usbd_suspend_req();  // Allow the library to put the peripheral into sleep mode
            // bsp_board_leds_off();
            break;
        case APP_USBD_EVT_DRV_RESUME:  // 4
            m_report_pending = false;
            // bsp_board_led_on(LED_USB_START);
            kbd_status(); /* Restore LED state - during SUSPEND all LEDS are turned off */
            break;
        case APP_USBD_EVT_STARTED:  // 2
            m_report_pending = false;
            // bsp_board_led_on(LED_USB_START);
            break;
        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            // bsp_board_leds_off();
            break;
        case APP_USBD_EVT_POWER_DETECTED:  // 0
            NRF_LOG_INFO("USB power detected");
            if (!nrf_drv_usbd_is_enabled()) {
                app_usbd_enable();
            }
            ble_disconnect();
            set_output(OUTPUT_USB);
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            NRF_LOG_INFO("USB power removed");
            app_usbd_stop();
            advertising_start(false);
            break;
        case APP_USBD_EVT_POWER_READY:  // 1
            NRF_LOG_INFO("USB ready");
            app_usbd_start();
            break;
        default:
            break;
    }
}

#ifdef MOUSE_ENABLE
static ret_code_t idle_handle(app_usbd_class_inst_t const *p_inst, uint8_t report_id) {
    switch (report_id) {
        case 0: {
            uint8_t report[] = {0xBE, 0xEF};
            return app_usbd_hid_generic_idle_report_set(&m_app_hid_mouse, report, sizeof(report));
        }
        default:
            return NRF_ERROR_NOT_SUPPORTED;
    }
}
#endif

static uint8_t keyboard_leds(void);
static void    send_keyboard(report_keyboard_t *report);
static void    send_mouse(report_mouse_t *report);
static void    send_system(uint16_t data);
static void    send_consumer(uint16_t data);

static host_driver_t driver = {keyboard_leds, send_keyboard, send_mouse, send_system, send_consumer};

host_driver_t *nrf5_usb_driver(void) { return &driver; }

static uint8_t keyboard_leds(void) { return keyboard_led_stats; }

static void send_keyboard(report_keyboard_t *report) {
    if (NRF_USBD->ENABLE) {
        hid_kbd_process_state(report);
    }
}

static void send_mouse(report_mouse_t *report) {
#ifdef MOUSE_ENABLE
    if (NRF_USBD->ENABLE) {
        hid_mouse_process_state(report);
    }
#endif
}

static void send_system(uint16_t data) {
#ifdef EXTRAKEY_ENABLE
    if (NRF_USBD->ENABLE) {
        hid_extra_process_state(REPORT_ID_SYSTEM, data);
    }
#endif
}

static void send_consumer(uint16_t data) {
#ifdef EXTRAKEY_ENABLE
    if (NRF_USBD->ENABLE) {
        hid_extra_process_state(REPORT_ID_CONSUMER, data);
    }
#endif
}

#ifdef MIDI_ENABLE

void send_midi_packet(MIDI_EventPacket_t *event) {
    // NRF_LOG_INFO("asdfjkl %d", sizeof(MIDI_EventPacket_t));
    // NRF_LOG_INFO("asdfjkl %d %d", event->Event, event->Data1);
    // app_usbd_audio_class_tx_start(&m_app_audio_microphone.base, (uint8_t *)event, sizeof(MIDI_EventPacket_t));

    // NRF_LOG_INFO("EPINEN: %d", NRF_USBD->EPINEN);
    // chnWrite(&drivers.midi_driver.driver, (uint8_t *)event, sizeof(MIDI_EventPacket_t));
}

bool recv_midi_packet(MIDI_EventPacket_t *const event) {
    // size_t size = chnReadTimeout(&drivers.midi_driver.driver, (uint8_t *)event, sizeof(MIDI_EventPacket_t), TIME_IMMEDIATE);
    // return size == sizeof(MIDI_EventPacket_t);
    return 0;
}

#endif

void usb_keyboard_init(void) {
    ret_code_t                     ret;
    static const app_usbd_config_t usbd_config = {.ev_state_proc = usbd_user_ev_handler,
                                                  // chie4 add
                                                  .enable_sof = true};

    ret = app_usbd_init(&usbd_config);
    APP_ERROR_CHECK(ret);

    app_usbd_class_inst_t const *class_inst_kbd;
    class_inst_kbd = app_usbd_hid_generic_class_inst_get(&m_app_hid_kbd);
    ret            = app_usbd_class_append(class_inst_kbd);
    APP_ERROR_CHECK(ret);

#ifdef NKRO_ENABLE
    app_usbd_class_inst_t const *class_inst_kbd2;
    class_inst_kbd2 = app_usbd_hid_generic_class_inst_get(&m_app_hid_kbd2);
    ret             = app_usbd_class_append(class_inst_kbd2);
    APP_ERROR_CHECK(ret);
#endif

#ifdef MOUSE_ENABLE
    app_usbd_class_inst_t const *class_inst_mouse;
    class_inst_mouse = app_usbd_hid_generic_class_inst_get(&m_app_hid_mouse);
    ret              = hid_generic_idle_handler_set(class_inst_mouse, idle_handle);
    ret              = app_usbd_class_append(class_inst_mouse);
    APP_ERROR_CHECK(ret);
#endif

#ifdef EXTRAKEY_ENABLE
    app_usbd_class_inst_t const *class_inst_extra;
    class_inst_extra = app_usbd_hid_generic_class_inst_get(&m_app_hid_extra);
    ret              = app_usbd_class_append(class_inst_extra);
    APP_ERROR_CHECK(ret);
#endif

    NRF_LOG_INFO("USBD HID composite example started.");

    nrf_delay_ms(50);
    ret = app_usbd_power_events_enable();
    APP_ERROR_CHECK(ret);
}
