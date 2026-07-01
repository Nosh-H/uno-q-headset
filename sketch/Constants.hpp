#ifndef HEADSET_CONSTANTS_HPP
#define HEADSET_CONSTANTS_HPP

#include <Arduino.h>

namespace headset_config {

constexpr uint8_t LED_PIN = LED_BUILTIN;

constexpr uint8_t ONE_WIRE_BUS_LEFT = 4;
constexpr uint8_t ONE_WIRE_BUS_RIGHT = 10;
constexpr double THERMOMETER_PERIOD_MS = 1000.0;

constexpr uint8_t FIRST_ENCODER_A_PIN = 2;
constexpr uint8_t FIRST_ENCODER_B_PIN = 5;
constexpr uint8_t SECOND_ENCODER_A_PIN = 6;
constexpr uint8_t SECOND_ENCODER_B_PIN = 7;
constexpr uint8_t MOTOR_COUNT = 2;

constexpr int DIRECTION_PINS[MOTOR_COUNT] = {12, 13};
constexpr int PWM_PINS[MOTOR_COUNT] = {3, 11};
constexpr int BRAKE_PINS[MOTOR_COUNT] = {9, 8};
constexpr bool INVERT_MOTOR = false;

constexpr int HIGH_POS = 800;
constexpr int LOW_POS = -50;
constexpr int STATIC_GAIN[MOTOR_COUNT] = {255, 73};

constexpr uint8_t LEFT_BUTTON = A0;
constexpr uint8_t RIGHT_BUTTON = A1;
constexpr uint8_t HOME_BUTTON = A2;
constexpr bool BUTTON_PULLUP = false;
constexpr uint16_t BUTTON_DEBOUNCE_RISING_MS = 50;
constexpr uint16_t BUTTON_DEBOUNCE_FALLING_MS = 50;

}  // namespace headset_config

#endif
