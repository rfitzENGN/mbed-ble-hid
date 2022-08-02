///
///   ble_mouse.ino
///
///   created: 2020-05
///   
///  Create a wireless Human Interface Device (HID) using the Bluetooth Low-Energy (BLE) 
///  HID-over-GATT Profile (HOGP) on a mbed stack (Arduino nano 33 BLE).
///

#include "Nano33BleHID.h"
#include "AnalogJoystick.h"
#include "signal_utils.h"

#define DEMO_ENABLE_RANDOM_INPUT        0
#define DEMO_DURATION_MS                4200

/* -------------------------------------------------------------------------- */

Nano33BleMouse bleMouse("nano33BLE Mouse");

// Analog Joystick wrapper, used to simulate a mouse.
AnalogJoystick gJoystick(A7, A6, 2);
static const float kJoystickSensibility = 0.125f;

// Builtin LED animation delays when disconnect. 
static const int kLedBeaconDelayMilliseconds = 1250;
static const int kLedErrorDelayMilliseconds = kLedBeaconDelayMilliseconds / 10;

// Builtin LED intensity when connected.
static const int kLedConnectedIntensity = 30;

/* -------------------------------------------------------------------------- */

void setup()
{
  // General setup.
  pinMode(LED_BUILTIN, OUTPUT);
  gJoystick.initialize();

  // Initialize both BLE and the HID.
  bleMouse.initialize();

  // Launch the event queue that will manage both BLE events and the loop. 
  // After this call the main thread will be halted.
  MbedBleHID_RunEventThread();
Serial.begin(9600);
}

void loop()
{
  // When disconnected, we animate the builtin LED to indicate the device state.
  if (bleMouse.connected() == false) {
    animateLED(LED_BUILTIN, (bleMouse.has_error()) ? kLedErrorDelayMilliseconds 
                                                   : kLedBeaconDelayMilliseconds);
    return;
  }

  // When connected, we slightly dim the builtin LED.
  analogWrite(LED_BUILTIN, kLedConnectedIntensity);
  
  // Read the analog joystick inputs.
  gJoystick.update();
  float fx = -(kJoystickSensibility * gJoystick.x());
  float fy = kJoystickSensibility * gJoystick.y();
  auto buttons = gJoystick.button() ? HIDMouseService::BUTTON_NONE 
                                    : HIDMouseService::BUTTON_LEFT;

//Serial.println(fx);
//Serial.println(fy);
Serial.println(buttons);  

  // When demo mode is enabled we bypass the captured values 
  // to output random motion for a few seconds instead.
#if DEMO_ENABLE_RANDOM_INPUT
  bool const bPlayDemo = true;// && (bleMouse.connection_time() < DEMO_DURATION_MS);
  if (bPlayDemo)
  {
    fx = kJoystickSensibility * randf(-1.0f, 1.0f);
    fy = kJoystickSensibility * randf(-1.0f, 1.0f);
    buttons = HIDMouseService::BUTTON_NONE;
  }
  else
  {
    fx = 0.0f;
    fy = 0.0f;
  }
#endif
 
  auto *mouse = bleMouse.hid();
 
  // Update the HID report.
  if(fx > 0.01 || fx < -0.01 || fy > 0.01 || fy < -0.01){
    mouse->motion(fx, fy);
  }
  else{
    mouse->motion(0,0);
  }
   mouse->button(buttons);
   mouse->SendReport();
}

/* -------------------------------------------------------------------------- */
