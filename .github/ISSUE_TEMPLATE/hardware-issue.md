---
name: ForgeUI Tunnel Run Issue
about: Report a Tunnel Run game, joystick, display, build, or hardware problem
title: "[Tunnel Run] "
labels: hardware
assignees: ''
---

# ForgeUI Tunnel Run Issue

Thanks for testing this physically proven ESP32-S3, ST7789, and analog-joystick procedural tunnel game.

**ForgeUI:** https://forgeui.co.nz

**ForgeUI Studio:** https://studio.forgeui.co.nz

## Problem

Describe what happened, what you expected, and the shortest sequence that reproduces it.

## Current game state

Where did the problem occur?

- [ ] Title / launch
- [ ] Joystick calibration
- [ ] Gameplay / steering
- [ ] Boost
- [ ] Tunnel rendering
- [ ] Collision / `CRITICAL IMPACT`
- [ ] `MISSION LOST` / relaunch
- [ ] Build or flash
- [ ] Other (describe below)

## Hardware

Please provide:

- ESP32 board/model:
- ESP32-S3 revision and PSRAM, if known:
- Display controller and size/resolution:
- Display/module product link:
- Interface type:
- Additional connected hardware:

### Physically proven reference hardware

- ESP32-S3 DevKitC-1
- ST7789 2.25-inch IPS TFT
- Native resolution: 76×284
- Game viewport: 284×76 landscape
- SPI at 27 MHz

## Display wiring

List the wiring you are using.

### Physically proven reference wiring

| Display | ESP32-S3 | Function |
|---|---:|---|
| GND | GND | Ground |
| VCC | 3.3V | Power |
| SCL / SCLK | GPIO12 | SPI clock |
| SDA / MOSI | GPIO11 | SPI data |
| RST | GPIO10 | Reset |
| DC | GPIO9 | Data / command |
| CS | GPIO8 | Chip select |
| BL | GND | Active-low backlight |

## Joystick, steering, and boost

Please provide:

- Joystick module/model or product link:
- Joystick mapping and wiring (SW, VRy, VRx, supply, GND):
- Observed joystick centre/calibration values, if available:
- X/Y steering behaviour (direction, range, drift, or dead zone):
- Button behaviour:
- Boost behaviour and meter response:

### Physically proven joystick mapping

| Joystick | ESP32-S3 |
|---|---:|
| SW | GPIO4 |
| VRy | GPIO5 |
| VRx | GPIO6 |
| +5V-labelled supply | 3.3V |
| GND | GND |

GPIO7 is spare. The +5V-labelled joystick supply pin is intentionally powered from 3.3V for this project.

## Tunnel and impact behaviour

Describe the observed behaviour for any relevant items:

- Tunnel rendering, boundaries, or scrolling:
- Difficulty, narrowing, or speed increase:
- Starfield or exhaust particles:
- Tunnel collision and `CRITICAL IMPACT`:
- Distance / best distance:
- `MISSION LOST` and relaunch:

## Software environment

Please provide:

- Operating system:
- PlatformIO version:
- Espressif32 platform version:
- Arduino ESP32 framework version:
- Build result: PASS / FAIL
- Flash result: PASS / FAIL

## Build or serial output

Paste the relevant output inside a code block. For large logs, attach a file.

## Physical photos

Physical photos are especially useful. If possible, include the ESP32 board, display, joystick wiring, and the on-screen game state where the problem occurs.

## Additional information

Add anything else that may help reproduce or diagnose the issue.
