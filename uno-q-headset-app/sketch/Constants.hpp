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
 *  File: Constants.hpp
 *  Author: Noah Haskell
 *  Description: Constant values for The Headset are stored here for good organization purposes.
 */

#ifndef HEADSET_CONSTANTS_HPP
#define HEADSET_CONSTANTS_HPP

#include <Arduino.h>

namespace Constants {

// Debug mode - print out to serial if true (1)
#define DEBUG 1

#if DEBUG == 1
#define outputDebug(x) do { Serial.print(x); } while (0)
#define outputDebugLine(x) do { Serial.println(x); } while (0)
#else
#define outputDebug(x) ((void)0)
#define outputDebugLine(x) ((void)0)
#endif

constexpr uint8_t LED_PIN = LED_BUILTIN;

constexpr uint8_t ONE_WIRE_BUS_LEFT = 9;
constexpr uint8_t ONE_WIRE_BUS_RIGHT = 12;
constexpr double THERMOMETER_PERIOD_MS = 1000.0;

const int MOTOR_A_PINS[3] = {7, 8, 5};
const int MOTOR_B_PINS[3] = {10, 11, 6};

constexpr uint8_t MOTOR_COUNT = 2;

constexpr bool INVERT_MOTOR_A = false;
constexpr bool INVERT_MOTOR_B = false;

constexpr int TO_HIGH_POS_OUTPUT = 255;
constexpr int TO_LOW_POS_OUTPUT = 255;

constexpr uint8_t LEFT_BUTTON = A2;
constexpr uint8_t RIGHT_BUTTON = A3;
constexpr uint8_t E_STOP_BUTTON = A4;
constexpr bool BUTTON_PULLUP = false;
constexpr uint16_t BUTTON_DEBOUNCE_RISING_MS = 50;
constexpr uint16_t BUTTON_DEBOUNCE_FALLING_MS = 50;

}  // namespace Constants

#endif
