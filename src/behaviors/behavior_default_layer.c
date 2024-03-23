/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT elpekenin_behavior_default_layer

#include <zephyr/device.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(elpekenin, CONFIG_ZMK_LOG_LEVEL);

#include <zephyr/settings/settings.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>

static uint8_t __default_layer = 0;

static inline bool is_valid_layer(uint8_t layer_no) {
    return layer_no < ZMK_KEYMAP_LAYERS_LEN;
}

static void default_layer_set(uint8_t layer_no) {
    __default_layer = layer_no;
    zmk_keymap_layer_activate(__default_layer);
    settings_save_one("default_layer/value", &__default_layer, sizeof(__default_layer));
}


static int __settings_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg) { 
    const char *next;
    int rc;

    if (settings_name_steq(name, "value", &next) && !next) {
        if (len != sizeof(__default_layer)) {
            return -EINVAL;
        }

        rc = read_cb(cb_arg, &__default_layer, sizeof(__default_layer));
        if (rc >= 0) {
            return 0;
        }

        return rc;
    }

    return -ENOENT;
}

struct settings_handler default_layer_conf = {
    .name = "default_layer",
    .h_set = __settings_set,
};

static int default_layer_init(void) {
    settings_subsys_init();

    int ret = settings_register(&default_layer_conf);
    if (ret) {
        LOG_ERR("Could not register default layer settings (%d).", ret);
        return ret;
    }

    settings_load_subtree("default_layer");

    if (!is_valid_layer(__default_layer)) {
        // this should only happen on first run
        LOG_ERR("Read: %d, which is invalid. Assigning the setting to 0.", __default_layer);
        __default_layer = 0;
    }

    default_layer_set(__default_layer);

    return 0;
}
SYS_INIT(default_layer_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

// ^ settings setup
// -----
// v actual behavior

static int behavior_default_layer_init(const struct device *dev) {
    return 0; // no-op
}

static int on_keymap_binding_pressed(
    struct zmk_behavior_binding *binding,
    struct zmk_behavior_binding_event event
) {
    uint8_t param = binding->param1;

    if (!is_valid_layer(param)) {
        LOG_ERR("Invalid value (%d) for layer number.", param);
        return -EINVAL;
    };

    LOG_INF("Setting default layer to %d.", param);
    default_layer_set(param);

    return 0;
}

static int on_keymap_binding_released(
    struct zmk_behavior_binding *binding,
    struct zmk_behavior_binding_event event
) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_default_layer_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

BEHAVIOR_DT_INST_DEFINE(
    0,
    behavior_default_layer_init,
    NULL,
    NULL,
    NULL,
    POST_KERNEL,
    CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
    &behavior_default_layer_driver_api
);
