# Wiring

## Tested wiring

The tested build uses an **Adafruit QT Py RP2040** and an **Infineon TLV493D** magnetic sensor.

- TLV493D connects through STEMMA QT / Qwiic.
- Firmware uses `Wire1`.
- Sensor I2C address is `0x5E`.
- Home button connects GPIO27 to GND.

## GPIO notes

GPIO24 is intentionally disabled in the firmware. On this build it was observed stuck LOW, so do not use GPIO24 for buttons or other inputs without changing and retesting the firmware.

## Wiring diagram

See the existing PDF wiring diagram:

[`../Resources/diy-spacemouse _wiring diagram.pdf`](../Resources/diy-spacemouse%20_wiring%20diagram.pdf)

## Board substitution warning

The firmware is tested with the QT Py RP2040 FQBN:

```text
rp2040:rp2040:adafruit_qtpy:usbstack=picosdk
```

A Raspberry Pi Pico is also RP2040-based, but it is not a drop-in replacement for this documented wiring. Check its I2C pins, button GPIO, boot/upload flow, and Arduino FQBN before using it.
