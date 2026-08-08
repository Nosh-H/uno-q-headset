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
Thermometer leftThermo(Constants::ONE_WIRE_BUS_LEFT, Constants::THERMOMETER_PERIOD_MS);
Thermometer rightThermo(Constants::ONE_WIRE_BUS_RIGHT, Constants::THERMOMETER_PERIOD_MS);

// Initial target outputs - high {left, right}
int setpoints[Constants::MOTOR_COUNT] = {Constants::TO_HIGH_POS_OUTPUT, Constants::TO_HIGH_POS_OUTPUT};

// Global DCMotor instances: safe because constructor does not call Arduino APIs.
DCMotor motorA(Constants::MOTOR_A_PINS, 3, Constants::INVERT_MOTOR_A);
DCMotor motorB(Constants::MOTOR_B_PINS, 3, Constants::INVERT_MOTOR_B);

// Define button pins to toggle arms
// https://forum.arduino.cc/t/using-analog-pins-for-push-buttons/309407/7
bool leftArmState = false; // HIGH = false (off eye)
bool rightArmState = false; // LOW = true (on eye)
DigitalInput leftButton(Constants::LEFT_BUTTON, Constants::BUTTON_PULLUP, Constants::BUTTON_DEBOUNCE_RISING_MS, Constants::BUTTON_DEBOUNCE_FALLING_MS); // Verify if there is no pullup resistor
DigitalInput rightButton(Constants::RIGHT_BUTTON, Constants::BUTTON_PULLUP, Constants::BUTTON_DEBOUNCE_RISING_MS, Constants::BUTTON_DEBOUNCE_FALLING_MS);
DigitalInput eStopButton(Constants::E_STOP_BUTTON, Constants::BUTTON_PULLUP, Constants::BUTTON_DEBOUNCE_RISING_MS, Constants::BUTTON_DEBOUNCE_FALLING_MS);
bool eStopEnabled = false;

void setLed(bool on) {
  ledState = on;
  digitalWrite(Constants::LED_PIN, on ? LOW : HIGH);
}

// Test Only (for the Bridge and python program)
int getSensor() {
  return leftButton.getDebounced();
}

/* Meant to be called by the bridge*/
bool getState(int side) {
  outputDebugLine("GET STATE CALLED");
  if (side == 1) {
    return rightArmState;
  } else if (side == 0) {
    return leftArmState;
  } else {
    return false;
  }
}

/** 
 * Test Bridge - Allows web app to perform a simple test to verify successful connection to MCU.
 * 
 * @return Arduino's millis(), rounded to the nearest millisecond
 */
int getStatus() {
  return (int) millis();
}

/** Meant to be called by the bridge */
float getTemp(int side) {
  outputDebugLine("GET TEMP CALLED");
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
  outputDebugLine("SET STATE CALLED");
  if (side == 1) {
    rightArmState = state;
  } else if (side == 0) {
    leftArmState = state;
  } else {
    outputDebugLine("TRIED TO CALL SETSTATE WITH INVALID SIDE");
  }
}

// Emergency stop
void stopAll() {
  outputDebugLine("EMERGENCY STOP");
  setLed(true);
  currentMode = 0;
  // Coast Motors
  motorA.run_motor(0, 0);
  motorB.run_motor(0, 0);
  eStopEnabled = true;
}

// End emergency stop
void resumeAll() {
  outputDebugLine("DISABLE EMERGENCY STOP");
  setLed(false);
  eStopEnabled = false;
}

void setup() {
  pinMode(Constants::LED_PIN, OUTPUT);
  digitalWrite(Constants::LED_PIN, LOW);
  leftThermo.setup();
  rightThermo.setup();
  motorA.begin();
  motorB.begin();
  leftButton.setup();
  rightButton.setup();
  eStopButton.setup();
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
  Bridge.provide("resumeAll", resumeAll);
}

void checkButtons() {
  leftButton.periodic();
  rightButton.periodic();
  eStopButton.periodic();

  if (leftButton.onRisingEdge()) {
    // If the button was just pressed, toggle the matching arm state
    leftArmState = !leftArmState;
  }

  if (rightButton.onRisingEdge()) {
    // If the button was just pressed, toggle the matching arm state
    rightArmState = !rightArmState;
  }
  
  // Emergency stop toggle
  if (eStopButton.onRisingEdge()) {
    if (eStopEnabled) {
      resumeAll();
    } else {
      stopAll();
    }
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
  delay(500);
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
  if (!eStopEnabled) {
    updateSetpoints();
  }

}

void loop() {
  periodic();
}
