/*
 * SoundWave.cpp
 *
 *  Created on: Apr 12, 2020
 *      Author: ans
 */

#include "SoundWave.h"

#include <iostream>

// constructor: set the properties of the sound wave and calculate some standard values
SoundWave::SoundWave(Type type, double frequency, double length, double startTime)
		: type(type),
		  frequency(frequency),
		  period(1. / frequency),
		  length(length),
		  startTime(startTime),
		  angularVelocity(frequency * 2. * M_PI),
		  halfAngularVelocity(frequency * M_PI) {}

// destructor stub
SoundWave::~SoundWave() {}

// get the sound wave value at the specified time, in diminishing volume
double SoundWave::get(double time) const {
	constexpr double squareMax = 0.6;
	constexpr double sawToothMax = 0.7;

	if(time > this->startTime + this->length)
		return 0.;

	const double volume = 1. - (time - this->startTime) / this->length;

	switch(this->type) {
	case SOUNDWAVE_SINE:
		// generate sine wave
		return volume * std::sin(this->angularVelocity * time);

	case SOUNDWAVE_SQUARE:
		// generate square wave
		if(std::sin(this->angularVelocity * time) > 0.)
			return volume * squareMax;
		else
			return -volume * squareMax;

	case SOUNDWAVE_TRIANGLE:
		// generate triangle wave
		return volume * std::asin(std::sin(this->angularVelocity * time)) * M_2_PI;

	case SOUNDWAVE_SAWTOOTH:
	{
		// generate "smooth" sawtooth wave
		double result = 0.0;

		for(double n = 1.0; n < 50.; n++)
			result -= (std::sin(n * this->angularVelocity * time)) / n;

		return volume * sawToothMax * result * M_2_PI;
	}

	case SOUNDWAVE_SAWTOOTH_OPTIMIZED:
		// generate sawtooth wave in an optimized way
		return volume * sawToothMax * M_2_PI * (this->frequency * M_PI * std::fmod(time, this->period) - M_PI_2);
	}

	return 0.;
}

// get whether the sound wave has ended at the specified time
bool SoundWave::done(double time) const {
	return time > this->startTime + this->length;
}
