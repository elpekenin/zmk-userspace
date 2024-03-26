# Copyright (c) 2024 The ZMK Contributors
# SPDX-License-Identifier: MIT

zephyr_library()

zephyr_library_include_directories(include)
zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)

# standalone files
zephyr_library_sources(src/elpekenin.c)

# behaviors
zephyr_library_sources_ifdef(CONFIG_ELPEKENIN_BEHAVIOR_GPIO src/behaviors/behavior_gpio.c)

# features
zephyr_library_sources_ifdef(CONFIG_ELPEKENIN_LAYER_CALLBACK src/layer_callback.c)

# only on central side (or unibody boards)
if ((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  zephyr_library_sources_ifdef(CONFIG_ELPEKENIN_BEHAVIOR_DEFAULT_LAYER src/behaviors/behavior_default_layer.c)
endif()