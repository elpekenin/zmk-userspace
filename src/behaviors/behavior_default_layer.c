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

#define MAGIC_FLAG (0xF00BA5)
#define MAGIC_MASK (0xFFFFFF00)

static uint32_t masked_value = 0;

static int default_layer_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg) { 
    if (sizeof(masked_value) != len) {
        return -EINVAL;
    }

    const char *next;
    if (!settings_name_steq(name, "val", &next) || next) {
        return -ENOENT;
    }

    int ret = read_cb(cb_arg, &masked_value, sizeof(masked_value));
    if (ret < 0) {
        return ret;
    }

    return 0; // successful read
}

typedef int (*storage_fn_t)(const char *name, const void *val, size_t len);
static int default_layer_export(storage_fn_t storage_cb) {
    return storage_cb("def/val", &masked_value, sizeof(masked_value));
}

struct settings_handler default_layer_conf = {
    .name = "def",
    .h_set = default_layer_set,
    .h_export = default_layer_export,
};

// ^ settings setup
// -----
// v actual behavior 

static int behavior_default_layer_init(__unused const struct device *dev) {
    settings_subsys_init();
    settings_register(&default_layer_conf);
    settings_load();

    if ((masked_value & MAGIC_MASK) != MAGIC_FLAG) {
        return 0; // no flag -> nothing to be done
    }

    uint8_t layer = masked_value & 0xFF;
    (void)zmk_keymap_layer_activate(layer);

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

    (void)zmk_keymap_layer_activate(binding->param1);

    // TODO: handle error?
    masked_value = ((MAGIC_FLAG) << 8) | binding->param1;
    settings_save_one("def/val", &masked_value, sizeof(masked_value));

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
