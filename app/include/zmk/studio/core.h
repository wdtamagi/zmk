
#pragma once

#include <zmk/event_manager.h>

enum zmk_studio_core_lock_state {
    ZMK_STUDIO_CORE_LOCK_STATE_LOCKED = 0,
    ZMK_STUDIO_CORE_LOCK_STATE_UNLOCKING = 1,
    ZMK_STUDIO_CORE_LOCK_STATE_UNLOCKED = 2,
};

struct zmk_studio_core_lock_state_changed {
    enum zmk_studio_core_lock_state state;
};

ZMK_EVENT_DECLARE(zmk_studio_core_lock_state_changed);

enum zmk_studio_core_lock_state zmk_studio_core_get_lock_state(void);

void zmk_studio_core_unlock();
void zmk_studio_core_lock();
void zmk_studio_core_initiate_unlock();
void zmk_studio_core_complete_unlock();