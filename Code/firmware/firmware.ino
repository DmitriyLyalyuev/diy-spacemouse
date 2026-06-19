#include <USB.h>
#include <CoreMutex.h>
#include <Wire.h>
#include <Tlv493d.h>
#include <math.h>
#include <tusb-hid.h>
#include "class/hid/hid_device.h"

// Faster polling makes replay/recenter complete sooner.
int usb_hid_poll_interval = 1;

static const uint8_t hidReportDesc[] = {
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1)),
  TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(2))
};

uint8_t hidLocalId = 0;
const uint8_t KEYBOARD_REPORT_ID = 1;
const uint8_t MOUSE_REPORT_ID = 2;

const uint8_t MOUSE_MIDDLE_BUTTON = 0x04;

const uint8_t TLV493D_ADDR = 0x5E;
const unsigned long SERIAL_WAIT_MS = 1000;

Tlv493d mag = Tlv493d();

const uint8_t HOME_BUTTON_PIN = 27;
const unsigned long BUTTON_DEBOUNCE_MS = 25;
bool homeLastReading = HIGH;
bool homeStableState = HIGH;
unsigned long homeLastChangeMs = 0;

const int calSamples = 300;
const int CAL_MAX_ATTEMPTS = calSamples * 5;
const int sensivity = 8;
const int magRange = 3;
const int outRange = 64;
const int inRange = magRange * sensivity;

const float alpha = 0.18;
const float xyThreshold = 1.0;
const float responseRange = 10.0;
const float speedCurveBase = 0.55;
const int minReportMagnitude = 2;

const unsigned long IDLE_RELEASE_INTERVAL_MS = 250;
const unsigned long MAX_CONTINUOUS_DRAG_MS = 3500;
const unsigned long DRAG_COOLDOWN_MS = 120;

uint8_t mouseButtons = 0;

bool movementActive = false;
bool recenterActive = false;
unsigned long lastMovementMs = 0;
unsigned long dragStartMs = 0;
unsigned long lastIdleReleaseMs = 0;
unsigned long dragCooldownUntilMs = 0;
const unsigned long RECENTER_IDLE_MS = 120;
const uint16_t DRAG_HISTORY_SIZE = 384;
const uint16_t MAX_REPLAY_SAMPLES_PER_CALL = DRAG_HISTORY_SIZE;

struct DragSample
{
  int8_t x;
  int8_t y;
};

DragSample dragHistory[DRAG_HISTORY_SIZE];
uint16_t dragHistoryCount = 0;
uint16_t dragHistoryStart = 0;

float xOffset = 0.0;
float yOffset = 0.0;
float zOffset = 0.0;

float rawX = 0.0;
float rawY = 0.0;
float rawZ = 0.0;

float xCurrent = 0.0;
float yCurrent = 0.0;
float zCurrent = 0.0;

bool isOrbit = false;
unsigned long lastDebugMs = 0;
unsigned long lastSensorErrorMs = 0;

void setup()
{
  Serial.begin(115200);
  unsigned long serialStart = millis();
  while (!Serial && millis() - serialStart < SERIAL_WAIT_MS)
  {
    delay(10);
  }

  Serial.println("boot fusion-spacemouse");

  beginHid();

  pinMode(HOME_BUTTON_PIN, INPUT_PULLUP);
  homeLastReading = digitalRead(HOME_BUTTON_PIN);
  homeStableState = homeLastReading;
  homeLastChangeMs = millis();
  Serial.println("home button pin=27");

  Wire1.begin();
  Wire1.setClock(100000);

  mag.begin(Wire1);

  Serial.println("after begin kick");
  dumpRawTlv("raw");
  delay(50);

  uint8_t modeErr = mag.setAccessMode(mag.MASTERCONTROLLEDMODE);
  Serial.print("setAccessMode err=");
  Serial.println(modeErr);
  if (modeErr != 0)
  {
    dumpRawTlv("raw");
    delay(50);
    modeErr = mag.setAccessMode(mag.MASTERCONTROLLEDMODE);
    Serial.print("setAccessMode retry err=");
    Serial.println(modeErr);
  }
  delay(20);

  mag.disableTemp();

  Serial.println("config raw dump");
  dumpRawTlv("raw");

  printPreCalibrationSamples();
  calibrateSensor();

  releaseKeyboard();
  mouseButtons = 0;
  sendMouseReport(0, 0, 0, 0);
  Serial.println("hid ready");


}

void loop()
{
  handleHomeButton();

  delay(mag.getMeasurementDelay());
  handleHomeButton();
  Tlv493d_Error_t err = updateSensor();

  if (err != TLV493D_NO_ERROR)
  {
    printSensorError(err);
    releaseMotionOnly();
    releaseShift();
    clearDragHistory();
    recenterActive = false;
    return;
  }

  applyFilter();

  int xMove = 0;
  int yMove = 0;
  int xOut = 0;
  int yOut = 0;
  bool sentMovement = false;
  unsigned long now = millis();

  if (now >= dragCooldownUntilMs && (fabs(xCurrent) > xyThreshold || fabs(yCurrent) > xyThreshold))
  {
    if (recenterActive)
    {
      recenterActive = false;
      clearDragHistory();
    }

    xMove = scaleAxis(xCurrent);
    yMove = scaleAxis(yCurrent);

    // Axis dominance keeps mostly horizontal/vertical tilts from producing accidental diagonal orbit.
    if (xMove != 0 && yMove != 0)
    {
      if (abs(xMove) > abs(yMove) * 2)
      {
        yMove = 0;
      }
      else if (abs(yMove) > abs(xMove) * 2)
      {
        xMove = 0;
      }
    }

    // Do not hold MMB for values that were swallowed by the coarse deadzone.
    if (xMove != 0 || yMove != 0)
    {
      // Z is intentionally ignored here; automatic pan/orbit switching made Fusion movement unpredictable.
      holdShift();

      if (!movementActive)
      {
        mouseButtons = MOUSE_MIDDLE_BUTTON;
        movementActive = true;
        dragStartMs = now;
      }

      if (movementActive && now - dragStartMs >= MAX_CONTINUOUS_DRAG_MS)
      {
        breakMotionLatch();
      }
      else
      {
        // Orbit uses direct XY mapping; the old swapped pan mapping made rotation direction unintuitive.
        // Invert Y so physical up/down matches Fusion orbit expectation.
        xOut = xMove;
        yOut = -yMove;
        sendMouseReport(xOut, yOut, 0, 0);
        recordDragSample(xOut, yOut);
        lastMovementMs = now;
        sentMovement = true;
      }
    }
  }

  if (!sentMovement && now - lastMovementMs >= RECENTER_IDLE_MS)
  {
    if (movementActive)
    {
      stopMotion();
    }
    else if (recenterActive)
    {
      replayRecenter();
    }
  }

  if (!sentMovement)
  {
    sendIdleReleaseIfNeeded();
  }

  printDebug(xMove, yMove, xOut, yOut);
}

void printHexByte(uint8_t value)
{
  if (value < 0x10)
  {
    Serial.print('0');
  }
  Serial.print(value, HEX);
}

void dumpRawTlv(const char *label)
{
  uint8_t n = Wire1.requestFrom(TLV493D_ADDR, (uint8_t)10);

  Serial.print(label);
  Serial.print(" n=");
  Serial.print(n);
  Serial.print(":");

  while (Wire1.available())
  {
    Serial.print(' ');
    printHexByte(Wire1.read());
  }

  Serial.println();
}

void beginHid()
{
  USB.disconnect();
  hidLocalId = USB.registerHIDDevice(hidReportDesc, sizeof(hidReportDesc), 10, 0x0003);
  USB.connect();
  delay(1000);

  Serial.print("hid local id=");
  Serial.print(hidLocalId);
  Serial.println(" keyboardReportId=1 mouseReportId=2");
}

void printPreCalibrationSamples()
{
  for (int i = 0; i < 5; i++)
  {
    delay(mag.getMeasurementDelay());
    Tlv493d_Error_t err = updateSensor();

    Serial.print("pre raw=");
    Serial.print(rawX, 4);
    Serial.print(",");
    Serial.print(rawY, 4);
    Serial.print(",");
    Serial.print(rawZ, 4);
    Serial.print(" err=");
    Serial.println((int)err);
  }
}

void calibrateSensor()
{
  Serial.println("calibrating");

  xOffset = 0.0;
  yOffset = 0.0;
  zOffset = 0.0;

  int validSamples = 0;
  int attempts = 0;
  while (validSamples < calSamples && attempts < CAL_MAX_ATTEMPTS)
  {
    attempts++;
    delay(mag.getMeasurementDelay());
    Tlv493d_Error_t err = updateSensor();

    if (err != TLV493D_NO_ERROR)
    {
      continue;
    }

    xOffset += rawX;
    yOffset += rawY;
    zOffset += rawZ;
    validSamples++;
  }

  if (validSamples == 0)
  {
    Serial.println("calibration failed");
    return;
  }

  xOffset /= validSamples;
  yOffset /= validSamples;
  zOffset /= validSamples;

  if (validSamples < calSamples)
  {
    Serial.print("cal samples=");
    Serial.println(validSamples);
  }

  Serial.print("offset=");
  Serial.print(xOffset, 4);
  Serial.print(",");
  Serial.print(yOffset, 4);
  Serial.print(",");
  Serial.println(zOffset, 4);
}

Tlv493d_Error_t updateSensor()
{
  Tlv493d_Error_t err = mag.updateData();

  if (err == TLV493D_NO_ERROR)
  {
    rawX = mag.getX();
    rawY = mag.getY();
    rawZ = mag.getZ();
  }

  return err;
}

void applyFilter()
{
  xCurrent = filterAxis(xCurrent, rawX - xOffset);
  yCurrent = filterAxis(yCurrent, rawY - yOffset);
  zCurrent = filterAxis(zCurrent, rawZ - zOffset);
}

float filterAxis(float current, float value)
{
  return current + alpha * (value - current);
}

int scaleAxis(float value)
{
  if (fabs(value) <= xyThreshold)
  {
    return 0;
  }

  // The response curve keeps the center calm but adds speed as deflection increases.
  float magnitude = fabs(value) - xyThreshold;
  float normalized = constrain(magnitude / responseRange, 0.0, 1.0);
  float curved = normalized * (speedCurveBase + (1.0 - speedCurveBase) * normalized);
  int output = (int)lround(curved * outRange);

  if (output < minReportMagnitude)
  {
    return 0;
  }

  output = constrain(output, -outRange, outRange);
  return value < 0 ? -output : output;
}

void releaseMotionOnly()
{
  // Repeated release reports recover from a missed USB button-up packet.
  mouseButtons = 0;
  sendMouseReport(0, 0, 0, 0);
  movementActive = false;
}

void stopMotion()
{
  releaseMotionOnly();
  releaseShift();
  if (dragHistoryCount > 0)
  {
    recenterActive = true;
    replayRecenter();
  }
}

void sendIdleReleaseIfNeeded()
{
  if (movementActive || recenterActive)
  {
    return;
  }

  unsigned long now = millis();
  if (now - lastIdleReleaseMs < IDLE_RELEASE_INTERVAL_MS)
  {
    return;
  }

  // Idle release heartbeat clears a stuck host-side MMB/Shift state.
  releaseMotionOnly();
  releaseShift();
  lastIdleReleaseMs = now;
}

void breakMotionLatch()
{
  // The watchdog breaks an endless drag if the neutral position is misdetected.
  releaseMotionOnly();
  releaseShift();
  clearDragHistory();
  recenterActive = false;
  dragStartMs = 0;
  dragCooldownUntilMs = millis() + DRAG_COOLDOWN_MS;
  Serial.println("watchdog release");
}

void clearDragHistory()
{
  dragHistoryCount = 0;
  dragHistoryStart = 0;
}

void recordDragSample(int8_t x, int8_t y)
{
  if (x == 0 && y == 0)
  {
    return;
  }

  uint16_t index = (dragHistoryStart + dragHistoryCount) % DRAG_HISTORY_SIZE;
  if (dragHistoryCount == DRAG_HISTORY_SIZE)
  {
    dragHistoryStart = (dragHistoryStart + 1) % DRAG_HISTORY_SIZE;
    index = (dragHistoryStart + dragHistoryCount - 1) % DRAG_HISTORY_SIZE;
  }
  else
  {
    dragHistoryCount++;
  }

  dragHistory[index].x = x;
  dragHistory[index].y = y;
}

void replayRecenter()
{
  if (dragHistoryCount == 0)
  {
    recenterActive = false;
    return;
  }

  mouseButtons = 0;
  sendMouseReport(0, 0, 0, 0);
  movementActive = false;
  releaseShift();

  // Replaying recorded small deltas is used because summed drift chunks are distorted by pointer acceleration.
  uint16_t samples = 0;
  while (dragHistoryCount > 0 && samples < MAX_REPLAY_SAMPLES_PER_CALL)
  {
    uint16_t index = (dragHistoryStart + dragHistoryCount - 1) % DRAG_HISTORY_SIZE;
    DragSample sample = dragHistory[index];
    dragHistoryCount--;

    sendMouseReport(-sample.x, -sample.y, 0, 0);
    samples++;
    delay(1);
  }

  if (dragHistoryCount == 0)
  {
    recenterActive = false;
    dragHistoryStart = 0;
  }
}

void holdShift()
{
  if (isOrbit)
  {
    return;
  }

  uint8_t keys[6] = { 0, 0, 0, 0, 0, 0 };
  sendKeyboardReport(KEYBOARD_MODIFIER_LEFTSHIFT, keys);
  isOrbit = true;
}

void releaseShift()
{
  releaseKeyboard();
  isOrbit = false;
}

void releaseKeyboard()
{
  uint8_t keys[6] = { 0, 0, 0, 0, 0, 0 };
  sendKeyboardReport(0, keys);
}

void sendKeyboardReport(uint8_t modifiers, uint8_t keys[6])
{
  CoreMutex m(&USB.mutex);
  tud_task();
  if (USB.HIDReady())
  {
    tud_hid_keyboard_report(KEYBOARD_REPORT_ID, modifiers, keys);
  }
  tud_task();
}

void sendMouseReport(int8_t x, int8_t y, int8_t wheel, int8_t pan)
{
  CoreMutex m(&USB.mutex);
  tud_task();
  if (USB.HIDReady())
  {
    tud_hid_mouse_report(MOUSE_REPORT_ID, mouseButtons, x, y, wheel, pan);
  }
  tud_task();
}

void printSensorError(Tlv493d_Error_t err)
{
  unsigned long now = millis();

  if (now - lastSensorErrorMs < 100)
  {
    return;
  }

  lastSensorErrorMs = now;
  Serial.print("sensor error=");
  Serial.println((int)err);
}

void printDebug(int xMove, int yMove, int xOut, int yOut)
{
  unsigned long now = millis();

  if (now - lastDebugMs < 100)
  {
    return;
  }

  lastDebugMs = now;

  Serial.print("raw=");
  Serial.print(rawX, 4);
  Serial.print(",");
  Serial.print(rawY, 4);
  Serial.print(",");
  Serial.print(rawZ, 4);
  Serial.print(" cur=");
  Serial.print(xCurrent, 4);
  Serial.print(",");
  Serial.print(yCurrent, 4);
  Serial.print(",");
  Serial.print(zCurrent, 4);
  Serial.print(" move=");
  Serial.print(xMove);
  Serial.print(",");
  Serial.print(yMove);
  Serial.print(" out=");
  Serial.print(xOut);
  Serial.print(",");
  Serial.print(yOut);
  Serial.print(" hist=");
  Serial.print(dragHistoryCount);
  Serial.print(" recenter=");
  Serial.print(recenterActive ? 1 : 0);
  Serial.print(" active=");
  Serial.print(movementActive ? 1 : 0);
  Serial.println(" mode=fusion-spacemouse");
}

void handleHomeButton()
{
  // Raw edge handling is used because the second button line is stuck low on this build.
  bool reading = digitalRead(HOME_BUTTON_PIN);
  unsigned long now = millis();

  if (reading != homeLastReading)
  {
    homeLastReading = reading;
    homeLastChangeMs = now;
  }

  if (now - homeLastChangeMs < BUTTON_DEBOUNCE_MS || reading == homeStableState)
  {
    return;
  }

  homeStableState = reading;
  if (homeStableState == LOW)
  {
    goHome();
  }
}

void goHome()
{
  releaseMotionOnly();
  releaseShift();
  clearDragHistory();
  recenterActive = false;
  dragStartMs = 0;
  dragCooldownUntilMs = millis() + DRAG_COOLDOWN_MS;

  // Fusion AddIn handles Cmd+Shift+N as the home-view command.
  uint8_t keys[6] = { HID_KEY_N, 0, 0, 0, 0, 0 };
  sendKeyboardReport(KEYBOARD_MODIFIER_LEFTGUI | KEYBOARD_MODIFIER_LEFTSHIFT, keys);
  delay(30);
  releaseKeyboard();
  isOrbit = false;
  Serial.println("pressed cmd-shift-n");
}
