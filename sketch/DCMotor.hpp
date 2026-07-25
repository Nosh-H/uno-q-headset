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
 *  File: DCMotor.hpp
 *  Author: Noah Haskell
 *  Date: 24 July 2026
 *  Decription: A class to control a DC motor with a L293D motor driver. Currently not using an encoder for closed loop control.
 */
#ifndef DCMOTOR_HPP
#define DCMOTOR_HPP

#include <Arduino.h>

/** 
 *  Single-file DCMotor implementation (header + inline methods)
 */
class DCMotor
{
public:
    /**
     *  @param pins Array containing at least [directionPin1, directionPin2, pwmPin] in that order
     *  @brief Constructor: lightweight, does NOT call Arduino hardware functions.
     *  Call `begin()` from setup() to configure pins and hardware.
     *  @param pin_count Number of pins - probably up to three
     *  @param invert_direction Whether the direction that is forward should be flipped
     */
    DCMotor(const int *pins, int pin_count, bool invert_direction = false)
        : pinIDs(nullptr), pin_count_(0), target(0), invert(invert_direction), direction(0)
    {
        if (pins == nullptr || pin_count < 2)
        {
            pinIDs = nullptr;
            pin_count_ = 0;
            return;
        }
        pin_count_ = pin_count;
        pinIDs = new int[pin_count_];
        for (int i = 0; i < pin_count_; ++i)
        {
            pinIDs[i] = pins[i];
        }
    }

    /**
     *  @brief Initialize hardware — safe to call from `setup()` when Arduino APIs are ready.
     */
    void begin()
    {
        if (pinIDs == nullptr || pin_count_ < 2)
            return;
        if (pin_count_ > 0)
            pinMode(pinIDs[0], OUTPUT);
        if (pin_count_ > 1)
            pinMode(pinIDs[1], OUTPUT);
        if (pin_count_ > 2)
            pinMode(pinIDs[2], OUTPUT);

    }

    /**
     *  @brief Deleted default copy semantics to avoid double-free on pinIDs.
     *  These two lines of code in C++ prevent the DCMotor class from being copied or assigned, making it a non-copyable class.
     *  This is a common design pattern used for classes that manage unique resources, such as hardware interfaces,
     *  to ensure that only one object controls a specific DC motor at a time.
     */
    DCMotor(const DCMotor &) = delete;
    DCMotor &operator=(const DCMotor &) = delete;

    ~DCMotor()
    {
        if (pinIDs)
        {
            delete[] pinIDs;
            pinIDs = nullptr;
        }
    }

    void set_target(int new_target) { target = new_target; }
    int get_target() const { return target; }
    void set_direction(int dir) { direction = dir; }

    /** 
     *  @brief method for convenience: run motor using provided dir and pwm value.
     *  @param dir Direction of output, 1 is forward, -1 is reverse, 0 is coast
     *  @param pwmVal Magnitude of output - Pulse Width Modulation - 0 min, 255 max
     */
    void run_motor(int dir, int pwmVal)
    {
        if (pinIDs == nullptr || pin_count_ < 2)
            return; // pins not configured

        // clamp pwm between 0-255 for Arduino analogWrite
        int pwm = pwmVal;
        if (pwm < 0)
            pwm = 0;
        if (pwm > 255)
            pwm = 255;

        if (invert)
            dir = -dir;

        if (pwm == 0) 
        {
            // Coast
            analogWrite(pinIDs[2], 0);
            digitalWrite(pinIDs[0], HIGH);
            digitalWrite(pinIDs[1], LOW);
        }
        else if (dir == 1)
        {
            // Forward direction
            digitalWrite(pinIDs[0], HIGH);
            digitalWrite(pinIDs[1], LOW);
            analogWrite(pinIDs[2], pwm);
        }
        else if (dir == -1)
        {
            // Reverse direction
            digitalWrite(pinIDs[0], LOW);
            digitalWrite(pinIDs[1], HIGH);
            analogWrite(pinIDs[2], pwm);
        }
        else
        {
            // Brake motor
            digitalWrite(pinIDs[0], LOW);
            digitalWrite(pinIDs[1], HIGH);
            analogWrite(pinIDs[2], 0);
        }
    }

    /** 
     *  @brief Backwards-compatible no-arg run; uses stored `direction` and `target` (as pwm)
     */
    void run_motor()
    {
        run_motor(direction, target);
    }

private:
    double position[2] = {0.0, 0.0};
    double velocity[2] = {0.0, 0.0};
    int *pinIDs;
    int pin_count_;
    int target;
    bool invert;
    int direction;

};

#endif // DCMOTOR_HPP