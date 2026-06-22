# Fusion SpaceMouse firmware

Firmware reads a TLV493D magnetic sensor on `Wire1` at address `0x5E` and exposes a custom USB HID keyboard + mouse device through `USB.registerHIDDevice`. It drives Fusion-style orbit fallback: hold Shift + middle mouse button, send XY mouse deltas, invert Y, then replay recent deltas to recenter.

GPIO27 is the Home button:

- short press sends Cmd+Shift+N;
- hold for `HOME_LONG_PRESS_MS` recalibrates the neutral magnetic position without also sending Home.

## Tuning parameters

All main constants live in `Config.h`.

| Parameter | Purpose |
| --- | --- |
| `KEYBOARD_REPORT_ID`, `MOUSE_REPORT_ID` | HID report IDs used by the custom descriptor and TinyUSB report calls. |
| `MOUSE_MIDDLE_BUTTON` | Mouse button bit used for orbit drag. |
| `TLV493D_ADDR` | TLV493D I2C address. |
| `SERIAL_WAIT_MS` | Boot wait for Serial before continuing. |
| `HOME_BUTTON_PIN` | GPIO for Home/recalibration button. |
| `BUTTON_DEBOUNCE_MS` | Debounce time for GPIO27. |
| `HOME_LONG_PRESS_MS` | Hold duration that triggers recalibration. |
| `CAL_SAMPLES` | Valid samples collected for calibration. |
| `CAL_MAX_ATTEMPTS` | Maximum sensor reads attempted during calibration. |
| `OUT_RANGE` | Maximum mouse delta magnitude per report. |
| `ALPHA` | Low-pass filter strength. Higher follows motion faster. |
| `XY_THRESHOLD` | Deadzone around neutral for X/Y motion. |
| `RESPONSE_RANGE` | Input span that reaches full output. Lower is more sensitive. |
| `SPEED_CURVE_BASE` | Safe curve base speed. Lower makes the center calmer. |
| `MIN_REPORT_MAGNITUDE` | Coarse deadzone after scaling. |
| `IDLE_RELEASE_INTERVAL_MS` | Heartbeat interval for releasing stuck Shift/MMB while idle. |
| `MAX_CONTINUOUS_DRAG_MS` | Watchdog limit for one continuous drag. |
| `DRAG_COOLDOWN_MS` | Cooldown after Home, watchdog, or recalibration. |
| `RECENTER_IDLE_MS` | Idle time before stopping drag/recenter replay. |
| `DRAG_HISTORY_SIZE` | Number of motion samples kept for recenter replay. |
| `MAX_REPLAY_SAMPLES_PER_CALL` | Recenter replay chunk size per loop call. |

## Common changes

- More sensitivity: lower `RESPONSE_RANGE`, or raise `OUT_RANGE`.
- Larger deadzone: raise `XY_THRESHOLD` or `MIN_REPORT_MAGNITUDE`.
- Faster response: raise `ALPHA` carefully; too high may feel jittery.
- Softer center / safer speed: lower `SPEED_CURVE_BASE`.
- Watchdog behavior: lower `MAX_CONTINUOUS_DRAG_MS` to break stuck drags sooner, or raise it for longer continuous orbit gestures.
- Recalibration hold time: change `HOME_LONG_PRESS_MS`.

## Custom HID vs native 3D HID

This firmware is not a native 3Dconnexion-style 3D HID device. It intentionally emulates the Fusion orbit fallback with keyboard Shift and mouse middle-button drag, because that works with the current Fusion setup without a host driver or native 3D HID integration.

## Compile

```sh
arduino-cli compile --fqbn rp2040:rp2040:adafruit_qtpy:usbstack=picosdk Code/firmware
```
