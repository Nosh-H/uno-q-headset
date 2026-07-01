/*
 *  Copyright (C) 2026 Noah Haskell
 * 
 *  File: TimeDebounce.hpp
 *  Author: Noah Haskell
 *  Date: 9 March 2026
 *  Description: Timer-based Debounce Class Header File
 *
 *  Based on the TimeDebounce Class by chillibasket:
 *  https://github.com/chillibasket/arduino-classes/blob/master/button-debounce/TimeDebounce.hpp
 * 
 *  Which is in turn based on the Debounce Class by wkoch:
 *  https://github.com/wkoch/Debounce
 */

#ifndef TIME_DEBOUNCE_HPP
#define TIME_DEBOUNCE_HPP

// Required for millis()
#include <Arduino.h>


// Struct used to pack boolean bits
struct TimeDebounceFlags {
	unsigned currentState   :  1;
	unsigned changeDetected :  1;
	unsigned waiting        :  1;
	unsigned changeCounter  : 13;
};

/**
 * @class TimeDebounce
 */
class TimeDebounce {
public:
	// Constructor and destructor
	TimeDebounce(uint16_t risingEdgeDelay = 50, uint16_t fallingEdgeDelay = 50);
	~TimeDebounce();

    // Initialise the button
	void begin(bool enablePullup = true);

	// Update button reading
	bool update(bool readState = false);

	// Functions to check for current state
	bool read();
	bool onChange();
	bool onRisingEdge();
	bool onFallingEdge();

	// Button press counter functions
	uint16_t count();
	void reset();

private:
	unsigned long debounceTimer;
	const uint16_t risingEdgeDelay;
    const uint16_t fallingEdgeDelay;
	struct TimeDebounceFlags flags;
};

/**
 * Constructor
 *
 * @param  pin    The I/O pin used for the button
 * @param  risingEdgeDelay  Time in milliseconds for the rising edge debounce (default = 50)
 * @param  fallingEdgeDelay Time in milliseconds for the falling edge debounce (default = 50)
 */
TimeDebounce::TimeDebounce(uint16_t risingEdgeDelay, uint16_t fallingEdgeDelay)
	: risingEdgeDelay(risingEdgeDelay)
	, fallingEdgeDelay(fallingEdgeDelay)
{
	// Empty, as all initialization is done in the begin() function
}

/**
 * Default Destructor
 */
TimeDebounce::~TimeDebounce() {
	// Empty - no dynamic memory used
}

/**
 * @brief Initialize the button pins
 *
 * @param currentstate  The current state of the boolean that this debounce is applied to.
 */
void TimeDebounce::begin(bool currentState) {
	debounceTimer = 0;
	flags.currentState = currentState;
	flags.waiting = false;
	flags.changeCounter = 0;
	flags.changeDetected = false;
}

/**
 * @brief Update the debouncer with the newest value, and get the value with the debounce applied.
 *
 * @param readState The updated state of the boolean that this debounce is applied to
 * @return Current state after applying debounce
 */
bool TimeDebounce::update(bool readState) {

	// For each change of state
	if (readState != flags.currentState) {

		// Start the debounce timer
		if (!flags.waiting) {
			debounceTimer = millis();
			flags.waiting = true;

		// If reading stayed the same during debounce, update the state
        } else if (readState && (millis() - debounceTimer > risingEdgeDelay)) {
			flags.currentState = readState;
			flags.changeDetected = true;
			flags.changeCounter++;
			flags.waiting = false;
        } else if (!readState && (millis() - debounceTimer > fallingEdgeDelay)) {
            flags.currentState = readState;
			flags.changeDetected = true;
			flags.changeCounter++;
			flags.waiting = false;
        }

	// If debounce was active but state reverted, reset the timer
	} else {
		flags.waiting = false;
	}

	return flags.currentState;
}

/**
 * Read the current button state
 *
 * @return Debounced button State
 */
bool TimeDebounce::read() {
	return flags.currentState;
}

/**
 * Check if the button state has changed
 *
 * @return True on rising/falling edge, false otherwise
 */
bool TimeDebounce::onChange() {
	if (flags.changeDetected) {
		flags.changeDetected = false;
		return true;
	}
	return false;
}

/**
 * Check for a rising edge
 *
 * @return True if rising edge detected
 */
bool TimeDebounce::onRisingEdge() {
    if (flags.currentState && flags.changeDetected) {
		flags.changeDetected = false;
		return true;
	}
	return false;
}

/**
 * Check for a falling edge
 *
 * @return True if falling edge detected
 */
bool TimeDebounce::onFallingEdge() {
	if (!flags.currentState && flags.changeDetected) {
		flags.changeDetected = false;
		return true;
	}
	return false;
}

/**
 * Count how many button clicks have occurred
 *
 * @return The number of full button press & releases
 */
uint16_t TimeDebounce::count() {
	// Counter records presses and releases, so divide by 2
	return flags.changeCounter / 2;
}

/**
 * Reset the button click counter
 */
void TimeDebounce::reset() {
	// Set counter to 1 if button is currently pressed, 0 otherwise
	flags.changeCounter = !flags.currentState;
}

#endif