#pragma once

#include <Arduino.h>

namespace Config
{
constexpr uint8_t KEYBOARD_REPORT_ID = 1;
constexpr uint8_t MOUSE_REPORT_ID = 2;
constexpr uint8_t MOUSE_MIDDLE_BUTTON = 0x04;

constexpr uint8_t TLV493D_ADDR = 0x5E;
constexpr unsigned long SERIAL_WAIT_MS = 1000;

constexpr uint8_t HOME_BUTTON_PIN = 27;
constexpr unsigned long BUTTON_DEBOUNCE_MS = 25;
constexpr unsigned long HOME_LONG_PRESS_MS = 3000;

constexpr int CAL_SAMPLES = 300;
constexpr int CAL_MAX_ATTEMPTS = CAL_SAMPLES * 5;

constexpr int OUT_RANGE = 64;
constexpr float ALPHA = 0.18;
constexpr float XY_THRESHOLD = 1.0;
constexpr float RESPONSE_RANGE = 10.0;
constexpr float SPEED_CURVE_BASE = 0.55;
constexpr int MIN_REPORT_MAGNITUDE = 2;

constexpr unsigned long IDLE_RELEASE_INTERVAL_MS = 250;
constexpr unsigned long MAX_CONTINUOUS_DRAG_MS = 3500;
constexpr unsigned long DRAG_COOLDOWN_MS = 120;
constexpr unsigned long RECENTER_IDLE_MS = 120;
constexpr uint16_t DRAG_HISTORY_SIZE = 384;
constexpr uint16_t MAX_REPLAY_SAMPLES_PER_CALL = DRAG_HISTORY_SIZE;
}
