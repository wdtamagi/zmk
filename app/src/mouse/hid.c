/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/mouse/hid.h>

#if IS_ENABLED(CONFIG_ZMK_MOUSE)

static struct zmk_hid_mouse_report mouse_report = {
    .report_id = ZMK_MOUSE_HID_REPORT_ID_MOUSE,
    .body = {.buttons = 0, .d_x = 0, .d_y = 0, .d_scroll_y = 0}};

// Keep track of how often a button was pressed.
// Only release the button if the count is 0.
static int explicit_button_counts[5] = {0, 0, 0, 0, 0};
static zmk_mod_flags_t explicit_buttons = 0;

#define SET_MOUSE_BUTTONS(btns)                                                                    \
    {                                                                                              \
        mouse_report.body.buttons = btns;                                                          \
        LOG_DBG("Mouse buttons set to 0x%02X", mouse_report.body.buttons);                         \
    }

int zmk_hid_mouse_button_press(zmk_mouse_button_t button) {
    if (button >= ZMK_MOUSE_HID_NUM_BUTTONS) {
        return -EINVAL;
    }

    explicit_button_counts[button]++;
    LOG_DBG("Button %d count %d", button, explicit_button_counts[button]);
    WRITE_BIT(explicit_buttons, button, true);
    SET_MOUSE_BUTTONS(explicit_buttons);
    return 0;
}

int zmk_hid_mouse_button_release(zmk_mouse_button_t button) {
    if (button >= ZMK_MOUSE_HID_NUM_BUTTONS) {
        return -EINVAL;
    }

    if (explicit_button_counts[button] <= 0) {
        LOG_ERR("Tried to release button %d too often", button);
        return -EINVAL;
    }
    explicit_button_counts[button]--;
    LOG_DBG("Button %d count: %d", button, explicit_button_counts[button]);
    if (explicit_button_counts[button] == 0) {
        LOG_DBG("Button %d released", button);
        WRITE_BIT(explicit_buttons, button, false);
    }
    SET_MOUSE_BUTTONS(explicit_buttons);
    return 0;
}

int zmk_hid_mouse_buttons_press(zmk_mouse_button_flags_t buttons) {
    for (zmk_mouse_button_t i = 0; i < ZMK_MOUSE_HID_NUM_BUTTONS; i++) {
        if (buttons & BIT(i)) {
            zmk_hid_mouse_button_press(i);
        }
    }
    return 0;
}

int zmk_hid_mouse_buttons_release(zmk_mouse_button_flags_t buttons) {
    for (zmk_mouse_button_t i = 0; i < ZMK_MOUSE_HID_NUM_BUTTONS; i++) {
        if (buttons & BIT(i)) {
            zmk_hid_mouse_button_release(i);
        }
    }
    return 0;
}

void zmk_hid_mouse_movement_set(int16_t x, int16_t y) {
    mouse_report.body.d_x = x;
    mouse_report.body.d_y = y;
    LOG_DBG("Mouse movement set to %d/%d", mouse_report.body.d_x, mouse_report.body.d_y);
}

void zmk_hid_mouse_movement_update(int16_t x, int16_t y) {
    mouse_report.body.d_x += x;
    mouse_report.body.d_y += y;
    LOG_DBG("Mouse movement updated to %d/%d", mouse_report.body.d_x, mouse_report.body.d_y);
}

void zmk_hid_mouse_scroll_set(int8_t x, int8_t y) {
    mouse_report.body.d_scroll_x = x;
    mouse_report.body.d_scroll_y = y;
    LOG_DBG("Mouse scroll set to %d/%d", mouse_report.body.d_scroll_x,
            mouse_report.body.d_scroll_y);
}

void zmk_hid_mouse_scroll_update(int8_t x, int8_t y) {
    mouse_report.body.d_scroll_x += x;
    mouse_report.body.d_scroll_y += y;
    LOG_DBG("Mouse scroll updated to X: %d/%d", mouse_report.body.d_scroll_x,
            mouse_report.body.d_scroll_y);
}

void zmk_hid_mouse_clear(void) {
    LOG_DBG("Mouse report cleared");
    memset(&mouse_report.body, 0, sizeof(mouse_report.body));
}

struct zmk_hid_mouse_report *zmk_mouse_hid_get_mouse_report(void) {
    return &mouse_report;
}

#endif // IS_ENABLED(CONFIG_ZMK_MOUSE)

#if IS_ENABLED(CONFIG_ZMK_TRACKPAD)

#define FINGER_INIT(idx, _rest)                                                                    \
    { .contact_id = idx }

static struct zmk_hid_ptp_report ptp_report = {
    .report_id = ZMK_MOUSE_HID_REPORT_ID_DIGITIZER,
};

int zmk_mouse_hid_set_ptp_finger(struct zmk_ptp_finger finger) {
    // First try to update existing
    for (int i = 0; i < ptp_report.body.contact_count; i++) {
        if (ptp_report.body.fingers[i].contact_id == finger.contact_id) {
            ptp_report.body.fingers[i] = finger;
            return 0;
        }
    }

    if (ptp_report.body.contact_count == CONFIG_ZMK_TRACKPAD_FINGERS) {
        return -ENOMEM;
    }

    ptp_report.body.fingers[ptp_report.body.contact_count++] = finger;

    return 0;
}

void zmk_mouse_hid_ptp_clear_lifted_fingers(void) {
    int valid_count = 0;
    for (int i = 0; i < ptp_report.body.contact_count; i++) {
        if (!ptp_report.body.fingers[i].tip_switch) {
            continue;
        }

        ptp_report.body.fingers[valid_count++] = ptp_report.body.fingers[i];
    }

    for (int i = valid_count; i < ptp_report.body.contact_count; i++) {
        memset(&ptp_report.body.fingers[i], 0, sizeof(struct zmk_ptp_finger));
    }

    ptp_report.body.contact_count = valid_count;

    if (ptp_report.body.contact_count == 0) {
        ptp_report.body.scan_time = 0;
    }
}

void zmk_mouse_hid_ptp_update_scan_time(void) {
    if (ptp_report.body.contact_count > 0) {
        // scan time is in 100 microsecond units
        ptp_report.body.scan_time = (uint16_t)((k_uptime_get_32() * 10) & 0xFFFF);
    } else {
        ptp_report.body.scan_time = 0;
    }
}

void zmk_mouse_hid_ptp_clear(void) {
    memset(&ptp_report.body, sizeof(struct zmk_hid_ptp_report_body), 0);
}

struct zmk_hid_ptp_report *zmk_mouse_hid_get_ptp_report() {
    return &ptp_report;
}

struct zmk_hid_ptp_feature_selective_report selective_report = {
    .report_id = ZMK_MOUSE_HID_REPORT_ID_FEATURE_PTP_SELECTIVE,
    .body =
        {
            .surface_switch = 1,
            .button_switch = 1,
        },
};

struct zmk_hid_ptp_feature_selective_report *zmk_mouse_hid_ptp_get_feature_selective_report() {
    return &selective_report;
}

void zmk_mouse_hid_ptp_set_feature_selective_report(bool surface_switch, bool button_switch);

struct zmk_hid_ptp_feature_mode_report mode_report = {
    .report_id = ZMK_MOUSE_HID_REPORT_ID_FEATURE_PTP_MODE,
    .mode = 3,
};

struct zmk_hid_ptp_feature_mode_report *zmk_mouse_hid_ptp_get_feature_mode_report() {
    return &mode_report;
}

void zmk_mouse_hid_ptp_set_feature_mode(uint8_t mode) { mode_report.mode = mode; }

static struct zmk_hid_ptp_feature_certification_report cert_report = {
    .report_id = ZMK_MOUSE_HID_REPORT_ID_FEATURE_PTPHQA,
    .ptphqa_blob = {0xfc, 0x28, 0xfe, 0x84, 0x40, 0xcb, 0x9a, 0x87, 0x0d, 0xbe, 0x57, 0x3c, 0xb6,
                    0x70, 0x09, 0x88, 0x07, 0x97, 0x2d, 0x2b, 0xe3, 0x38, 0x34, 0xb6, 0x6c, 0xed,
                    0xb0, 0xf7, 0xe5, 0x9c, 0xf6, 0xc2, 0x2e, 0x84, 0x1b, 0xe8, 0xb4, 0x51, 0x78,
                    0x43, 0x1f, 0x28, 0x4b, 0x7c, 0x2d, 0x53, 0xaf, 0xfc, 0x47, 0x70, 0x1b, 0x59,
                    0x6f, 0x74, 0x43, 0xc4, 0xf3, 0x47, 0x18, 0x53, 0x1a, 0xa2, 0xa1, 0x71, 0xc7,
                    0x95, 0x0e, 0x31, 0x55, 0x21, 0xd3, 0xb5, 0x1e, 0xe9, 0x0c, 0xba, 0xec, 0xb8,
                    0x89, 0x19, 0x3e, 0xb3, 0xaf, 0x75, 0x81, 0x9d, 0x53, 0xb9, 0x41, 0x57, 0xf4,
                    0x6d, 0x39, 0x25, 0x29, 0x7c, 0x87, 0xd9, 0xb4, 0x98, 0x45, 0x7d, 0xa7, 0x26,
                    0x9c, 0x65, 0x3b, 0x85, 0x68, 0x89, 0xd7, 0x3b, 0xbd, 0xff, 0x14, 0x67, 0xf2,
                    0x2b, 0xf0, 0x2a, 0x41, 0x54, 0xf0, 0xfd, 0x2c, 0x66, 0x7c, 0xf8, 0xc0, 0x8f,
                    0x33, 0x13, 0x03, 0xf1, 0xd3, 0xc1, 0x0b, 0x89, 0xd9, 0x1b, 0x62, 0xcd, 0x51,
                    0xb7, 0x80, 0xb8, 0xaf, 0x3a, 0x10, 0xc1, 0x8a, 0x5b, 0xe8, 0x8a, 0x56, 0xf0,
                    0x8c, 0xaa, 0xfa, 0x35, 0xe9, 0x42, 0xc4, 0xd8, 0x55, 0xc3, 0x38, 0xcc, 0x2b,
                    0x53, 0x5c, 0x69, 0x52, 0xd5, 0xc8, 0x73, 0x02, 0x38, 0x7c, 0x73, 0xb6, 0x41,
                    0xe7, 0xff, 0x05, 0xd8, 0x2b, 0x79, 0x9a, 0xe2, 0x34, 0x60, 0x8f, 0xa3, 0x32,
                    0x1f, 0x09, 0x78, 0x62, 0xbc, 0x80, 0xe3, 0x0f, 0xbd, 0x65, 0x20, 0x08, 0x13,
                    0xc1, 0xe2, 0xee, 0x53, 0x2d, 0x86, 0x7e, 0xa7, 0x5a, 0xc5, 0xd3, 0x7d, 0x98,
                    0xbe, 0x31, 0x48, 0x1f, 0xfb, 0xda, 0xaf, 0xa2, 0xa8, 0x6a, 0x89, 0xd6, 0xbf,
                    0xf2, 0xd3, 0x32, 0x2a, 0x9a, 0xe4, 0xcf, 0x17, 0xb7, 0xb8, 0xf4, 0xe1, 0x33,
                    0x08, 0x24, 0x8b, 0xc4, 0x43, 0xa5, 0xe5, 0x24, 0xc2},
};

struct zmk_hid_ptp_feature_certification_report *
zmk_mouse_hid_ptp_get_feature_certification_report() {
    return &cert_report;
}

static struct zmk_hid_ptp_feature_capabilities_report cap_report = {
    .report_id = ZMK_MOUSE_HID_REPORT_ID_FEATURE_PTP_CAPABILITIES,
    .body =
        {
            .max_touches = CONFIG_ZMK_TRACKPAD_FINGERS,
            .pad_type = PTP_PAD_TYPE_DEPRESSIBLE,
        },
};

struct zmk_hid_ptp_feature_capabilities_report *
zmk_mouse_hid_ptp_get_feature_capabilities_report() {
    return &cap_report;
}

#endif // IS_ENABLED(CONFIG_ZMK_TRACKPAD)