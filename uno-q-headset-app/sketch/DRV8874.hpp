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
 *  File: DRV8874.hpp
 *  Author: Noah Haskell
 *  Date: 9 August 2026
 *  Decription: A class to control one DC motor with a DRV8874 motor driver. Currently only supporting PWM control mode. Planned to implement current sensing, may implement PHEN.
 *  Sources: https://github.com/szolotykh/DRV8874-breakout-board, https://www.ti.com/lit/ds/symlink/drv8874.pdf, https://www.pololu.com/product/4035
 */
#ifndef DRV8874_HPP
#define DRV8874_HPP

#include <Arduino.h>
#include "Constants.hpp"

/** 
 *  Single-file DRV8874 controlling a DC motor implementation (header + inline methods)
 */
class DRV8874
{
public:
    /**
     *  @param pins Array containing at least [controlPin1, controlPin2] and maybe also [... currentPin, sleepPin] in that order.
     *  Note: if in PHEN mode, the first pin is direction, the second is output
     *  @brief Constructor: lightweight, does NOT call Arduino hardware functions.
     *  Call `begin()` from setup() to configure pins and hardware.
     *  @param pin_count Number of pins - probably up to three
     *  @param invertDirection Whether the direction that is forward should be flipped
     *  @param mode Whether the motor is to be PHEN (false) or PWM (true) controlled.
     */
    DRV8874(const int *pins, int pin_count, bool invertDirection = false, bool mode = true)
        : pinIDs(nullptr), pin_count_(0), output(0), invert(invertDirection), mode(mode)
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

        if (pin_count_ > 3) {
            // Sets sleep pin to LOW as soon as possible
            pinMode(pinIDs[3], OUTPUT);
            digitalWrite(pinIDs[3], LOW);
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
        if (pin_count_ > 1) {
            pinMode(pinIDs[1], OUTPUT);
            // Stop motor during startup
            if (mode) {
                // The first pin must also be set to zero in PWM mode
                analogWrite(pinIDs[0], 0);
            }
            analogWrite(pinIDs[1], 0);
        }
        if (pin_count_ > 2) {
            // Current sensing pin. pinMode(..., INPUT) is technically not necessary for analog pins, but still doing it here.
            pinMode(pinIDs[2], INPUT);
        }
        if (pin_count_ > 3) {
            // Sleep pin - set to high (3.3V on the Uno Q)
            digitalWrite(pinIDs[3], HIGH);
        }
    }

    /**
     *  @brief Deleted default copy semantics to avoid double-free on pinIDs.
     *  These two lines of code in C++ prevent the DRV8874 class from being copied or assigned, making it a non-copyable class.
     *  This is a common design pattern used for classes that manage unique resources, such as hardware interfaces,
     *  to ensure that only one object controls a specific DC motor at a time.
     */
    DRV8874(const DRV8874 &) = delete;
    DRV8874 &operator=(const DRV8874 &) = delete;

    ~DRV8874()
    {
        if (pinIDs)
        {
            delete[] pinIDs;
            pinIDs = nullptr;
        }
    }

    /** 
     *  @brief method for convenience: converts provided dir and pwm value to pwm control of the motor
     *  @param dir Direction of output, 1 is forward, -1 is reverse, 0 is coast
     *  @param pwmVal Magnitude of output - Pulse Width Modulation - 0 min, 255 max
     */
    void runPWM(int dir, int pwmVal)
    {
        if (pinIDs == nullptr || pin_count_ < 2)
            return; // Not in pwm mode or pins not configured

        // clamp pwm between 0-255 for Arduino analogWrite
        int pwm = pwmVal;
        if (pwm < 0)
            pwm = 0;
        if (pwm > 255)
            pwm = 255;

        if (invert) {
            dir = -dir;
        }

        // TODO: Test actual control of motor, fix if needed.
        if (pwm == 0) 
        {
            // Coast
            analogWrite(pinIDs[0], 0);
            analogWrite(pinIDs[1], 0);
        }
        else if (dir == 1) // TODO: finish
        {
            // Forward direction
            outputDebugLine("--------------------FORWARD");
            analogWrite(pinIDs[0], pwm);
            analogWrite(pinIDs[1], 0);
        }
        else if (dir == -1)
        {
            // Reverse direction
            outputDebugLine("--------------------REVERSE");
            analogWrite(pinIDs[0], 0);
            analogWrite(pinIDs[1], pwm);
        }
        else
        {
            // Brake motor - pwm probably shouldn't be max as that wastes power to heat
            analogWrite(pinIDs[0], pwm);
            analogWrite(pinIDs[1], pwm);
        }
    }

    /** 
     *  @brief Runs motor using PWM control; uses stored `output` (as pwm)
     *  @param direction The direction, either 1, -1, or 0 to run the motor at.
     */
    void runPWM(int direction)
    {
        runPWM(direction, output);
    }

    /** 
     *  @brief method for convenience: converts provided dir and output value to PH/EN control of the motor
     *  @param dir Direction of output, 1 is forward, -1 is reverse, 0 is coast
     *  @param out Magnitude of output - Pulse Width Modulation - 0 min, 255 max
     */
    void runPHEN(int dir, int out)
    {
        if (pinIDs == nullptr || pin_count_ < 2)
            return; // Not in pwm mode or pins not configured

        // clamp output between 0-255 for Arduino analogWrite
        int output = out;
        if (output < 0)
            output = 0;
        if (output > 255)
            output = 255;

        if (invert) {
            dir = -dir;
        }

        // TODO: Test actual control of motor, fix if needed.
        if (dir == 1)
        {
            // Forward direction
            outputDebugLine("--------------------FORWARD");
            digitalWrite(pinIDs[0], HIGH);
            analogWrite(pinIDs[1], output);
        }
        else if (dir == -1)
        {
            // Reverse direction
            outputDebugLine("--------------------REVERSE");
            analogWrite(pinIDs[0], LOW);
            analogWrite(pinIDs[1], output);
        }
        else
        {
            // Stop motor if dir == 0 or a value it shouldn't be - output probably shouldn't be max as that wastes power to heat
            analogWrite(pinIDs[1], 0);
        }
    }

    /** 
     *  @brief Runs motor using PH/EN control; uses stored `output`
     *  @param direction The direction, either 1, -1, or 0 to run the motor at.
     */
    void runPHEN(int direction)
    {
        runPHEN(direction, output);
    }

    /** 
     *  @brief Simple function that checks mode and passes provided dir and output value to control via PWM or PH/EN.
     *  @param direction Direction of output, 1 is forward, -1 is reverse, 0 is coast
     *  @param output Magnitude of analog output - 0 min, 255 max
     */
    void runMotor(int direction, int output) {
        if (mode) {
            runPWM(direction, output);
        } else {
            runPHEN(direction, output);
        }
    }

    /** 
     *  @brief Simple function that checks mode and passes provided dir value and pre-set output magnitude to control via PWM or PH/EN.
     *  @param dir Direction of output, 1 is forward, -1 is reverse, 0 is coast
     */
    void runMotor(int direction) {
        if (mode) {
            runPWM(direction, output);
        } else {
            runPHEN(direction, output);
        }
    }

    /**
     *  @brief Sets a stored default output, used in run_PWM(int direction);
     *  @param output The integer output to store, either for PWM or PHEN. Clamped: [0, 255].
     */
    void setOutput(int output) {
        if (output < 0) {
            this->output = 0;
        } else if (output > 255) {
            this->output = 255;
        } else {
            this->output = output;
        }
    }

    /**
     *  @return the current draw of the motor, in Amps. Calls analogRead() directly.
     *  TODO: Test getCurrent()
     */
    double getCurrent() {
        int counts = analogRead(pinIDs[2]);
        // Convert raw ADC counts to the IPROPI pin voltage
        double vipropi = ((double) counts) / DacMax * AdcRefVoltage;
        // VIPROPI = Imotor * AIPROPI * RIPROPI
        // Rearrange: Imotor = VIPROPI / (RIPROPI * AIPROPI)
        return vipropi / (Ripropi * Aipropi);
    }

private:
    int *pinIDs;
    int pin_count_;
    int output; // Preset output, from 0 to 255
    bool invert;
    bool mode;
    
    // Vref sets the current threshold at which the DRV8874 begins regulating the current
    double Vref = 3.3;
    // Reference voltage of the MCU's ADC (Uno Q logic level), used to convert analogRead() counts to volts.
    double AdcRefVoltage = 3.3;
    // Current-sense scaling factor inside the DRV8874.
    // If the motor is drawing 1A, the DRV8874 produces 0.455 mA at its IPROPI output.
    double Aipropi = 0.455;
    double Ripropi = 2.49; // On the Pololu DRV8874 carrier, Pololu already installs a 2.49 kΩ resistor.
    int DacMax = 1024; // ADC resolution (counts) of analogRead()
};
#endif // DRV8874_HPP