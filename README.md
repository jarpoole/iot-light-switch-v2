# iot-light-switch-v2

An internet-connected voice-controlled easy-to-use lightswitch controller.

## Features

1. Voice control through google-assistant.
2. Extremely long battery life. The wall-mounted lightswitcher can achieve a very low energy sleep state while the base station remains connected to the internet waiting for commmands.
3. Tool-less magnetic installation.
4. Compatibility with both rocker and post style switches.
5. USB rechargable Li-ion battery pack.
6. Side-by-side installation thanks to narrow width.
7. Contact activation through subtle physical switch.

## Subsystems

- [Light Switch](#light-switch)
  - [Motherboard](#motherboard)
  - [Daughterboard](#daughterboard)
- [Base Station](#light-switch)

![installed](./docs/installed.jpg)

![render](./mechanical/light-switch/inside.png)

![render](./docs/teardown.jpg)

## Light Switch

### Motherboard

![motherboard_populated](./electrical/light-switch-motherboard/populated1.jpg)

![motherboard_3d_top](./electrical/light-switch-motherboard/3d-top.png)

![motherboard_3d_bottom](./electrical/light-switch-motherboard/3d-bottom.png)

![motherboard_pcb_top](./electrical/light-switch-motherboard/pcb-top.png)

![motherboard_pcb_bottom](./electrical/light-switch-motherboard/pcb-bottom.png)

### Daughterboard

The daughterboard has two roles

1. To connect a small momentary switch for the front bumper to the motherboard
2. To connect the battery temperature monitoring thermocouple to the motherboard

![daughterboard_3d](./electrical/light-switch-daughterboard/3d.png)

![daughterboard_installed](./electrical/light-switch-daughterboard/installed.jpg)

![daughterboard_pcb](./electrical/light-switch-daughterboard/pcb.png)

## Base Station

![case](./mechanical/base-station/assembly.jpg)

![case](./mechanical/base-station/inside.jpg)

![case](./mechanical/base-station/case.jpg)

![base_pcb](./electrical/base-station-motherboard/pcb.png)

![schematic](./electrical/base-station-motherboard/schematic.png)

![base_pcb](./electrical/base-station-motherboard/pcb_front.png)

![base_pcb](./electrical/base-station-motherboard/pcb_back.png)
