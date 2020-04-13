/*
 * SoundWave.h
 *
 *  Created on: Apr 12, 2020
 *      Author: ans
 */

#ifndef SOUNDWAVE_H_
#define SOUNDWAVE_H_

#include <cmath>		// M_2_PI, M_PI, M_PI_2, std::asin, std::fmod, std::sin

// class representing an abstract sound wave that diminishes over time
class SoundWave {
public:
	enum Type {
		SOUNDWAVE_SINE,
		SOUNDWAVE_SQUARE,
		SOUNDWAVE_TRIANGLE,
		SOUNDWAVE_SAWTOOTH,
		SOUNDWAVE_SAWTOOTH_OPTIMIZED
	};

	SoundWave(Type type, double frequency, double length, double startTime);
	virtual ~SoundWave();

	double get(double time) const;
	bool done(double time) const;

	void setWaveVolume(double volume) const;
	void setAnalogSawToothN(double n) const;

private:
	Type type;

	// initial values
	double frequency;
	double period;
	double length;
	double startTime;

	// pre-calculated value for wave generation
	double angularVelocity;

	// additional properties (setting them is optional)
	double waveVolume;
	double analogSawToothN;
};

#endif /* SOUNDWAVE_H_ */
