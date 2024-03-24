/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zmk/mouse/hid.h>

#if IS_ENABLED(CONFIG_ZMK_MOUSE)
int zmk_mouse_hog_send_mouse_report(struct zmk_hid_mouse_report_body *body);
#endif // IS_ENABLED(CONFIG_ZMK_MOUSE)

#if IS_ENABLED(CONFIG_ZMK_TRACKPAD)
int zmk_mouse_hog_send_ptp_report(struct zmk_hid_ptp_report_body *body);
#endif // IS_ENABLED(CONFIG_ZMK_TRAACKPAD)