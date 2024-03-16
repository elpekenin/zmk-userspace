/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT elpekenin_behavior_gpio

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(elpekenin, CONFIG_ZMK_LOG_LEVEL);

#include <drivers/behavior.h>
#include <zmk/behavior.h>

#include "elpekenin/__dts/behaviors/gpio.h"

#define GET_GPIO_SPEC_AND_COMMA(node_id) \
    GPIO_DT_SPEC_GET(node_id, gpios),

static const struct gpio_dt_spec gpios[] = {
    DT_FOREACH_CHILD(DT_NODELABEL(gpios), GET_GPIO_SPEC_AND_COMMA)
};

#define N_GPIO ARRAY_SIZE(gpios)

static int behavior_gpio_init(const struct device *dev) {
    for (
        const struct gpio_dt_spec *gpio = gpios;
        gpio < &gpios[N_GPIO];
        ++gpio
    ) {
        // initialize pin
        if (
            !gpio_is_ready_dt(gpio)
            || gpio_pin_configure_dt(gpio, GPIO_OUTPUT_ACTIVE) != 0
        ) {
            LOG_ERR("gpio (%s) did not cooperate: extermine", dev->name);
            return -ENODEV;
        }

        // ensure LED is off
        gpio_pin_set_dt(gpio, false);
    }

    return 0;
};

static int on_keymap_binding_pressed(
    struct zmk_behavior_binding *binding,
    struct zmk_behavior_binding_event event
) {
    if (
        binding->param2 < 0
        && binding->param2 > GPIO_TOG
    ) {
        LOG_ERR("Invalid state (%d) for gpio.", binding->param2);
        return -ENOTSUP;
    }

    if (
        // only supported leds
        binding->param1 < 0
        || binding->param1 >= N_GPIO
    ) {
        LOG_ERR("Invalid gpio number (%d).", binding->param1);
        return -ENOTSUP;
    }

    if (binding->param2 == GPIO_TOG) {
        return gpio_pin_toggle_dt(&gpios[binding->param1]);
    }

    return gpio_pin_set_dt(&gpios[binding->param1], binding->param2);
}

static int on_keymap_binding_released(
    struct zmk_behavior_binding *binding,
    struct zmk_behavior_binding_event event
) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_gpio_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

BEHAVIOR_DT_INST_DEFINE(
    0,
    behavior_gpio_init,
    NULL,
    NULL,
    NULL,
    POST_KERNEL,
    CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
    &behavior_gpio_driver_api
);
