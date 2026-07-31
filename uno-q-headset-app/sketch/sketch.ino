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
#include "DCMotor.hpp"
#include "DigitalInput.hpp"
#include "Constants.hpp"
#include <OneWire.h>
#include "Thermometer.hpp"
#include <zephyr/kernel.h> // Required for Zephyr RTOS Timer APIs

bool ledState = false;
int currentMode = 0;

// Define Sensors
Thermometer leftThermo(headset_config::ONE_WIRE_BUS_LEFT, headset_config::THERMOMETER_PERIOD_MS);
Thermometer rightThermo(headset_config::ONE_WIRE_BUS_RIGHT, headset_config::THERMOMETER_PERIOD_MS);

// Initial target outputs - high {left, right}
int setpoints[headset_config::MOTOR_COUNT] = {headset_config::TO_HIGH_POS_OUTPUT, headset_config::TO_HIGH_POS_OUTPUT};

// Global DCMotor instances: safe because constructor does not call Arduino APIs.
DCMotor motorA(headset_config::MOTOR_A_PINS, 3, headset_config::INVERT_MOTOR_A);
DCMotor motorB(headset_config::MOTOR_B_PINS, 3, headset_config::INVERT_MOTOR_B);

// Define button pins to toggle arms
// https://forum.arduino.cc/t/using-analog-pins-for-push-buttons/309407/7
// TODO: Convert the "HOME" button to an Emergency Stop Button
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
bool getState(int side) {
  Serial.println("GET STATE CALLED");
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
float getTemp(int side) {
  Serial.println("GET TEMP CALLED");
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

void setState(int side, bool state) {
  Serial.println("SET STATE CALLED");
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
  Serial.println("EMERGENCY STOP");
  setLed(false);
  currentMode = 0;
  // Also stop motors, relays, PWM outputs, etc.

}

void setup() {
  pinMode(headset_config::LED_PIN, OUTPUT);
  digitalWrite(headset_config::LED_PIN, LOW);
  leftThermo.setup();
  rightThermo.setup();
  motorA.begin();
  motorB.begin();
  Serial.begin(9600);

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

/** Applies the state to the motor setpoints and runs motors. Should be called periodically. */
void updateSetpoints() {
  
  if (leftArmState) {
    motorA.run_motor(-1, setpoints[0]);
  } else {
    motorA.run_motor(1, setpoints[0]);
  }
  
  if (rightArmState) {
    motorB.run_motor(-1, setpoints[1]);
  } else {
    motorB.run_motor(1, setpoints[1]);
  }

  // TODO: Delete when Uno Q timer code replaces loop()
  delay(200);
}

void periodic() {
  // Keep time-critical hardware behavior here
  // Do not depend on web requests for safety-critical timing.

  // Can put thermometer calls into a periodic() called less often
  leftThermo.periodic();
  rightThermo.periodic();

  // Check buttons and update state as necessary
  checkButtons();
  // Apply motor output based on arm states
  updateSetpoints();

}

void loop() {
  periodic();
}
