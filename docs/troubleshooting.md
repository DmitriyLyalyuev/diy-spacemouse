# Troubleshooting

## No serial device

Run:

```sh
arduino-cli board list
```

If the RP2040 does not appear, try another USB data cable and port. Put the board into BOOTSEL mode by holding BOOT/BOOTSEL while connecting USB, then run the flash helper again:

```sh
tools/flash_monitor.py Code/firmware --seconds 20
```

## TLV sensor error or no movement

Check that the TLV493D is connected to STEMMA QT / Qwiic and that the firmware is built for the tested QT Py RP2040 target. The firmware expects the sensor on `Wire1` at address `0x5E`.

If you changed boards or wiring, verify the active I2C bus and pins. A Raspberry Pi Pico is not wired like a QT Py RP2040 by default.

## Fusion does not orbit

This project does not use native SpaceMouse HID reports. It emulates `Shift + middle mouse button` mouse dragging for Fusion 360 orbit.

Check that Fusion 360 has focus, the model viewport is active, and macOS allows the USB HID device to send mouse/keyboard events.

## Home button does not work

The Home button is connected between GPIO27 and GND. A short press sends `Cmd+Shift+N`; a long press recalibrates the neutral position. Fusion 360 needs the included AddIn running and bound to `Cmd+Shift+N` for Home View.

See `Code/Fusion360 Add-in/Send Home` and the README section about the Fusion 360 AddIn.

## Stuck Shift or middle mouse button

The firmware includes release heartbeat/watchdog logic, but a stuck modifier can still happen during development, crashes, unplugging, or failed uploads. Unplug the device, click the mouse once, and press/release Shift on the keyboard to clear host-side state.

If it repeats, reflash the firmware and check serial output with:

```sh
tools/flash_monitor.py Code/firmware --seconds 20
```

## Cursor recenter is approximate

After orbiting, the firmware replays cursor movement to approximate recentering. This is not exact because host mouse acceleration, Fusion viewport state, and OS event handling can change the final cursor position.

## Neutral position drifts

Hold the GPIO27 button for `HOME_LONG_PRESS_MS` to recalibrate the neutral magnetic position. Keep the cap released and centered while holding the button.

## Axes feel inverted, too fast, or too slow

The firmware keeps Y inverted for Fusion orbit behavior and applies a nonlinear safe curve. If the direction or speed feels wrong for your build, check sensor orientation, magnet placement, and mechanical centering first. Firmware constants may need tuning for different magnets, springs, or sensor placement.
