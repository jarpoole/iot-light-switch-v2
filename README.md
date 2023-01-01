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

## Limitations

1. The low-energy 433Mhz radios I selected are not suitable for this project as they are extremely unreliable and require a complicated software state-machine to service. A better choice would have been something like the `NRF24L01+` which includes automatic retransmission, multicast and an extremely low power consumption in sleep. See [this](https://github.com/jarpoole/NRF24L01_driver) project for more.

2. The `ESP8266` WiFi module is not powerful enough to act as a base station for multiple devices. Its single processing core is mostly preoccupied with handling the networking stack and cannot be blocked by user software for two long without connectivity problems. A more modern SoC like the `ESP32` would have been a much better choice.

## Subsystems

- [Light Switch](#light-switch)
  - [Motherboard](#motherboard)
  - [Daughterboard](#daughterboard)
  - [Actuator](#actuator)
- [Base Station](#light-switch)

![installed](./docs/installed.jpg)

![render](./mechanical/light-switch/inside.png)

![render](./docs/teardown.jpg)

## Light Switch

### Motherboard

1. Low-energy 433Mhz radios
2. Deep-sleep enabled by a 32768Hz oscillator and timer interrupt
3. BMS built around the Texas Instruments BQ25890 buck-boost charge controller. Charges 3.7 Li-ion battery pack from 5v USB power (buck) and produces 5v system power from battery pack when in operation (boost)
4. Audio feedback from a buzzer
5. Integrated USB debug/serial output
6. ICSP programming header

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

### Actuator

The actuator is compatible with both post-style and rocker-style switches. The slide engages with post-style while the vertial rocker engages the rocker-style.

![case](./mechanical/light-switch/case.jpg)

![carriage](./mechanical/light-switch/carriage.jpg)

![carriage_holder](./mechanical/light-switch/carriage_holder.jpg)

![assembled](./mechanical/light-switch/assembly.jpg)

## Base Station

The base station's primary responsibility is to act as a bridge between the internet and the low-energy 433Mhz radio on the light-switch.

1. WiFi connectivity using ESP8266
2. Dual power-supply options
   - Mini USB
   - Terminal block 5V DC
3. Integrated USB debug/serial output
4. Captive portal configuration through a mobile browser

![case](./mechanical/base-station/assembly.jpg)

![case](./mechanical/base-station/inside.jpg)

![case](./mechanical/base-station/case.jpg)

![base_pcb](./electrical/base-station-motherboard/pcb.png)

![schematic](./electrical/base-station-motherboard/schematic.png)

![base_pcb](./electrical/base-station-motherboard/pcb_front.png)

![base_pcb](./electrical/base-station-motherboard/pcb_back.png)
