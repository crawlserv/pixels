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
		  angularVelocity(0.),
		  waveVolume(1.),
		  analogSawToothN(10) {
	// set type-specific default volumes and precalculate angularVelocity if necessary
	if(this->type == SOUNDWAVE_SAWTOOTH_OPTIMIZED)
		this->waveVolume = 0.7;
	else
		this->angularVelocity = frequency * 2. * M_PI;

	if(this->type == SOUNDWAVE_SQUARE)
		this->waveVolume = 0.6;
}

// destructor stub
SoundWave::~SoundWave() {}

// get the sound wave value at the specified time, in diminishing volume
double SoundWave::get(double time) const {
	if(time > this->startTime + this->length)
		return 0.;

	const double volume = 1. - (time - this->startTime) / this->length;

	switch(this->type) {
	case SOUNDWAVE_SINE:
		// generate sine wave
		return volume
				* this->waveVolume
				* std::sin(this->angularVelocity * time);

	case SOUNDWAVE_SQUARE:
		// generate square wave
		if(std::sin(this->angularVelocity * time) > 0.)
			return volume * this->waveVolume;
		else
			return - volume * this->waveVolume;

	case SOUNDWAVE_TRIANGLE:
		// generate triangle wave
		return volume
				* this->waveVolume
				* M_2_PI
				* std::asin(
						std::sin(
								this->angularVelocity * time
						)
				);

	case SOUNDWAVE_SAWTOOTH:
	{
		// generate "smooth" sawtooth wave
		double result = 0.0;

		for(double n = 1.0; n < analogSawToothN; n++)
			result -= (std::sin(n * this->angularVelocity * time)) / n;

		return volume * this->waveVolume * M_2_PI * result;
	}

	case SOUNDWAVE_SAWTOOTH_OPTIMIZED:
		// generate sawtooth wave in an optimized way
		return volume
				* this->waveVolume
				* M_2_PI
				* (
						this->frequency
						* M_PI
						* std::fmod(time, this->period) - M_PI_2
				);
	}

	return 0.;
}

// get whether the sound wave has ended at the specified time
bool SoundWave::done(double time) const {
	return time > this->startTime + this->length;
}
