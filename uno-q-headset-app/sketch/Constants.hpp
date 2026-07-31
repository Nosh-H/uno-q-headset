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

namespace headset_config {

constexpr uint8_t LED_PIN = LED_BUILTIN;

constexpr uint8_t ONE_WIRE_BUS_LEFT = 4;
constexpr uint8_t ONE_WIRE_BUS_RIGHT = 10;
constexpr double THERMOMETER_PERIOD_MS = 1000.0;

// NO LONGER USING PIN 10 FOR DIRECTION - TESTED TO NOT WORK (likely due to thermometer pin conflict)
const int MOTOR_A_PINS[3] = {8, 9, 5};
const int MOTOR_B_PINS[3] = {11, 12, 6};

constexpr uint8_t MOTOR_COUNT = 2;

constexpr bool INVERT_MOTOR_A = false;
constexpr bool INVERT_MOTOR_B = false;

constexpr int TO_HIGH_POS_OUTPUT = 200;
constexpr int TO_LOW_POS_OUTPUT = 200;

constexpr uint8_t LEFT_BUTTON = A0;
constexpr uint8_t RIGHT_BUTTON = A1;
constexpr uint8_t E_STOP_BUTTON = A2;
constexpr bool BUTTON_PULLUP = false;
constexpr uint16_t BUTTON_DEBOUNCE_RISING_MS = 50;
constexpr uint16_t BUTTON_DEBOUNCE_FALLING_MS = 50;

}  // namespace headset_config

#endif
