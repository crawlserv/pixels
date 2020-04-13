/*
 * SoundWave.cpp
 *
 *  Created on: Apr 12, 2020
 *      Author: ans
 */

#include "SoundWave.h"

#include <iostream>

// constructor: set the properties of the sound wave
SoundWave::SoundWave(Type type, double frequency, double length, double startTime)
		: type(type), frequency(frequency), length(length), startTime(startTime) {}

// destructor stub
SoundWave::~SoundWave() {}

// get the sound wave value at the specified time
double SoundWave::get(double time) const {
	constexpr double squareMax = 0.7;
	constexpr double sawtoothMax = 0.7;

	if(time > this->startTime + this->length)
		return 0.;

	const double volume = 1. - (time - this->startTime) / this->length;

	switch(this->type) {
	case SOUNDWAVE_SINE:
		// generate sine wave
		return volume * std::sin(this->frequency * 2. * M_PI * time);

	case SOUNDWAVE_SQUARE:
		// generate square wave
		if(std::sin(this->frequency * 2. * M_PI * time) > 0.)
			return volume * squareMax;
		else
			return -volume * squareMax;

	case SOUNDWAVE_TRIANGLE:
		// generate triangle wave
		return volume * std::asin(std::cos(this->frequency * 2. * M_PI * time)) / M_PI_2;

	case SOUNDWAVE_SAWTOOTH:
		// generate sawtooth wave
		return volume * sawtoothMax * - M_2_PI * std::atan(1. / std::tan((time * M_PI) * this->frequency));
	}

	return 0.;
}

// get whether the sound wave has ended at the specified time
bool SoundWave::done(double time) const {
	return time > this->startTime + this->length;
}
