/*
 * SoundWave.h
 *
 *  Created on: Apr 12, 2020
 *      Author: ans
 */

#ifndef SOUNDWAVE_H_
#define SOUNDWAVE_H_

#include "Math.h"
#include "Rand.h"
#include "SoundEnvelope.h"

#include <cmath>		// M_2_PI, M_PI, M_PI_2, std::asin, std::fmod

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

	struct Properties {
		Type type;
		double frequency;
		double length;
		double startTime;

		Properties(Type type, double frequency, double length, double startTime)
				: type(type), frequency(frequency), length(length), startTime(startTime) {}
	};

	SoundWave(const Properties& properties, Rand * noiseGeneratorPointer = nullptr);
	SoundWave(const Properties& properties, const SoundEnvelope& envelope, Rand * noiseGeneratorPointer = nullptr);
	virtual ~SoundWave();

	void start(double time);
	void stop(double time);

	double get(double time);
	bool done(double time) const;

	void setEnvelope(const SoundEnvelope& envelope);
	void setWaveVolume(double volume);
	void setAnalogSawToothN(unsigned int n);

private:
	// properties and envelope
	Properties properties;
	SoundEnvelope soundEnvelope;

	// pre-calculated values for wave generation
	double period;
	double angularVelocity;

	// pointer to pseudo-random number generator
	Rand * noiseGeneratorPointer;

	// additional properties (setting them is optional)
	double waveVolume;
	double analogSawToothN;
};

#endif /* SOUNDWAVE_H_ */
