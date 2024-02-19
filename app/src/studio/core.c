/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zmk/studio/core.h>

ZMK_EVENT_IMPL(zmk_studio_core_lock_state_changed);

static enum zmk_studio_core_lock_state state = ZMK_STUDIO_CORE_LOCK_STATE_UNLOCKED;

enum zmk_studio_core_lock_state zmk_studio_core_get_lock_state(void) { return state; }

static void set_state(enum zmk_studio_core_lock_state new_state) {
    state = new_state;

    raise_zmk_studio_core_lock_state_changed(
        (struct zmk_studio_core_lock_state_changed){.state = state});
}

void zmk_studio_core_unlock() { set_state(ZMK_STUDIO_CORE_LOCK_STATE_UNLOCKED); }
void zmk_studio_core_lock() { set_state(ZMK_STUDIO_CORE_LOCK_STATE_LOCKED); }
void zmk_studio_core_initiate_unlock() { set_state(ZMK_STUDIO_CORE_LOCK_STATE_UNLOCKING); }
void zmk_studio_core_complete_unlock() { set_state(ZMK_STUDIO_CORE_LOCK_STATE_UNLOCKED); }