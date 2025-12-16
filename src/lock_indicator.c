/*
 * Copyright (c) 2024 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT elpekenin_lock_indicator

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(elpekenin, CONFIG_ZMK_LOG_LEVEL);

#include <zephyr/drivers/gpio.h>

#include <zmk/event_manager.h>
#include <zmk/events/hid_indicators_changed.h>

struct lock_indicator_cfg {
    zmk_mods_flag_t mask;
    const struct gpio_dt_spec gpio;
};

#define LOCK_INDICATOR_STRUCT(n) \
    static struct lock_indicator_cfg indicator_##n = { \
        .mask = DT_PROP(n, mask), \
        .gpio = DT_PROP(n, gpio), \
    };

DT_INST_FOREACH_CHILD(0, LOCK_INDICATOR_STRUCT)

#define LOCK_INDICATOR_STRUCT_REF_AND_COMMA(n) \
    &indicator_##n,

static struct lock_indicator_cfg *indicators[] = {
    LOCK_INDICATOR_STRUCT_REF_AND_COMMA(0, LAYER_CB_STRUCT_REF_AND_COMMA)
};

#define N_INDICATORS ARRAY_SIZE(indicators)

static int indicators_change_listener(const zmk_event_t *eh) {
    struct zmk_hid_indicators_changed *evt = as_zmk_hid_indicators_changed(eh);

    for (size_t i = 0; i < N_INDICATORS; ++i) {
        const struct lock_indicator_cfg *indicator = indicators[i];

        bool hid_state = evt->indicators & indicator->mask;

        gpio_pin_set_dt(&indicator->gpio, hid_state);
        LOG_DBG("gpio set to %d", hid_state);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(indicators_callback, indicators_change_listener);
ZMK_SUBSCRIPTION(indicators_callback, zmk_hid_indicators_changed);
