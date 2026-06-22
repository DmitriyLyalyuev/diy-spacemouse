# Flashing firmware

Target firmware: `Code/firmware/firmware.ino`. Use `Code/firmware` as the Arduino sketch path.

## Tested board

The current firmware is tested on **Adafruit QT Py RP2040** with this FQBN:

```text
rp2040:rp2040:adafruit_qtpy:usbstack=picosdk
```

The BOM contains a local Raspberry Pi Pico source link, but Pico is not a drop-in replacement for the tested QT Py build. If you use a Pico, verify the pinout, I2C bus, USB stack, and FQBN before flashing.

## Setup overview

1. Install `arduino-cli`.
2. Install the RP2040 Arduino core that provides the `rp2040:rp2040` platform.
3. Install the TLV493D Arduino library required by the firmware.
4. Connect the QT Py RP2040 over USB.

Check installed cores and libraries with:

```sh
arduino-cli core list
arduino-cli lib list
```

## Compile

From the repository root:

```sh
arduino-cli compile --fqbn rp2040:rp2040:adafruit_qtpy:usbstack=picosdk Code/firmware
```

## Find the serial port

Before flashing, list connected boards:

```sh
arduino-cli board list
```

If no RP2040 serial device appears, put the board into BOOTSEL mode: hold BOOT/BOOTSEL while plugging in USB, or press reset while holding BOOT/BOOTSEL if your board exposes both buttons.

## Flash and monitor

Use the project helper to upload and then watch serial output:

```sh
tools/flash_monitor.py Code/firmware --seconds 20
```

The helper uses the `Code/firmware` sketch path and opens the serial monitor after upload.
