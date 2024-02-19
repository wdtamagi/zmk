/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/studio/core.h>
#include <zmk/studio/rpc.h>

ZMK_RPC_SUBSYSTEM(core)

#define CORE_RESPONSE(type, ...) ZMK_RPC_RESPONSE(core, type, __VA_ARGS__)

zmk_Response get_lock_state(const zmk_Request *req) {
    zmk_core_LockState resp = zmk_studio_core_get_lock_state();

    return CORE_RESPONSE(get_lock_state, resp);
}

zmk_Response request_unlock(const zmk_Request *req) {
    // TODO: Actually request it, instead of unilaterally unlocking!

    zmk_studio_core_unlock();

    return ZMK_RPC_NO_RESPONSE();
}

ZMK_RPC_SUBSYSTEM_HANDLER(core, get_lock_state, false);
ZMK_RPC_SUBSYSTEM_HANDLER(core, request_unlock, false);

static int core_event_mapper(const zmk_event_t *eh, zmk_Notification *n) {
    struct zmk_studio_core_lock_state_changed *lock_ev = as_zmk_studio_core_lock_state_changed(eh);

    if (!lock_ev) {
        return -ENOTSUP;
    }

    LOG_DBG("Mapped a lock state event properly");

    *n = ZMK_RPC_NOTIFICATION(core, lock_state_changed, lock_ev->state);
    return 0;
}

ZMK_RPC_EVENT_MAPPER(core, core_event_mapper, zmk_studio_core_lock_state_changed);
