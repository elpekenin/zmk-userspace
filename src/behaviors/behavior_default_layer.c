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

static uint8_t default_layer_no = 0;

static int default_layer_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg) { 
    const char *next;
    int rc;

    if (settings_name_steq(name, "value", &next) && !next) {
        if (len != sizeof(default_layer_no)) {
            return -EINVAL;
        }

        rc = read_cb(cb_arg, &default_layer_no, sizeof(default_layer_no));
        if (rc >= 0) {
            return 0;
        }

        return rc;
    }

    return -ENOENT;
}

struct settings_handler default_layer_conf = {
    .name = "default_layer",
    .h_set = default_layer_set,
};

static int default_layer_init(void) {
    settings_subsys_init();

    int ret = settings_register(&default_layer_conf);
    if (ret) {
        LOG_ERR("Could not register default layer settings (%d).", ret);
        return ret;
    }

    settings_load_subtree("default_layer");
    zmk_keymap_layer_to(default_layer_no);

    return 0;
}
SYS_INIT(default_layer_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

// ^ settings setup
// -----
// v actual behavior

static int behavior_default_layer_init(const struct device *dev) {
    return 0;
}

static int on_keymap_binding_pressed(
    struct zmk_behavior_binding *binding,
    struct zmk_behavior_binding_event event
) {
    if (
        binding->param1 < 0
        && binding->param1 >= ZMK_KEYMAP_LAYERS_LEN
    ) {
        LOG_ERR("Invalid value (%d) for layer number.", binding->param1);
        return -EINVAL;
    }

    zmk_keymap_layer_to(binding->param1);

    default_layer_no = binding->param1;
    settings_save_one("default_layer/value", &default_layer_no, sizeof(default_layer_no));

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
