# Fusion SpaceMouse

[![CC BY-NC-SA 4.0][cc-by-nc-sa-shield]][cc-by-nc-sa]

[Русская версия](README_RU.md)

A DIY magnetic SpaceMouse-style controller for Autodesk Fusion 360 on macOS. It uses an Adafruit QT Py RP2040 and a TLV493D magnetic sensor to emulate orbit controls through standalone USB HID mouse and keyboard reports.

## Hardware

- Adafruit QT Py RP2040
- Infineon TLV493D 3-axis magnetic sensor
- STEMMA QT connection using `Wire1`, sensor address `0x5E`
- Home button between GPIO27 and GND

GPIO24 is intentionally unused; on this build it was observed stuck LOW.

The BOM contains a local Raspberry Pi Pico source link, but the tested firmware target is Adafruit QT Py RP2040. A Pico is not a drop-in replacement without checking pinout, I2C bus, GPIOs, and FQBN.

## Limitation

Fusion 360 on macOS did not accept raw native SpaceMouse HID reports without 3DxWare/navlib. This firmware therefore uses a standalone USB HID mouse/keyboard fallback instead of Arduino `Mouse.h`, `Keyboard.h`, or TinyUSB helper libraries.

## Controls

- Cap deflection: orbit via Shift + middle mouse button mouse emulation
- Release: cursor replay/recenter after orbit movement
- GPIO27 short press: sends Cmd+Shift+N, handled by the included Fusion 360 AddIn to go home
- GPIO27 long press: recalibrates the neutral magnetic position

The firmware keeps Y inverted for Fusion orbit behavior, applies a nonlinear safe curve, and includes anti-stuck release heartbeat/watchdog logic.

## Fusion 360 AddIn

Copy `Code/Fusion360 Add-in/Send Home` into your Fusion 360 AddIns folder. Run the AddIn and bind its command to `Cmd+Shift+N`, which is what the firmware sends from the GPIO27 button. The AddIn command calls `activeViewport.goHome(True)`.

## Documentation

- [Firmware tuning](Code/firmware/README.md)
- [Flashing firmware](docs/flashing.md)
- [Wiring](docs/wiring.md)
- [Assembly notes](docs/assembly.md)
- [Troubleshooting](docs/troubleshooting.md)
- [BOM notes](BOM/README.md)

## Mechanical caveat

The model was designed around M2.5 inserts and screws, but some insert holes in the tested prints were oversized. M3 inserts and matching M3 screws were needed in those places. Test-fit inserts before final assembly.

## Build and upload

See [docs/flashing.md](docs/flashing.md) for setup, BOOTSEL, board detection, flashing, and monitoring notes.

Compile:

```sh
arduino-cli compile --fqbn rp2040:rp2040:adafruit_qtpy:usbstack=picosdk Code/firmware
```

Upload and monitor serial output:

```sh
tools/flash_monitor.py Code/firmware --seconds 20
```

## Repository layout

```text
Code/firmware/                  Final Arduino firmware sketch
Code/Fusion360 Add-in/Send Home/ Fusion 360 AddIn for Home view command
tools/flash_monitor.py           Upload and serial monitor helper
BOM/                             Bill of materials
CAD/                             CAD files
docs/                            Flashing, wiring, assembly, and troubleshooting notes
Images/                          Project images
Resources/                       Additional project resources
```

## License

This project is licensed under CC BY-NC-SA 4.0. See `LICENSE`.

[cc-by-nc-sa]: http://creativecommons.org/licenses/by-nc-sa/4.0/
[cc-by-nc-sa-shield]: https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg
