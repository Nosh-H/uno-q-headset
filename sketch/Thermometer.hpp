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
 *  File: Thermometer.hpp
 *  Author: Noah Haskell
 *  Date: 18 June 2026
 *  Description: Class to manage the reading of input data from the Thermometer
 */

#ifndef THERMOMETER_HPP
#define THERMOMETER_HPP
#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

class Thermometer {
public:

    /**
     * @brief Construct a Thermometer, to manage a thermometer using the DallasTemperature library.
     * @param pin The pin number on the Arduino to read the input
     * @param period How often to update the thermometer, in milliseconds (min: 0.75 seconds)
     */
    Thermometer(uint8_t, double);

    /**
     * @brief Call in the main file's setup() to begin the temperature sensor.
     */
    void setup();

    /**
     * @brief Gets the latest temperature, in degrees Fahrenheit
     */
    float getDegreesFahrenheit();

    /**
     *  @brief Update the DigitalInput - get the state of the boolean. Should be called regularly.
     */
    void periodic();

private:
    double period;
    float latestTemperatureF;
    // The millis() time of the start of the period
    unsigned long cycleStartTime;
    // Stores whether periodic() already called the thermometer's temperature during the current period.
    bool gotTempThisCycle;

    OneWire oneWire;
    DallasTemperature thermometer;
};

Thermometer::Thermometer(uint8_t pin, double period)
    : period(period), oneWire(pin), thermometer(&oneWire) {
}

void Thermometer::setup() {
    thermometer.begin();
    cycleStartTime = millis();
}

void Thermometer::periodic() {
    // Restarts the cycle at the end of the period, and begin the request to see a new temperature
    if (millis() > period && gotTempThisCycle) {
        thermometer.requestTemperatures();
        cycleStartTime = millis();
        gotTempThisCycle = false;
        return;
    }
    // Don't poll the thermometer if we already did in this cycle
    if (gotTempThisCycle) {
        return;
    }
    // 750 ms into the cycle, we can acces the updated temperature.
    if (millis() - cycleStartTime >= 750) {
        latestTemperatureF = thermometer.getTempFByIndex(0);
        gotTempThisCycle = true;
    }
}

float Thermometer::getDegreesFahrenheit() {
    return latestTemperatureF;
}
#endif