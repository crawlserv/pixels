/*
 * SoundWave.h
 *
 *  Created on: Apr 12, 2020
 *      Author: ans
 */

#ifndef SOUNDWAVE_H_
#define SOUNDWAVE_H_

#pragma once

#include "Math.h"
#include "Rand.h"
#include "SoundEnvelope.h"

#include <cmath>		// M_2_PI, M_PI, M_PI_2, std::asin, std::fmod
#include <cstddef>		// std::size_t
#include <string>		// std::string
#include <vector>		// std::vector

// class representing an abstract sound wave that diminishes over time
class SoundWave {
public:
	enum Type {
		SOUNDWAVE_NONE,
		SOUNDWAVE_SINE,
		SOUNDWAVE_SQUARE,
		SOUNDWAVE_TRIANGLE,
		SOUNDWAVE_SAWTOOTH,
		SOUNDWAVE_SAWTOOTH_OPTIMIZED,
		SOUNDWAVE_NOISE,
		SOUNDWAVE_NOISE_PRECALCULATED
	};

	struct Properties {
		Type type;
		double frequency;
		double length;
		double startTime;

		Properties(Type type, double frequency, double length, double startTime)
				: type(type), frequency(frequency), length(length), startTime(startTime) {}
		Properties() : Properties(SOUNDWAVE_NONE, 0., 0., 0.) {}
	};

	SoundWave();
	SoundWave(const Properties& properties, Rand * noiseGeneratorPointer = nullptr);
	SoundWave(
			const Properties& properties,
			const SoundEnvelope& envelope,
			Rand * noiseGeneratorPointer = nullptr,
			const std::vector<double> * noiseValues = nullptr,
			double samplesPerSecond = 0.
	);
	virtual ~SoundWave();

	void start(double time);
	void stop(double time);

	double get(double time);
	bool done(double time) const;
	bool exists() const;

	void setEnvelope(const SoundEnvelope& envelope);
	void setWaveVolume(double volume);
	void setAnalogSawToothN(unsigned int n);

	void clear();

	std::string getTypeString() const;

	SoundWave(const SoundWave& other);
	SoundWave& operator=(const SoundWave& other);
	SoundWave(SoundWave&& other);
	SoundWave& operator=(SoundWave&& other);

private:
	// properties and envelope
	Properties properties;
	SoundEnvelope soundEnvelope;

	// pre-calculated values for wave generation
	double period;
	double angularVelocity;

	// pointer to pseudo-random number generator
	Rand * noiseGeneratorPointer;

	// pre-calculated noise
	std::vector<double> noise;
	double samplesPerSecond;

	// additional properties (setting them is optional)
	double waveVolume;
	double analogSawToothN;
};

#endif /* SOUNDWAVE_H_ */
