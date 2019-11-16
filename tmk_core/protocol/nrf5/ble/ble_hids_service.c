#include "ble_hids_service.h"
#include "keycode_config.h"

#define OUTPUT_REPORT_INDEX 0    /**< Index of Output Report. */
#define OUTPUT_REPORT_MAX_LEN 1  /**< Maximum length of Output Report. */
#define OUTPUT_REP_REF_ID 1      /**< Id of reference to Keyboard Output Report. */
#define FEATURE_REPORT_INDEX 0   /**< Index of Feature Report. */
#define FEATURE_REPORT_MAX_LEN 2 /**< Maximum length of Feature Report. */
#define FEATURE_REP_REF_ID 0     /**< ID of reference to Keyboard Feature Report. */

#define BASE_USB_HID_SPEC_VERSION 0x0101 /**< Version number of base USB HID Specification implemented by this application. */

static bool            m_in_boot_mode = false; /**< Current protocol mode. */
extern keymap_config_t keymap_config;

BLE_HIDS_DEF(m_hids_kbd, /**< Structure used to identify the HID service. */
             NRF_SDH_BLE_TOTAL_LINK_COUNT, KBD_INPUT_REPORT_KEYS_MAX_LEN,
#ifdef EXTRAKEY_ENABLE
             SYSTEM_INPUT_REPORT_KEYS_MAX_LEN, CONSUMER_INPUT_REPORT_KEYS_MAX_LEN,
#endif
#ifdef MOUSE_ENABLE
             MOUSE_INPUT_REPORT_KEYS_MAX_LEN,
#endif
             OUTPUT_REPORT_MAX_LEN, FEATURE_REPORT_MAX_LEN);

static void on_hids_evt(ble_hids_t *p_hids, ble_hids_evt_t *p_evt);

/**@brief Function for handling Service errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void service_error_handler(uint32_t nrf_error) { APP_ERROR_HANDLER(nrf_error); }

/**@brief Function for initializing HID Service.
 */
void hids_kbd_init(void) {
    ret_code_t                   err_code;
    ble_hids_init_t              hids_init_obj;
    ble_hids_inp_rep_init_t *    p_input_report;
    ble_hids_outp_rep_init_t *   p_output_report;
    ble_hids_feature_rep_init_t *p_feature_report;
    uint8_t                      hid_info_flags;

    static ble_hids_inp_rep_init_t     input_report_array[NUM_BLE_HID_DESC];
    static ble_hids_outp_rep_init_t    output_report_array[1];
    static ble_hids_feature_rep_init_t feature_report_array[1];

    static uint8_t report_map_data[] = {
        0x05, 0x01,  // Usage Page (Generic Desktop)
        0x09, 0x06,  // Usage (Keyboard)
        0xA1, 0x01,  // Collection (Application)

        0x85, KBD_BLE_HID_DESC_NUM + 1,

        0x05, 0x07,  // Usage Page (Key Codes)
        0x19, 0xe0,  // Usage Minimum (224)
        0x29, 0xe7,  // Usage Maximum (231)
        0x15, 0x00,  // Logical Minimum (0)
        0x25, 0x01,  // Logical Maximum (1)
        0x75, 0x01,  // Report Size (1)
        0x95, 0x08,  // Report Count (8)
        0x81, 0x02,  // Input (Data, Variable, Absolute)

        0x95, 0x01,  // Report Count (1)
        0x75, 0x08,  // Report Size (8)
        0x81, 0x01,  // Input (Constant) reserved byte(1)

        0x95, 0x05,  // Report Count (5)
        0x75, 0x01,  // Report Size (1)
        0x05, 0x08,  // Usage Page (Page# for LEDs)
        0x19, 0x01,  // Usage Minimum (1)
        0x29, 0x05,  // Usage Maximum (5)
        0x91, 0x02,  // Output (Data, Variable, Absolute), Led report
        0x95, 0x01,  // Report Count (1)
        0x75, 0x03,  // Report Size (3)
        0x91, 0x01,  // Output (Data, Variable, Absolute), Led report padding

        0x95, 0x06,  // Report Count (6)
        0x75, 0x08,  // Report Size (8)
        0x15, 0x00,  // Logical Minimum (0)
        0x25, 0x65,  // Logical Maximum (101)
        0x05, 0x07,  // Usage Page (Key codes)
        0x19, 0x00,  // Usage Minimum (0)
        0x29, 0x65,  // Usage Maximum (101)
        0x81, 0x00,  // Input (Data, Array) Key array(6 bytes)

        0x09, 0x05,        // Usage (Vendor Defined)
        0x15, 0x00,        // Logical Minimum (0)
        0x26, 0xFF, 0x00,  // Logical Maximum (255)
        0x75, 0x08,        // Report Size (8 bit)
        0x95, 0x02,        // Report Count (2)
        0xB1, 0x02,        // Feature (Data, Variable, Absolute)

        0xC0,  // End Collection (Application)

#if defined BLE_NKRO && defined NKRO_ENABLE
        0x05, 0x01, /* USAGE_PAGE (Generic Desktop)                   */
        0x09, 0x06, /* USAGE (Keyboard)                               */
        0xa1, 0x01, /* COLLECTION (Application)                       */

        0x85, NKRO_BLE_HID_DESC_NUM + 1,

        0x05, 0x07, /*   USAGE_PAGE (Keyboard)                        */
        0x19, 0xe0, /*   USAGE_MINIMUM (Keyboard LeftControl)         */
        0x29, 0xe7, /*   USAGE_MAXIMUM (Keyboard Right GUI)           */
        0x15, 0x00, /*   LOGICAL_MINIMUM (0)                          */
        0x25, 0x01, /*   LOGICAL_MAXIMUM (1)                          */
        0x75, 0x01, /*   REPORT_SIZE (1)                              */
        0x95, 0x08, /*   REPORT_COUNT (8)                             */
        0x81, 0x02, /*   INPUT (Data,Var,Abs)                         */

        0x95, 0x05, /*   REPORT_COUNT (5)                             */
        0x75, 0x01, /*   REPORT_SIZE (1)                              */
        0x05, 0x08, /*   USAGE_PAGE (LEDs)                            */
        0x19, 0x01, /*   USAGE_MINIMUM (Num Lock)                     */
        0x29, 0x05, /*   USAGE_MAXIMUM (Kana)                         */
        0x91, 0x02, /*   OUTPUT (Data,Var,Abs)                        */
        0x95, 0x01, /*   REPORT_COUNT (1)                             */
        0x75, 0x03, /*   REPORT_SIZE (3)                              */
        0x91, 0x03, /*   OUTPUT (Cnst,Var,Abs)                        */

        0x95, (NKRO_INPUT_REPORT_KEYS_MAX_LEN - 1) * 8,     /*   REPORT_COUNT (6)                             */
        0x75, 0x01,                                         /*   REPORT_SIZE (8)                              */
        0x15, 0x00,                                         /*   LOGICAL_MINIMUM (0)                          */
        0x25, 0x01,                                         /*   LOGICAL_MAXIMUM (101)                        */
        0x05, 0x07,                                         /*   USAGE_PAGE (Keyboard)                        */
        0x19, 0x00,                                         /*   USAGE_MINIMUM (Reserved (no event indicated))*/
        0x29, (NKRO_INPUT_REPORT_KEYS_MAX_LEN - 1) * 8 - 1, /*   USAGE_MAXIMUM (Keyboard Application)         */
        0x81, 0x02,                                         /*   INPUT (Data,Ary,Abs)                         */
        0xc0,                                               /* END_COLLECTION                                 */
#endif

#ifdef EXTRAKEY_ENABLE
        // system
        0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
        0x09, 0x80,                    // USAGE (System Control)
        0xa1, 0x01,                    // COLLECTION (Application)
        0x85, EXTRA_BLE_HID_DESC_NUM,  //   REPORT_ID (2)
        0x15, 0x01,                    //   LOGICAL_MINIMUM (0x1)
        0x26, 0xb7, 0x00,              //   LOGICAL_MAXIMUM (0xb7)
        0x19, 0x01,                    //   USAGE_MINIMUM (0x1)
        0x29, 0xb7,                    //   USAGE_MAXIMUM (0xb7)
        0x75, 0x10,                    //   REPORT_SIZE (16)
        0x95, 0x01,                    //   REPORT_COUNT (1)
        0x81, 0x00,                    //   INPUT (Data,Array,Abs)
        0xc0,                          // END_COLLECTION

        // consumer
        0x05, 0x0c,                        // USAGE_PAGE (Consumer Devices)
        0x09, 0x01,                        // USAGE (Consumer Control)
        0xa1, 0x01,                        // COLLECTION (Application)
        0x85, EXTRA_BLE_HID_DESC_NUM + 1,  //   REPORT_ID (3)
        0x15, 0x01,                        //   LOGICAL_MINIMUM (0x1)
        0x26, 0x9c, 0x02,                  //   LOGICAL_MAXIMUM (0x29c)
        0x19, 0x01,                        //   USAGE_MINIMUM (0x1)
        0x2a, 0x9c, 0x02,                  //   USAGE_MAXIMUM (0x29c)
        0x75, 0x10,                        //   REPORT_SIZE (16)
        0x95, 0x01,                        //   REPORT_COUNT (1)
        0x81, 0x00,                        //   INPUT (Data,Array,Abs)
        0xc0,                              // END_COLLECTION
#endif

#ifdef MOUSE_ENABLE
        0x05, 0x01, /* Usage Page (Generic Desktop),       */
        0x09, 0x02, /* Usage (Mouse),                      */
        0xA1, 0x01, /*  Collection (Application),          */

        0x85, MOUSE_BLE_HID_DESC_NUM + 1,

        0x09, 0x01, /*   Usage (Pointer),                  */
        0xA1, 0x00, /*  Collection (Physical),             */

        0x05, 0x09, /*     Usage Page (Buttons),           */
        0x19, 0x01, /*     Usage Minimum (01),             */
        0x29, 5,    /*     Usage Maximum (bcnt),           */
        0x15, 0x00, /*     Logical Minimum (0),            */
        0x25, 0x01, /*     Logical Maximum (1),            */
        0x75, 0x01, /*     Report Size (1),                */
        0x95, 5,    /*     Report Count (bcnt),            */
        0x81, 0x02, /*     Input (Data, Variable, Absolute)*/
        0x75, 3,    /*     Report Size (8-(bcnt)),         */
        0x95, 0x01, /*     Report Count (1),               */
        0x81, 0x03, /*     Input (Constant,Var,Abs),               */

        0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08, 0x95, 0x02, 0x81, 0x06,

        0x09, 0x38, 0x15, 0x81, 0x25, 0x7f, 0x35, 0x00, 0x45, 0x00, 0x75, 0x08, 0x95, 0x01, 0x81, 0x06,

        0x05, 0x0c, 0x0a, 0x38, 0x02, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08, 0x95, 0x01, 0x81, 0x06, 0xC0, /*  End Collection,                        */
        0xC0,                                                                                           /* End Collection                          */
#endif
    };

    memset((void *)input_report_array, 0, sizeof(ble_hids_inp_rep_init_t));
    memset((void *)output_report_array, 0, sizeof(ble_hids_outp_rep_init_t));
    memset((void *)feature_report_array, 0, sizeof(ble_hids_feature_rep_init_t));

    // Initialize HID Service
    p_input_report                      = &input_report_array[KBD_BLE_HID_DESC_NUM];
    p_input_report->max_len             = KBD_INPUT_REPORT_KEYS_MAX_LEN;
    p_input_report->rep_ref.report_id   = KBD_BLE_HID_DESC_NUM + 1;
    p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    p_input_report->sec.cccd_wr = SEC_JUST_WORKS;
    p_input_report->sec.wr      = SEC_JUST_WORKS;
    p_input_report->sec.rd      = SEC_JUST_WORKS;

#if defined BLE_NKRO && defined NKRO_ENABLE
    p_input_report                      = &input_report_array[NKRO_BLE_HID_DESC_NUM];
    p_input_report->max_len             = NKRO_INPUT_REPORT_KEYS_MAX_LEN;
    p_input_report->rep_ref.report_id   = NKRO_BLE_HID_DESC_NUM + 1;
    p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    p_input_report->sec.cccd_wr = SEC_JUST_WORKS;
    p_input_report->sec.wr      = SEC_JUST_WORKS;
    p_input_report->sec.rd      = SEC_JUST_WORKS;
#endif

#ifdef EXTRAKEY_ENABLE
    p_input_report                      = &input_report_array[EXTRA_BLE_HID_DESC_NUM - 1];
    p_input_report->max_len             = SYSTEM_INPUT_REPORT_KEYS_MAX_LEN;
    p_input_report->rep_ref.report_id   = EXTRA_BLE_HID_DESC_NUM;
    p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    p_input_report->sec.cccd_wr = SEC_JUST_WORKS;
    p_input_report->sec.wr      = SEC_JUST_WORKS;
    p_input_report->sec.rd      = SEC_JUST_WORKS;

    p_input_report                      = &input_report_array[EXTRA_BLE_HID_DESC_NUM];
    p_input_report->max_len             = CONSUMER_INPUT_REPORT_KEYS_MAX_LEN;
    p_input_report->rep_ref.report_id   = EXTRA_BLE_HID_DESC_NUM + 1;
    p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    p_input_report->sec.cccd_wr = SEC_JUST_WORKS;
    p_input_report->sec.wr      = SEC_JUST_WORKS;
    p_input_report->sec.rd      = SEC_JUST_WORKS;
#endif

#ifdef MOUSE_ENABLE
    p_input_report                      = &input_report_array[MOUSE_BLE_HID_DESC_NUM];
    p_input_report->max_len             = MOUSE_INPUT_REPORT_KEYS_MAX_LEN;
    p_input_report->rep_ref.report_id   = MOUSE_BLE_HID_DESC_NUM + 1;
    p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    p_input_report->sec.cccd_wr = SEC_JUST_WORKS;
    p_input_report->sec.wr      = SEC_JUST_WORKS;
    p_input_report->sec.rd      = SEC_JUST_WORKS;
#endif

    p_output_report                      = &output_report_array[OUTPUT_REPORT_INDEX];
    p_output_report->max_len             = OUTPUT_REPORT_MAX_LEN;
    p_output_report->rep_ref.report_id   = OUTPUT_REP_REF_ID;
    p_output_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_OUTPUT;

    p_output_report->sec.wr = SEC_JUST_WORKS;
    p_output_report->sec.rd = SEC_JUST_WORKS;

    p_feature_report                      = &feature_report_array[FEATURE_REPORT_INDEX];
    p_feature_report->max_len             = FEATURE_REPORT_MAX_LEN;
    p_feature_report->rep_ref.report_id   = FEATURE_REP_REF_ID;
    p_feature_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_FEATURE;

    p_feature_report->sec.rd = SEC_JUST_WORKS;
    p_feature_report->sec.wr = SEC_JUST_WORKS;

    hid_info_flags = HID_INFO_FLAG_REMOTE_WAKE_MSK | HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK;

    memset(&hids_init_obj, 0, sizeof(hids_init_obj));

    hids_init_obj.evt_handler      = on_hids_evt;
    hids_init_obj.error_handler    = service_error_handler;
    hids_init_obj.is_kb            = false;
    hids_init_obj.is_mouse         = false;
    hids_init_obj.inp_rep_count    = NUM_BLE_HID_DESC;
    hids_init_obj.p_inp_rep_array  = input_report_array;
    hids_init_obj.outp_rep_count   = 1;
    hids_init_obj.p_outp_rep_array = output_report_array;
    // hids_init_obj.feature_rep_count              = 1;
    hids_init_obj.feature_rep_count              = 0;
    hids_init_obj.p_feature_rep_array            = feature_report_array;
    hids_init_obj.rep_map.data_len               = sizeof(report_map_data);
    hids_init_obj.rep_map.p_data                 = report_map_data;
    hids_init_obj.hid_information.bcd_hid        = BASE_USB_HID_SPEC_VERSION;
    hids_init_obj.hid_information.b_country_code = 0;
    hids_init_obj.hid_information.flags          = hid_info_flags;
    hids_init_obj.included_services_count        = 0;
    hids_init_obj.p_included_services_array      = NULL;

    hids_init_obj.rep_map.rd_sec         = SEC_JUST_WORKS;
    hids_init_obj.hid_information.rd_sec = SEC_JUST_WORKS;

    hids_init_obj.boot_kb_inp_rep_sec.cccd_wr = SEC_JUST_WORKS;
    hids_init_obj.boot_kb_inp_rep_sec.rd      = SEC_JUST_WORKS;

    hids_init_obj.boot_kb_outp_rep_sec.rd = SEC_JUST_WORKS;
    hids_init_obj.boot_kb_outp_rep_sec.wr = SEC_JUST_WORKS;

    hids_init_obj.protocol_mode_rd_sec = SEC_JUST_WORKS;
    hids_init_obj.protocol_mode_wr_sec = SEC_JUST_WORKS;
    hids_init_obj.ctrl_point_wr_sec    = SEC_JUST_WORKS;

    err_code = ble_hids_init(&m_hids_kbd, &hids_init_obj);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for sending sample key presses to the peer.
 *
 * @param[in]   report_index      Packet report index. 0:keyboard, 1:system, 2:consumer.
 * @param[in]   key_pattern_len   Pattern length.
 * @param[in]   p_key_pattern     Pattern to be sent.
 */
void keys_send(uint8_t report_index, uint16_t len, uint8_t *p_data) {
    ret_code_t err_code;

    err_code = ble_hids_inp_rep_send(&m_hids_kbd, report_index, len, p_data, m_conn_handle);

    if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_RESOURCES) && (err_code != NRF_ERROR_BUSY) && (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING) && (err_code != NRF_ERROR_FORBIDDEN)) {
        APP_ERROR_HANDLER(err_code);
    }
}

/**@brief Function for handling the HID Report Characteristic Write event.
 *
 * @param[in]   p_evt   HID service event.
 */
static void on_hid_rep_char_write(ble_hids_evt_t *p_evt) {
    if (p_evt->params.char_write.char_id.rep_type == BLE_HIDS_REP_TYPE_OUTPUT) {
        ret_code_t err_code;
        uint8_t    report_val;
        uint8_t    report_index = p_evt->params.char_write.char_id.rep_index;

        if (report_index == OUTPUT_REPORT_INDEX) {
            // This code assumes that the output report is one byte long. Hence the following
            // static assert is made.
            STATIC_ASSERT(OUTPUT_REPORT_MAX_LEN == 1);

            err_code = ble_hids_outp_rep_get(&m_hids_kbd, report_index, OUTPUT_REPORT_MAX_LEN, 0, m_conn_handle, &report_val);
            APP_ERROR_CHECK(err_code);
            keyboard_ble_led_stats = report_val;
        }
    }
}

/**@brief Function for handling HID events.
 *
 * @details This function will be called for all HID events which are passed to the application.
 *
 * @param[in]   p_hids  HID service structure.
 * @param[in]   p_evt   Event received from the HID service.
 */
static void on_hids_evt(ble_hids_t *p_hids, ble_hids_evt_t *p_evt) {
    switch (p_evt->evt_type) {
        case BLE_HIDS_EVT_BOOT_MODE_ENTERED:
            m_in_boot_mode = true;
            break;

        case BLE_HIDS_EVT_REPORT_MODE_ENTERED:
            m_in_boot_mode = false;
            break;

        case BLE_HIDS_EVT_REP_CHAR_WRITE:
            on_hid_rep_char_write(p_evt);
            break;

        case BLE_HIDS_EVT_NOTIF_ENABLED:
            break;

        default:
            // No implementation needed.
            break;
    }
}

static uint8_t keyboard_leds(void);
static void    send_keyboard(report_keyboard_t *report);
static void    send_mouse(report_mouse_t *report);
static void    send_system(uint16_t data);
static void    send_consumer(uint16_t data);

static host_driver_t driver = {keyboard_leds, send_keyboard, send_mouse, send_system, send_consumer};

host_driver_t *nrf5_ble_driver(void) { return &driver; }

static uint8_t keyboard_leds(void) { return keyboard_ble_led_stats; }

static void send_keyboard(report_keyboard_t *report) {
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
        if (!keyboard_protocol || !keymap_config.nkro) {
            keys_send(KBD_BLE_HID_DESC_NUM, KBD_INPUT_REPORT_KEYS_MAX_LEN, &report->mods);
        }
#if defined BLE_NKRO && defined NKRO_ENABLE
        else {
            keys_send(NKRO_BLE_HID_DESC_NUM, NKRO_INPUT_REPORT_KEYS_MAX_LEN, &report->nkro.mods);
        }
#endif
    }
}

static void send_mouse(report_mouse_t *report) {
#ifdef MOUSE_ENABLE
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
        keys_send(MOUSE_BLE_HID_DESC_NUM, MOUSE_INPUT_REPORT_KEYS_MAX_LEN, &report->buttons);
    }
#endif
}

static void send_system(uint16_t data) {
#ifdef EXTRAKEY_ENABLE
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
        keys_send(EXTRA_BLE_HID_DESC_NUM - 1, SYSTEM_INPUT_REPORT_KEYS_MAX_LEN, (uint8_t *)&data);
    }
#endif
}

static void send_consumer(uint16_t data) {
#ifdef EXTRAKEY_ENABLE
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
        keys_send(EXTRA_BLE_HID_DESC_NUM, CONSUMER_INPUT_REPORT_KEYS_MAX_LEN, (uint8_t *)&data);
    }
#endif
}
