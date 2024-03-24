/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_input_listener

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zephyr/dt-bindings/input/input-event-codes.h>

#include <zmk/endpoints.h>
#include <zmk/mouse/types.h>
#include <zmk/mouse/hid.h>

#define ONE_IF_DEV_OK(n)                                                                           \
    COND_CODE_1(DT_NODE_HAS_STATUS(DT_INST_PHANDLE(n, device), okay), (1 +), (0 +))

#define VALID_LISTENER_COUNT (DT_INST_FOREACH_STATUS_OKAY(ONE_IF_DEV_OK) 0)

#if VALID_LISTENER_COUNT > 0

enum input_listener_xy_data_mode {
    INPUT_LISTENER_XY_DATA_MODE_NONE,
    INPUT_LISTENER_XY_DATA_MODE_REL,
    INPUT_LISTENER_XY_DATA_MODE_ABS,
};

struct input_listener_xy_data {
    enum input_listener_xy_data_mode mode;
    int16_t x;
    int16_t y;
};

#if IS_ENABLED(CONFIG_ZMK_TRACKPAD)
struct input_listener_ptp_finger {
    int16_t x;
    int16_t y;
    bool active;
};
#endif // IS_ENABLED(CONFIG_ZMK_TRACKPAD)

struct input_listener_data {
    union {
        struct {
            struct input_listener_xy_data data;
            struct input_listener_xy_data wheel_data;

            uint8_t button_set;
            uint8_t button_clear;
        } mouse;

#if IS_ENABLED(CONFIG_ZMK_TRACKPAD)
        struct {
            bool pending_data;
            struct input_listener_xy_data data;
            uint8_t finger_idx;
            bool touched;
        } ptp;
#endif // IS_ENABLED(CONFIG_ZMK_TRACKPAD)
    };
};

enum input_listener_mode {
    INPUT_LISTENER_MODE_MOUSE,
    INPUT_LISTENER_MODE_PTP,
};

struct input_listener_config {
    bool xy_swap;
    bool x_invert;
    bool y_invert;
    uint16_t scale_multiplier;
    uint16_t scale_divisor;
    enum input_listener_mode mode;
};

static void handle_rel_code(struct input_listener_data *data, struct input_event *evt) {
    switch (evt->code) {
    case INPUT_REL_X:
        data->mouse.data.mode = INPUT_LISTENER_XY_DATA_MODE_REL;
        data->mouse.data.x += evt->value;
        break;
    case INPUT_REL_Y:
        data->mouse.data.mode = INPUT_LISTENER_XY_DATA_MODE_REL;
        data->mouse.data.y += evt->value;
        break;
    case INPUT_REL_WHEEL:
        data->mouse.wheel_data.mode = INPUT_LISTENER_XY_DATA_MODE_REL;
        data->mouse.wheel_data.y += evt->value;
        break;
    case INPUT_REL_HWHEEL:
        data->mouse.wheel_data.mode = INPUT_LISTENER_XY_DATA_MODE_REL;
        data->mouse.wheel_data.x += evt->value;
        break;
    default:
        break;
    }
}

#if IS_ENABLED(CONFIG_ZMK_TRACKPAD)

static int send_ptp_finger_to_hid(struct input_listener_data *data) {
    struct zmk_ptp_finger finger = {.contact_id = data->ptp.finger_idx};
    finger.x = data->ptp.data.x;
    finger.y = data->ptp.data.y;
    finger.touch_valid = 1;
    finger.tip_switch = data->ptp.touched ? 1 : 0;

    int err = zmk_mouse_hid_set_ptp_finger(finger);

    data->ptp.pending_data = false;
    data->ptp.data.y = data->ptp.data.x = 0;
    data->ptp.touched = false;

    return err;
}

#endif // IS_ENABLED(CONFIG_ZMK_TRACKPAD)

static void handle_abs_code(const struct input_listener_config *config,
                            struct input_listener_data *data, struct input_event *evt) {
    if (config->mode == INPUT_LISTENER_MODE_PTP) {
#if IS_ENABLED(CONFIG_ZMK_TRACKPAD)
        switch (evt->code) {
        case INPUT_ABS_MT_SLOT:
            if (data->ptp.pending_data) {
                send_ptp_finger_to_hid(data);
            }

            data->ptp.pending_data = true;
            data->ptp.finger_idx = evt->value;
            break;
        case INPUT_ABS_X:
            data->ptp.pending_data = true;
            data->ptp.data.mode = INPUT_LISTENER_XY_DATA_MODE_ABS;
            data->ptp.data.x = evt->value;
            break;
        case INPUT_ABS_Y:
            data->ptp.pending_data = true;
            data->ptp.data.mode = INPUT_LISTENER_XY_DATA_MODE_ABS;
            data->ptp.data.y = evt->value;
            break;
        default:
            break;
        }
#else
        LOG_ERR("ZMK_TRACKPAD not enabled");
#endif // IS_ENABLED(CONFIG_ZMK_TRACKPAD)
    }
}

static void handle_key_code(const struct input_listener_config *config,
                            struct input_listener_data *data, struct input_event *evt) {
    int8_t btn;

    switch (evt->code) {
    case INPUT_BTN_0:
    case INPUT_BTN_1:
    case INPUT_BTN_2:
    case INPUT_BTN_3:
    case INPUT_BTN_4:
        // TODO Handle PTP mode!
        btn = evt->code - INPUT_BTN_0;
        if (evt->value > 0) {
            WRITE_BIT(data->mouse.button_set, btn, 1);
        } else {
            WRITE_BIT(data->mouse.button_clear, btn, 1);
        }
        break;
#if IS_ENABLED(CONFIG_ZMK_TRACKPAD)
    case INPUT_BTN_TOUCH:
        if (config->mode == INPUT_LISTENER_MODE_PTP) {
            data->ptp.touched = evt->value > 0;
        }
#endif // IS_ENABLED(CONFIG_ZMK_TRACKPAD)
    default:
        break;
    }
}

static void swap_xy(struct input_event *evt) {
    switch (evt->code) {
    case INPUT_REL_X:
        evt->code = INPUT_REL_Y;
        break;
    case INPUT_REL_Y:
        evt->code = INPUT_REL_X;
        break;
    }
}

static inline bool is_x_data(const struct input_event *evt) {
    return evt->type == INPUT_EV_REL && evt->code == INPUT_REL_X;
}

static inline bool is_y_data(const struct input_event *evt) {
    return evt->type == INPUT_EV_REL && evt->code == INPUT_REL_Y;
}

static void filter_with_input_config(const struct input_listener_config *cfg,
                                     struct input_event *evt) {
    if (!evt->dev) {
        return;
    }

    if (cfg->xy_swap) {
        swap_xy(evt);
    }

    if ((cfg->x_invert && is_x_data(evt)) || (cfg->y_invert && is_y_data(evt))) {
        evt->value = -(evt->value);
    }

    evt->value = (int16_t)((evt->value * cfg->scale_multiplier) / cfg->scale_divisor);
}

static void clear_xy_data(struct input_listener_xy_data *data) {
    data->x = data->y = 0;
    data->mode = INPUT_LISTENER_XY_DATA_MODE_NONE;
}

static void input_handler(const struct input_listener_config *config,
                          struct input_listener_data *data, struct input_event *evt) {
    // First, filter to update the event data as needed.
    filter_with_input_config(config, evt);

    switch (evt->type) {
    case INPUT_EV_REL:
        handle_rel_code(data, evt);
        break;
    case INPUT_EV_ABS:
        handle_abs_code(config, data, evt);
        break;
    case INPUT_EV_KEY:
        handle_key_code(config, data, evt);
        break;
    }

    if (evt->sync) {
        switch (config->mode) {
        case INPUT_LISTENER_MODE_MOUSE:
            if (data->mouse.wheel_data.mode == INPUT_LISTENER_XY_DATA_MODE_REL) {
                zmk_hid_mouse_scroll_set(data->mouse.wheel_data.x, data->mouse.wheel_data.y);
            }

            if (data->mouse.data.mode == INPUT_LISTENER_XY_DATA_MODE_REL) {
                zmk_hid_mouse_movement_set(data->mouse.data.x, data->mouse.data.y);
            }

            if (data->mouse.button_set != 0) {
                for (int i = 0; i < ZMK_MOUSE_HID_NUM_BUTTONS; i++) {
                    if ((data->mouse.button_set & BIT(i)) != 0) {
                        zmk_hid_mouse_button_press(i);
                    }
                }
            }

            if (data->mouse.button_clear != 0) {
                for (int i = 0; i < ZMK_MOUSE_HID_NUM_BUTTONS; i++) {
                    if ((data->mouse.button_clear & BIT(i)) != 0) {
                        zmk_hid_mouse_button_release(i);
                    }
                }
            }

            zmk_endpoints_send_mouse_report();
            zmk_hid_mouse_scroll_set(0, 0);
            zmk_hid_mouse_movement_set(0, 0);

            clear_xy_data(&data->mouse.data);
            clear_xy_data(&data->mouse.wheel_data);

            data->mouse.button_set = data->mouse.button_clear = 0;

            break;
        case INPUT_LISTENER_MODE_PTP:
#if IS_ENABLED(CONFIG_ZMK_TRACKPAD)
            if (data->ptp.pending_data) {
                send_ptp_finger_to_hid(data);
            }

            // TODO: Button Data

            zmk_mouse_hid_ptp_update_scan_time();
            zmk_endpoints_send_ptp_report();

            zmk_mouse_hid_ptp_clear_lifted_fingers();
#else
            LOG_WRN("PTP Mode not supported without CONFIG_ZMK_TRACKPAD=y");
#endif // IS_ENABLED(CONFIG_ZMK_TRACKPAD)

            break;
        }
    }
}

#endif // VALID_LISTENER_COUNT > 0

#define IL_INST(n)                                                                                 \
    COND_CODE_1(                                                                                   \
        DT_NODE_HAS_STATUS(DT_INST_PHANDLE(n, device), okay),                                      \
        (static const struct input_listener_config config_##n =                                    \
             {                                                                                     \
                 .xy_swap = DT_INST_PROP(n, xy_swap),                                              \
                 .x_invert = DT_INST_PROP(n, x_invert),                                            \
                 .y_invert = DT_INST_PROP(n, y_invert),                                            \
                 .scale_multiplier = DT_INST_PROP(n, scale_multiplier),                            \
                 .scale_divisor = DT_INST_PROP(n, scale_divisor),                                  \
                 .mode = DT_INST_ENUM_IDX_OR(n, mode, 0),                                          \
             };                                                                                    \
         static struct input_listener_data data_##n = {};                                          \
         void input_handler_##n(struct input_event *evt) {                                         \
             input_handler(&config_##n, &data_##n, evt);                                           \
         } INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(n, device)), input_handler_##n);),  \
        ())

DT_INST_FOREACH_STATUS_OKAY(IL_INST)
