# ForgeUI Tunnel Run — ESP32-S3 + ST7789 284×76

ForgeUI Tunnel Run is an official ForgeUI Hardware Lab project: a physically tested, joystick-controlled procedural tunnel and arcade-graphics showcase for the ESP32-S3 DevKitC-1 and ultra-wide ST7789 284×76 display.

It builds on the physically proven [ForgeUI ST7789 284×76 wide-display baseline](https://github.com/RTechAI/forgeui-hw-st7789-284x76-wide) and belongs to the wider ForgeUI ESP32 hardware/project ecosystem. ForgeUI is developed by [RTechAI](https://github.com/RTechAI).

Physical Hardware Lab validation does not by itself mean this display target is currently integrated into ForgeUI Studio.

![ForgeUI Tunnel Run physical game lifecycle](splash_Tunnelrun.png)

## Physical game pass

The hero image is physical evidence of the complete game lifecycle on the target hardware: the ForgeUI Tunnel Run title screen, active tunnel gameplay, and the `MISSION LOST` screen with distance and best distance.

## Gameplay lifecycle

Press the joystick to launch. Steer the ship through the scrolling procedural tunnel, use the joystick button to boost, and survive as speed increases and the tunnel narrows. A tunnel impact triggers `CRITICAL IMPACT`, followed by `MISSION LOST`; press the joystick again to relaunch.

## Controls

| Input | Action |
|---|---|
| Joystick X/Y | Steer the ship |
| Joystick press | Launch, boost while flying, or relaunch after `MISSION LOST` |

## Physical joystick proof

![ForgeUI joystick and input validation](splash_joystick-proof.png)

The analog joystick was independently validated on the physical hardware before Tunnel Run integration: X movement, Y movement, push switch, and display stability passed.

### Final physically proven joystick mapping

| Joystick | ESP32-S3 |
|---|---:|
| SW | GPIO4 |
| VRy | GPIO5 |
| VRx | GPIO6 |
| +5V-labelled supply | 3.3V |
| GND | GND |

The module's **+5V-labelled supply pin is intentionally powered from the ESP32-S3 3.3V rail**. GPIO7 remains spare.

## Tunnel Run graphical and technical features

- ForgeUI Tunnel Run title screen and joystick launch
- Automatic 64-sample joystick-centre calibration and 180-unit dead zone
- Joystick X/Y controls craft movement; normal speed progresses automatically
- Full-screen 16-bit `TFT_eSprite` framebuffer
- Approximately 30 FPS target loop
- Multi-layer moving starfield, including boosted star streaks
- Procedural sinusoidal tunnel generation, scrolling, and animated boundaries
- Progressive tunnel narrowing with distance; normal speed rises from about 1.7 to 4.2
- Button-edge launch/relaunch; button-hold boost at 1.75× current speed, with drain, recharge, and meter
- Exhaust and explosion particles, plus impact screen shake
- Tunnel collision detection
- Distance scoring and in-session best distance
- `CRITICAL IMPACT` and `MISSION LOST` states, with press-to-relaunch flow

## Hardware

### MCU

- ESP32-S3 DevKitC-1
- ESP32-S3 silicon revision v0.2
- 8 MB embedded PSRAM physically detected

### Display

- Controller: ST7789
- 2.25-inch IPS TFT
- Native resolution: 76×284
- Game viewport: 284×76 landscape
- SPI at 27 MHz

## Display and joystick wiring

### Physically proven display wiring

| TFT display | ESP32-S3 | Function |
|---|---:|---|
| GND | GND | Ground |
| VCC | 3.3V | Display power |
| SCL / SCLK | GPIO12 | SPI clock |
| SDA / MOSI | GPIO11 | SPI data |
| RST | GPIO10 | Reset |
| DC | GPIO9 | Data / command |
| CS | GPIO8 | Chip select |
| BL | GND | Active-low backlight enable |
| MISO | Unused | — |

The tested display module has an active-low backlight input, so BL is connected to GND. Other ST7789 modules may use different backlight circuitry. The joystick wiring is listed in the [final physically proven joystick mapping](#final-physically-proven-joystick-mapping).

## Proven display configuration

| Setting | Proven value |
|---|---|
| Driver | ST7789 |
| Native geometry | 76×284 |
| Landscape viewport | 284×76 |
| Rotation | 1 |
| Display inversion | `false` |
| SPI frequency | 27 MHz |
| MOSI / SCLK | GPIO11 / GPIO12 |
| CS / DC / RST | GPIO8 / GPIO9 / GPIO10 |
| Backlight | Active-low, connected to GND |

### Supporting ST7789 bring-up proof

![ForgeUI ST7789 284×76 physical display validation](splash_st7789-284x76-wide.png)

This original physical display bring-up result validates the wiring, orientation, and ST7789 configuration used by Tunnel Run. For the standalone baseline, see the hardware-reference project below.

## Software and build information

This project uses PlatformIO with the Arduino framework for ESP32. The physically proven board, display, and library configuration is in `platformio.ini`; the Tunnel Run implementation is in `src/main.cpp`.

## Related ForgeUI Projects

- [Golden ST7789 284×76 Wide Display](https://github.com/RTechAI/forgeui-hw-st7789-284x76-wide) — known-good physical ESP32-S3/ST7789 284×76 baseline.
- [ForgeUI MicroDash](https://github.com/RTechAI/forgeui-hw-st7789-284x76-wide-microdash) — compact dashboard/instrumentation graphics showcase.
- [ForgeUI MicroRacer](https://github.com/RTechAI/forgeui-hw-st7789-284x76-wide-microracer) — joystick-controlled racing/arcade graphics showcase.

## ForgeUI Hardware Lab

ForgeUI Hardware Lab is an RTechAI/ForgeUI collection of physically tested ESP32 boards, displays, peripherals, examples, and experimental projects. It establishes reproducible hardware baselines through hardware identification, minimal bring-up, physical proof, and preservation of known-good configurations. Demonstrations and candidate targets can then be evaluated for future ForgeUI Studio workflows.

This project's physical validation does not by itself indicate that the ST7789 284×76 target is currently integrated into ForgeUI Studio.

## About ForgeUI

ForgeUI is developed by [RTechAI](https://github.com/RTechAI). [ForgeUI Studio](https://studio.forgeui.co.nz) is a visual embedded UI/HMI development environment for supported ESP32 hardware.

[ForgeUI](https://forgeui.co.nz) Hardware Lab preserves reproducible physical evidence and evaluates hardware and examples for ForgeUI workflows.

[ForgeUI Hosted Studio](https://studio.forgeui.co.nz) is available for public registration.

## Third-party dependency attribution

This project depends on [TFT_eSPI-ST7789-76x284](https://github.com/atoomnetmarc/TFT_eSPI-ST7789-76x284.git), maintained by atoomnetmarc. It is external third-party software providing support for this ST7789 panel geometry. ForgeUI does not claim ownership of that library or fork; it retains its own copyright and licence terms.

## License and repository scope

Unless otherwise noted, ForgeUI-authored source and documentation in this repository are available under the [MIT License](LICENSE). Third-party dependencies and reference implementations remain subject to their respective licences and copyright.
