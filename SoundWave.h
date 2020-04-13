/*
 * SoundWave.h
 *
 *  Created on: Apr 12, 2020
 *      Author: ans
 */

#ifndef SOUNDWAVE_H_
#define SOUNDWAVE_H_

#include "Rand.h"

#include <cmath>		// M_2_PI, M_PI, M_PI_2, std::asin, std::fmod, std::sin

// class representing an abstract sound wave that diminishes over time
class SoundWave {
public:
	enum Type {
		SOUNDWAVE_SINE,
		SOUNDWAVE_SQUARE,
		SOUNDWAVE_TRIANGLE,
		SOUNDWAVE_SAWTOOTH,
		SOUNDWAVE_SAWTOOTH_OPTIMIZED,
		SOUNDWAVE_NOISE
	};

	SoundWave(Type type, double frequency, double length, double startTime, Rand * noiseGeneratorPointer = nullptr);
	virtual ~SoundWave();

	double get(double time) const;
	bool done(double time) const;

	void setWaveVolume(double volume);
	void setAnalogSawToothN(unsigned int n);

private:
	Type type;

	// initial values
	double frequency;	// if generating noise, this value will be used as seed
	double period;
	double length;
	double startTime;

	// pre-calculated value for wave generation
	double angularVelocity;

	// pointer to pseudo-random number generator
	Rand * noiseGeneratorPointer;

	// additional properties (setting them is optional)
	double waveVolume;
	double analogSawToothN;
};

#endif /* SOUNDWAVE_H_ */
