/*
 *  Copyright (C) 2026 Noah Haskell
 * 
 *  File: DigitalInput.hpp
 *  Author: Noah Haskell
 *  Date: 18 June 2026
 *  Description: Class to read a boolean from a Digital Input and apply debouncing.
 */

#ifndef DIGITALINPUT_HPP
#define DIGITALINPUT_HPP
#include <Arduino.h>
#include <TimeDebounce.hpp>

class DigitalInput {
public:

    /**
     * @brief Construct a DigitalInput and its tap (50ms) and hold debouncers.
     * @param receiverPin The pin number on the Arduino to read the input
     * @param pullup Whether to use the Arduino's 10K pullup resistor or not
     * @param debounceRising The debounce time for rising edge, in milliseconds.
     * @param debounceFalling The debounce time for falling edge, in milliseconds.
     */
    DigitalInput(uint8_t, bool, uint16_t, uint16_t);

    /**
     * @brief Call in the main file's setup() to call pinMode() and begin the debouncer.
     */
    void setup();

    /**
     * @brief Get the raw current state of the DigitalInput. If open, return true.
     */
    bool get();

    /**
     * @brief Get the state of the DigitalInput with the rising and falling debounces of the debouncer applied.
     */
    bool getDebounced();

    /**
     *  @brief Update the DigitalInput - get the state of the boolean. Should be called regularly.
     */
    void periodic();

    /**
     *  @brief Get whether the DigitalInput is on a rising edge: false --> true.
     */
    bool onRisingEdge();

    /**
     *  @brief Get whether the DigitalInput is on a falling edge: true --> false.
     */
    bool onFallingEdge();

private:
    uint8_t receiverPin;
    bool pullup;
    uint8_t state;

    TimeDebounce debounce;
};

// Construct debouncers with reasonable defaults: short tap and long hold
DigitalInput::DigitalInput(uint8_t receiverPin, bool pullup, uint16_t debounceRising, uint16_t debounceFalling)
    : receiverPin(receiverPin), pullup(pullup), state(0), debounce(debounceRising, debounceFalling) {
}

void DigitalInput::setup() {
    pinMode(receiverPin, (pullup ? INPUT_PULLUP : INPUT));

    // Initialise debouncer with the current physical state
    debounce.begin(get());
}

bool DigitalInput::get() {
    int raw = digitalRead(receiverPin);
    // If using INPUT_PULLUP, pressed == LOW (0), so invert to get "pressed == true"
    return pullup ? (raw == LOW) : (raw == HIGH);
}

bool DigitalInput::getDebounced() {
    // Return the short-tap debounced reading by default
    return debounce.read();
}

void DigitalInput::periodic() {
    debounce.update(get());
}

bool DigitalInput::onRisingEdge() {
    return debounce.onRisingEdge();
}

bool DigitalInput::onFallingEdge() {
    return debounce.onFallingEdge();
}

#endif