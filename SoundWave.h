/*
 * SoundWave.h
 *
 *  Created on: Apr 12, 2020
 *      Author: ans
 */

#ifndef SOUNDWAVE_H_
#define SOUNDWAVE_H_

#include <cmath>	// M_2_PI, M_PI, M_PI_2, std::asin, std::atan, std::sin, std::tan

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

private:
	Type type;

	double frequency;
	double period;
	double length;
	double startTime;
	double angularVelocity;
	double halfAngularVelocity;
};

#endif /* SOUNDWAVE_H_ */
