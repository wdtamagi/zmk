/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_hid.h>

#include <zmk/keys.h>
#include <zmk/hid.h>

#include <zmk/mouse/types.h>

#include <dt-bindings/zmk/hid_usage.h>
#include <dt-bindings/zmk/hid_usage_pages.h>

#define ZMK_MOUSE_HID_NUM_BUTTONS 0x05

#define ZMK_MOUSE_HID_REPORT_ID_MOUSE 0x01

// Windows PTP Collection
// (https://learn.microsoft.com/en-us/windows-hardware/design/component-guidelines/touchpad-windows-precision-touchpad-collection)
#define ZMK_MOUSE_HID_REPORT_ID_DIGITIZER 0x08
#define ZMK_MOUSE_HID_REPORT_ID_FEATURE_PTP_CAPABILITIES 0x04
#define ZMK_MOUSE_HID_REPORT_ID_FEATURE_PTPHQA 0x05

// Configuration Collection
// (https://learn.microsoft.com/en-us/windows-hardware/design/component-guidelines/touchpad-configuration-collection)
#define ZMK_MOUSE_HID_REPORT_ID_FEATURE_PTP_MODE 0x06
#define ZMK_MOUSE_HID_REPORT_ID_FEATURE_PTP_SELECTIVE 0x07

#define ZMK_HID_USAGE_PAGE16(page)                                                                 \
    HID_ITEM(HID_ITEM_TAG_USAGE_PAGE, HID_ITEM_TYPE_GLOBAL, 2), (page & 0xFF), (page >> 8 & 0xFF)

#define ZMK_HID_LOGICAL_MIN32(a)                                                                   \
    HID_ITEM(HID_ITEM_TAG_LOGICAL_MIN, HID_ITEM_TYPE_GLOBAL, 3), (a & 0xFF), (a >> 8 & 0xFF),      \
        (a >> 16 & 0xFF), (a >> 24 & 0xFF)
#define ZMK_HID_LOGICAL_MAX32(a)                                                                   \
    HID_ITEM(HID_ITEM_TAG_LOGICAL_MAX, HID_ITEM_TYPE_GLOBAL, 3), (a & 0xFF), (a >> 8 & 0xFF),      \
        (a >> 16 & 0xFF), (a >> 24 & 0xFF)

#define ZMK_HID_PHYSICAL_MAX32(a)                                                                  \
    HID_ITEM(HID_ITEM_TAG_PHYSICAL_MAX, HID_ITEM_TYPE_GLOBAL, 3), (a & 0xFF), (a >> 8 & 0xFF),     \
        (a >> 16 & 0xFF), (a >> 24 & 0xFF)

#define ZMK_HID_PHYSICAL_MIN8(a) HID_ITEM(HID_ITEM_TAG_PHYSICAL_MIN, HID_ITEM_TYPE_GLOBAL, 1), a
#define ZMK_HID_PHYSICAL_MAX8(a) HID_ITEM(HID_ITEM_TAG_PHYSICAL_MAX, HID_ITEM_TYPE_GLOBAL, 1), a
#define ZMK_HID_PHYSICAL_MAX16(a)                                                                  \
    HID_ITEM(HID_ITEM_TAG_PHYSICAL_MAX, HID_ITEM_TYPE_GLOBAL, 2), (a & 0xFF), (a >> 8 & 0xFF)

#define HID_REPORT_COUNT16(count)                                                                  \
    HID_ITEM(HID_ITEM_TAG_REPORT_COUNT, HID_ITEM_TYPE_GLOBAL, 2), (count & 0xFF),                  \
        (count >> 8 & 0xFF)

#define ZMK_HID_EXPONENT8(a) HID_ITEM(HID_ITEM_TAG_UNIT_EXPONENT, HID_ITEM_TYPE_GLOBAL, 1), a

#define ZMK_HID_UNIT8(a) HID_ITEM(HID_ITEM_TAG_UNIT, HID_ITEM_TYPE_GLOBAL, 1), a
#define ZMK_HID_UNIT16(a)                                                                          \
    HID_ITEM(HID_ITEM_TAG_UNIT, HID_ITEM_TYPE_GLOBAL, 2), (a & 0xFF), (a >> 8 & 0xFF)

#define TRACKPAD_FINGER_DESC(n, c)                                                                 \
    HID_USAGE_PAGE(HID_USAGE_DIGITIZERS), HID_USAGE(HID_USAGE_DIGITIZERS_FINGER),                  \
        HID_COLLECTION(HID_COLLECTION_LOGICAL), HID_LOGICAL_MIN8(0), HID_LOGICAL_MAX8(1),          \
        HID_USAGE(HID_USAGE_DIGITIZERS_TOUCH_VALID), HID_USAGE(HID_USAGE_DIGITIZERS_TIP_SWITCH),   \
        HID_REPORT_COUNT(2), HID_REPORT_SIZE(1),                                                   \
        HID_INPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),            \
                                                                                                   \
        HID_REPORT_COUNT(1), HID_REPORT_SIZE(4), HID_LOGICAL_MAX8(CONFIG_ZMK_TRACKPAD_FINGERS),    \
        HID_USAGE(HID_USAGE_DIGITIZERS_CONTACT_IDENTIFIER),                                        \
        HID_INPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR |                                   \
                  ZMK_HID_MAIN_VAL_ABS), /* Filler Bits */                                         \
        HID_REPORT_SIZE(1), HID_REPORT_COUNT(2),                                                   \
        HID_INPUT(ZMK_HID_MAIN_VAL_CONST | ZMK_HID_MAIN_VAL_ARRAY | ZMK_HID_MAIN_VAL_ABS),         \
                                                                                                   \
        HID_USAGE_PAGE(HID_USAGE_GD), HID_LOGICAL_MIN8(0),                                         \
        HID_LOGICAL_MAX16((CONFIG_ZMK_TRACKPAD_LOGICAL_X & 0xFF),                                  \
                          ((CONFIG_ZMK_TRACKPAD_LOGICAL_X >> 8) & 0xFF)),                          \
        HID_REPORT_SIZE(16),     /* Exponent (-2) */                                               \
        ZMK_HID_EXPONENT8(0x0E), /* Unit (Linear CM) */                                            \
        ZMK_HID_UNIT8(0x11), HID_USAGE(HID_USAGE_GD_X), ZMK_HID_PHYSICAL_MIN8(0x00),               \
        ZMK_HID_PHYSICAL_MAX16(CONFIG_ZMK_TRACKPAD_PHYSICAL_X), HID_REPORT_COUNT(1),               \
        HID_INPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),            \
        HID_LOGICAL_MAX16((CONFIG_ZMK_TRACKPAD_LOGICAL_Y & 0xFF),                                  \
                          ((CONFIG_ZMK_TRACKPAD_LOGICAL_Y >> 8) & 0xFF)),                          \
        ZMK_HID_PHYSICAL_MAX16(CONFIG_ZMK_TRACKPAD_PHYSICAL_Y), HID_USAGE(HID_USAGE_GD_Y),         \
        HID_INPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),            \
        HID_END_COLLECTION,

static const uint8_t zmk_mouse_hid_report_desc[] = {
#if IS_ENABLED(CONFIG_ZMK_MOUSE)
    HID_USAGE_PAGE(HID_USAGE_GD),
    HID_USAGE(HID_USAGE_GD_MOUSE),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
    HID_REPORT_ID(ZMK_MOUSE_HID_REPORT_ID_MOUSE),
    HID_USAGE(HID_USAGE_GD_POINTER),
    HID_COLLECTION(HID_COLLECTION_PHYSICAL),
    HID_USAGE_PAGE(HID_USAGE_BUTTON),
    HID_USAGE_MIN8(0x1),
    HID_USAGE_MAX8(ZMK_MOUSE_HID_NUM_BUTTONS),
    HID_LOGICAL_MIN8(0x00),
    HID_LOGICAL_MAX8(0x01),
    HID_REPORT_SIZE(0x01),
    HID_REPORT_COUNT(0x5),
    HID_INPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),
    // Constant padding for the last 3 bits.
    HID_REPORT_SIZE(0x03),
    HID_REPORT_COUNT(0x01),
    HID_INPUT(ZMK_HID_MAIN_VAL_CONST | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),
    // Some OSes ignore pointer devices without X/Y data.
    HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
    HID_USAGE(HID_USAGE_GD_X),
    HID_USAGE(HID_USAGE_GD_Y),
    HID_USAGE(HID_USAGE_GD_WHEEL),
    HID_LOGICAL_MIN16(0xFF, -0x7F),
    HID_LOGICAL_MAX16(0xFF, 0x7F),
    HID_REPORT_SIZE(0x10),
    HID_REPORT_COUNT(0x03),
    HID_INPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_REL),
    HID_USAGE_PAGE(HID_USAGE_CONSUMER),
    HID_USAGE16(HID_USAGE_CONSUMER_AC_PAN),
    HID_LOGICAL_MIN16(0xFF, -0x7F),
    HID_LOGICAL_MAX16(0xFF, 0x7F),
    HID_REPORT_SIZE(0x08),
    HID_REPORT_COUNT(0x01),
    HID_INPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_REL),
    HID_END_COLLECTION,
    HID_END_COLLECTION,
#endif // IS_ENABLED(CONFIG_ZMK_MOUSE)
#if IS_ENABLED(CONFIG_ZMK_TRACKPAD)
    HID_USAGE_PAGE(HID_USAGE_DIGITIZERS),
    HID_USAGE(HID_USAGE_DIGITIZERS_TOUCH_PAD),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),

    // Windows Precision Touchpad Input Reports
    HID_REPORT_ID(ZMK_MOUSE_HID_REPORT_ID_DIGITIZER),

    LISTIFY(CONFIG_ZMK_TRACKPAD_FINGERS, TRACKPAD_FINGER_DESC, ())

    /* Exponent (-4) */
    ZMK_HID_EXPONENT8(0x0C),
    /* Unit (Linear Seconds) */
    ZMK_HID_UNIT16(0x1001),
    ZMK_HID_PHYSICAL_MAX32(0xFFFF),
    ZMK_HID_LOGICAL_MAX32(0xFFFF),
    HID_REPORT_SIZE(16),
    HID_REPORT_COUNT(1),
    HID_USAGE_PAGE(HID_USAGE_DIGITIZERS),
    HID_USAGE(HID_USAGE_DIGITIZERS_SCAN_TIME),
    HID_INPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),

    // Contact Count
    HID_USAGE(HID_USAGE_DIGITIZERS_CONTACT_COUNT),

    ZMK_HID_PHYSICAL_MAX8(0x00),
    HID_LOGICAL_MAX8(0x05),

    HID_REPORT_COUNT(1),
    HID_REPORT_SIZE(4),
    HID_INPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),

    // Button report
    HID_USAGE_PAGE(HID_USAGE_GEN_BUTTON),
    // Buttons usages (1, 2, 3)
    HID_USAGE(0x01),
    HID_USAGE(0x02),
    HID_USAGE(0x03),

    HID_LOGICAL_MAX8(1),
    HID_REPORT_SIZE(1),
    HID_REPORT_COUNT(3),
    HID_INPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),
    // Byte Padding
    HID_REPORT_COUNT(1),
    HID_INPUT(ZMK_HID_MAIN_VAL_CONST | ZMK_HID_MAIN_VAL_ARRAY | ZMK_HID_MAIN_VAL_ABS),

    // Device Capabilities Feature Report

    HID_USAGE_PAGE(HID_USAGE_DIGITIZERS),
    HID_REPORT_ID(ZMK_MOUSE_HID_REPORT_ID_FEATURE_PTP_CAPABILITIES),
    HID_USAGE(HID_USAGE_DIGITIZERS_CONTACT_COUNT_MAXIMUM),
    HID_USAGE(HID_USAGE_DIGITIZERS_PAD_TYPE),
    HID_REPORT_SIZE(4),
    HID_REPORT_COUNT(2),
    HID_LOGICAL_MAX8(0x0F),
    HID_FEATURE(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),

    // PTPHQA Blob: Necessary for < Windows 10

    // USAGE_PAGE (Vendor Defined)
    ZMK_HID_USAGE_PAGE16(0xFF00),
    HID_REPORT_ID(ZMK_MOUSE_HID_REPORT_ID_FEATURE_PTPHQA),
    // Vendor Usage (0xC5)
    HID_USAGE(0xC5),

    HID_LOGICAL_MIN8(0),
    HID_LOGICAL_MAX16(0xFF, 0x00),
    HID_REPORT_SIZE(8),

    // Report Count (256)
    HID_REPORT_COUNT16(0x0100),

    HID_FEATURE(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),
    HID_END_COLLECTION,

    // // Configuration collection
    HID_USAGE_PAGE(HID_USAGE_DIGITIZERS),
    HID_USAGE(HID_USAGE_DIGITIZERS_DEVICE_CONFIGURATION),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),

    // PTP input mode report
    HID_REPORT_ID(ZMK_MOUSE_HID_REPORT_ID_FEATURE_PTP_MODE),
    HID_USAGE(HID_USAGE_DIGITIZERS_FINGER),
    HID_COLLECTION(HID_COLLECTION_LOGICAL),
    HID_USAGE(HID_USAGE_DIGITIZERS_DEVICE_MODE),
    HID_LOGICAL_MIN8(0),
    HID_LOGICAL_MAX8(10),
    HID_REPORT_SIZE(8),
    HID_REPORT_COUNT(1),
    HID_FEATURE(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),
    HID_END_COLLECTION,

    // PTP Selective reporting report
    HID_USAGE(HID_USAGE_DIGITIZERS_FINGER),
    HID_COLLECTION(HID_COLLECTION_PHYSICAL),
    HID_REPORT_ID(ZMK_MOUSE_HID_REPORT_ID_FEATURE_PTP_SELECTIVE),

    HID_USAGE(HID_USAGE_DIGITIZERS_SURFACE_SWITCH),
    HID_USAGE(HID_USAGE_DIGITIZERS_BUTTON_SWITCH),
    HID_REPORT_SIZE(1),
    HID_REPORT_COUNT(2),

    HID_LOGICAL_MIN8(0),
    HID_LOGICAL_MAX8(1),
    HID_FEATURE(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),
    // Byte Padding
    HID_REPORT_COUNT(6),
    HID_FEATURE(ZMK_HID_MAIN_VAL_CONST | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),
    HID_END_COLLECTION,
    HID_END_COLLECTION,
#endif
};

#if IS_ENABLED(CONFIG_ZMK_MOUSE)

struct zmk_hid_mouse_report_body {
    zmk_mouse_button_flags_t buttons;
    int16_t d_x;
    int16_t d_y;
    int16_t d_scroll_y;
    int16_t d_scroll_x;
} __packed;

struct zmk_hid_mouse_report {
    uint8_t report_id;
    struct zmk_hid_mouse_report_body body;
} __packed;

#endif // IS_ENABLED(CONFIG_ZMK_MOUSE)

#if IS_ENABLED(CONFIG_ZMK_TRACKPAD)

struct zmk_ptp_finger {
    // Touch Valid (bit 0) and tip switch (bit 1)
    uint8_t touch_valid : 1;
    uint8_t tip_switch : 1;
    // Contact ID
    uint8_t contact_id : 4;
    uint8_t padding : 2;
    // X
    uint16_t x;
    // Y
    uint16_t y;
} __packed;

struct zmk_hid_ptp_report_body {
    // Finger reporting
    struct zmk_ptp_finger fingers[CONFIG_ZMK_TRACKPAD_FINGERS];
    // scantime
    uint16_t scan_time;
    // Contact count
    uint8_t contact_count : 4;
    // Buttons
    uint8_t button1 : 1;
    uint8_t button2 : 1;
    uint8_t button3 : 1;

    uint8_t padding : 1;
} __packed;

// Report containing finger data
struct zmk_hid_ptp_report {
    uint8_t report_id;
    struct zmk_hid_ptp_report_body body;
} __packed;

// Feature report for configuration

struct zmk_hid_ptp_feature_selective_report_body {
    // Selective reporting: Surface switch (bit 0), Button switch (bit 1)
    uint8_t surface_switch : 1;
    uint8_t button_switch : 1;
    uint8_t padding : 6;
} __packed;

struct zmk_hid_ptp_feature_selective_report {
    uint8_t report_id;
    struct zmk_hid_ptp_feature_selective_report_body body;
} __packed;

// Feature report for mode
struct zmk_hid_ptp_feature_mode_report {
    uint8_t report_id;
    // input mode, 0 for mouse, 3 for trackpad
    uint8_t mode;
} __packed;

// Feature report for certification
struct zmk_hid_ptp_feature_certification_report {
    uint8_t report_id;

    uint8_t ptphqa_blob[256];
} __packed;

#define PTP_PAD_TYPE_DEPRESSIBLE 0x00
#define PTP_PAD_TYPE_PRESSURE 0x01
#define PTP_PAD_TYPE_NON_CLICKABLE 0x02

// Feature report for device capabilities
struct zmk_hid_ptp_feature_capabilities_report_body {
    // Max touches (L 4bit) and pad type (H 4bit):
    // Max touches: number 3-5
    // Pad type:    0 for Depressible, 1 for Non-depressible, 2 for Non-clickable
    uint8_t max_touches : 4;
    uint8_t pad_type : 4;
} __packed;

// Feature report for device capabilities
struct zmk_hid_ptp_feature_capabilities_report {
    uint8_t report_id;
    struct zmk_hid_ptp_feature_capabilities_report_body body;
} __packed;

#endif

#if IS_ENABLED(CONFIG_ZMK_MOUSE)
int zmk_hid_mouse_button_press(zmk_mouse_button_t button);
int zmk_hid_mouse_button_release(zmk_mouse_button_t button);
int zmk_hid_mouse_buttons_press(zmk_mouse_button_flags_t buttons);
int zmk_hid_mouse_buttons_release(zmk_mouse_button_flags_t buttons);
void zmk_hid_mouse_movement_set(int16_t x, int16_t y);
void zmk_hid_mouse_scroll_set(int8_t x, int8_t y);
void zmk_hid_mouse_movement_update(int16_t x, int16_t y);
void zmk_hid_mouse_scroll_update(int8_t x, int8_t y);
void zmk_hid_mouse_clear(void);

struct zmk_hid_mouse_report *zmk_mouse_hid_get_mouse_report();
#endif // IS_ENABLED(CONFIG_ZMK_MOUSE)

#if IS_ENABLED(CONFIG_ZMK_TRACKPAD)

int zmk_mouse_hid_set_ptp_finger(struct zmk_ptp_finger finger);
void zmk_mouse_hid_ptp_update_scan_time(void);
void zmk_mouse_hid_ptp_clear_lifted_fingers(void);

struct zmk_hid_ptp_report *zmk_mouse_hid_get_ptp_report();

struct zmk_hid_ptp_feature_selective_report *zmk_mouse_hid_ptp_get_feature_selective_report();
void zmk_mouse_hid_ptp_set_feature_selective_report(bool surface_switch, bool button_switch);
struct zmk_hid_ptp_feature_mode_report *zmk_mouse_hid_ptp_get_feature_mode_report();
void zmk_mouse_hid_ptp_set_feature_mode(uint8_t mode);
struct zmk_hid_ptp_feature_certification_report *
zmk_mouse_hid_ptp_get_feature_certification_report();
struct zmk_hid_ptp_feature_capabilities_report *zmk_mouse_hid_ptp_get_feature_capabilities_report();

#endif // IS_ENABLED(CONFIG_ZMK_TRACKPAD)
