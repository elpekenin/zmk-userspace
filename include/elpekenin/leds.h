/*
 * Copyright (c) 2024 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

// TODO: Find pins across dts, infer number of LEDs instead of hardcoding

#define GREEN_LED 0
#define BLUE_LED  1
#define N_LEDS    2

void set_led(uint8_t pin_no, bool state);
bool get_led_state(uint8_t pin_no);
