/* 
 *  Copyright (C) 2026 Noah Haskell
 *  
 *  This program is free software: you can redistribute it and/or modify it under the terms of the
 *  GNU General Public License as published by the Free Software Foundation, either version 3 of the
 *  License, or any later version.
 *  
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 *  even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program. If
 *  not, see <https://www.gnu.org/licenses/>.
 *  
 *  File: sketch.ino
 *  Author: Noah Haskell
 *  Description: "The Headset" is currently a prototype to automate the application of a cold compress on the wearer's eyes. 
 *  This is intended to treat eye itching and pain caused by allergies.
 *  This is the main code file for the Headset. Its key role is to contain the high level logic for the microcontroller.
 */

#include <Arduino.h>
#include <Arduino_RouterBridge.h>
// #include <util/atomic.h>
#include <DallasTemperature.h>
#include "DigitalInput.hpp"
#include "Constants.hpp"
#include <OneWire.h>
#include "Thermometer.hpp"
#include <zephyr/kernel.h> // Required for Zephyr RTOS Timer APIs

// I looked heavily at the current Arduino Bridge examples/docs.

bool ledState = false;
int currentMode = 0;

// Define Sensors
Thermometer leftThermo(headset_config::ONE_WIRE_BUS_LEFT, headset_config::THERMOMETER_PERIOD_MS);
Thermometer rightThermo(headset_config::ONE_WIRE_BUS_RIGHT, headset_config::THERMOMETER_PERIOD_MS);

// Encoders firstEncoder(headset_config::FIRST_ENCODER_A_PIN, headset_config::FIRST_ENCODER_B_PIN);
// Encoders secondEncoder(headset_config::SECOND_ENCODER_A_PIN, headset_config::SECOND_ENCODER_B_PIN); // the encoder objects could use analog pins

// Initial target position
int setpoints[headset_config::MOTOR_COUNT] = {headset_config::HIGH_POS, headset_config::HIGH_POS};

// For PID time calculation
long prevT = 0;

// Counter for printing position on the serial
int counter = 0;

// Define button pins
// https://forum.arduino.cc/t/using-analog-pins-for-push-buttons/309407/7
// Press this to zero the encoders. TODO: Make the press of this button start homing
// Records the button state. Either HIGH or LOW.
int leftButtonState = LOW;
int rightButtonState = LOW;
bool homeButtonState = LOW;
bool leftArmState = false; // HIGH = false (off eye)
bool rightArmState = false; // LOW = true (on eye)
DigitalInput leftButton(headset_config::LEFT_BUTTON, headset_config::BUTTON_PULLUP, headset_config::BUTTON_DEBOUNCE_RISING_MS, headset_config::BUTTON_DEBOUNCE_FALLING_MS); // Verify if there is no pullup resistor
DigitalInput rightButton(headset_config::RIGHT_BUTTON, headset_config::BUTTON_PULLUP, headset_config::BUTTON_DEBOUNCE_RISING_MS, headset_config::BUTTON_DEBOUNCE_FALLING_MS);
DigitalInput homeButton(headset_config::HOME_BUTTON, headset_config::BUTTON_PULLUP, headset_config::BUTTON_DEBOUNCE_RISING_MS, headset_config::BUTTON_DEBOUNCE_FALLING_MS); // Likely not needed if stepper is chosen

void setLed(bool on) {
  ledState = on;
  digitalWrite(headset_config::LED_PIN, on ? HIGH : LOW);
}

// Test Only (for the Bridge and python program)
int getSensor() {
  return leftButton.getDebounced();
}

/* Meant to be called by the bridge*/
bool getState(uint8_t side) {
  if (side == 1) {
    return rightArmState;
  } else if (side == 0) {
    return leftArmState;
  } else {
    return false;
  }
}

// Test Bridge with LED info
String getStatus() {
  // For real code, return a Bridge-supported structured type
  // if available, or simple values.
  return ledState ? "led_on" : "led_off";
}

/** Meant to be called by the bridge */
float getTemp(uint8_t side) {
  if (side == 1) {
    return rightThermo.getDegreesFahrenheit();
  } else if (side == 0) {
    return leftThermo.getDegreesFahrenheit();
  } else {
    return -999.9;
  }
}

void setMode(int mode) {
  currentMode = mode;
}

void setState(uint8_t side, bool state) {
  if (side == 1) {
    rightArmState = state;
  } else if (side == 0) {
    leftArmState = state;
  } else {
    Serial.println("TRIED TO CALL SETSTATE WITH INVALID SIDE");
  }
}

// Emergency stop
void stopAll() {
  setLed(false);
  currentMode = 0;
  // Also stop motors, relays, PWM outputs, etc.

}

void setup() {
  pinMode(headset_config::LED_PIN, OUTPUT);
  digitalWrite(headset_config::LED_PIN, LOW);
  leftThermo.setup();
  rightThermo.setup();

  // Register RPC-callable functions here. Template: 
  Bridge.begin();
  Bridge.provide("setLed", setLed);
  Bridge.provide("getSensor", getSensor);
  Bridge.provide("getState", getState);
  Bridge.provide("getStatus", getStatus);
  Bridge.provide("getTemp", getTemp);
  Bridge.provide("setMode", setMode);
  Bridge.provide("setState", setState);
  Bridge.provide("stopAll", stopAll);
}

void checkButtons() {
  if (leftButton.onRisingEdge()) {
    // If the button was just pressed, toggle the matching arm state
    leftArmState = !leftArmState;
  }

  if (rightButton.onRisingEdge()) {
    // If the button was just pressed, toggle the matching arm state
    rightArmState = !rightArmState;
  }
  
  // Zero the motor encoders right when home button is pressed
  if (homeButton.onRisingEdge()) {
    // resetEncoders();
  }
}

/** Applies the state to the motor setpoints. Should be called periodically. */
void updateSetpoints() {
  setpoints[0] = (leftArmState ? headset_config::LOW_POS : headset_config::HIGH_POS);
  setpoints[1] = (rightArmState ? headset_config::LOW_POS: headset_config::HIGH_POS);
}

void periodic() {
  // Keep time-critical hardware behavior here
  // Do not depend on web requests for safety-critical timing.
  // Bridge polling/update call if required by the library.

  // Can put thermometer calls into a periodic() called less often
  leftThermo.periodic();
  rightThermo.periodic();

  checkButtons();
  // Potentially implement checking for updates from main.py
  // but probably unnecessary as methods are given to the bridge to update global variables instead
  updateSetpoints();

}

void loop() {
  periodic();
}
