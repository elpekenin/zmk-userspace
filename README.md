## Short

In this repo I have a few snippets for ZMK config.

- behavior to control dumb (single-color) LEDs
- configure behaviors to be tapped (pressed+released) when layer gets (de)activated
- set up a layer as default (ie: still turned on after power reset)

## Install

To use these things, you dont need any Kconfig stuff, they get automatically compiled, if and only if, you are using them.

1. Add repo on your `west.yml`

```yaml
manifest:
  remotes:
    - name: elpekenin
      url-base: https://github.com/elpekenin
  projects:
    - name: zmk-userspace
      remote: elpekenin
      revision: main
```

2. Include my behaviors declarations in your `dts` (if you are using any)

```c
#include "elpekenin/behaviors.dtsi"
```

## Feature configuration

### GPIO

Configure a `gpio-leds` node. Eg:

```dts
gpios {
    compatible = "gpio-leds";
    green: green {
        gpios = <&gpio1 0 GPIO_ACTIVE_LOW>;
    };
    blue: blue {
        gpios = <&gpio1 2 GPIO_ACTIVE_LOW>;
    };
};
```

You can then use `&gpio N X` where `N` is the number of the LED and `X` the state to be set (`GPIO_ON`, `GPIO_OFF`, `GPIO_TOG`)

Numbering is based on the order of child nodes, in this example `green` would be `0` and `blue` would be `1`

### Layer callbacks

Configure a node with `compatible = "elpekenin,layer-callback";`, and similar to macros, the actual configuration is done in the children of this node.

To extend on the previous example, we could turn a LED on to indicate that a layer is active with:

```dts
layer_callbacks {
    compatible = "elpekenin,layer-callback";
    layer_on {
        layer = <1>;
        event = "on-activation";
        bindings = <&gpio 0 GPIO_ON>;
    };
    layer_off {
        layer = <1>;
        event = "on-deactivation";
        bindings = <&gpio 0 GPIO_OFF>;
    };
};
```

### Default layer

Use the `&df X` behavior to set `X` as the new default layer. It will still be configured after restarting the board.

---

Please, open GitHub issues to address any problems/improvements this repo could use :D
