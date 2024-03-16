/*
 * Copyright (c) 2024 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT elpekenin_layer_callback

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(elpekenin, CONFIG_ZMK_LOG_LEVEL);

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <zmk/keymap.h>
#include <zmk/behavior.h>

#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>

#include <drivers/behavior.h>

/* Hack NOT to edit ZMK internals */
#include <zmk/virtual_key_position.h>

#if DT_HAS_COMPAT_STATUS_OKAY(zmk_combos)
#define ADD_ONE() + 1
#define ZMK_COMBOS_LEN (0 DT_FOREACH_CHILD(DT_INST(0, zmk_combos), ADD_ONE))
#else
#define ZMK_COMBOS_LEN 0
#endif

#define ZMK_VIRTUAL_KEY_POSITION_LAYER_CB(index) (ZMK_KEYMAP_LEN + ZMK_KEYMAP_SENSORS_LEN + ZMK_COMBOS_LEN + (index))
/* ------------------------------ */


/* Further events could be implemented if we stored some state, eg 
 *   BECOME_HIGHEST_LAYER
 *   GET_SUPERSEDED
 */
enum layer_event {
    ON_ACTIVATION,
    ON_DEACTIVATION,
};

struct layer_cb_cfg {
    int8_t layer;
    enum layer_event event;
    int32_t virtual_key_position;
    size_t count;
    struct zmk_behavior_binding behaviors[];
};

#define EXTRACT_BINDINGS(n) \
    {LISTIFY(DT_PROP_LEN(n, bindings), ZMK_KEYMAP_EXTRACT_BINDING, (, ), n)}

#define LAYER_CB_STRUCT(n) \
    static struct layer_cb_cfg callback_##n = { \
        .layer = DT_PROP(n, layer), \
        .virtual_key_position = ZMK_VIRTUAL_KEY_POSITION_LAYER_CB(__COUNTER__), \
        .count = DT_PROP_LEN(n, bindings), \
        .event = DT_ENUM_IDX(n, event),\
        .behaviors = EXTRACT_BINDINGS(n) \
    };

DT_INST_FOREACH_CHILD(0, LAYER_CB_STRUCT)

#define LAYER_CB_STRUCT_REF_AND_COMMA(n) \
    &callback_##n,

static struct layer_cb_cfg *callbacks[] = {
    DT_INST_FOREACH_CHILD(0, LAYER_CB_STRUCT_REF_AND_COMMA)
};

#define N_CBS ARRAY_SIZE(callbacks)

// ^ Data model
// -----
// v Listener and potentially make actions

static int layer_change_listener(const zmk_event_t *eh) {
    struct zmk_layer_state_changed *evt = as_zmk_layer_state_changed(eh);

    for (size_t i = 0; i < N_CBS; ++i) {
        const struct layer_cb_cfg *callback = callbacks[i];

        if (
            evt->layer != callback->layer
            || (evt->state && callback->event == ON_DEACTIVATION)
            || (!evt->state && callback->event == ON_ACTIVATION)
        ) {
            continue;
        }

        for (size_t i = 0; i < callback->count; ++i) {
            /* TODO: Is it safe/should I pass a reference to the
             *       actual behavior, instead of making a copy 
             *       to be passed to the func?
             */
            struct zmk_behavior_binding behavior = callback->behaviors[i];

            struct zmk_behavior_binding_event event = {
                .position = callback->virtual_key_position,
                .timestamp = k_uptime_get(),
            };

            behavior_keymap_binding_pressed(&behavior, event);
            behavior_keymap_binding_released(&behavior, event);
        }
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(layer_callback, layer_change_listener);
ZMK_SUBSCRIPTION(layer_callback, zmk_layer_state_changed);
