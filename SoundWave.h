/*
 * SoundWave.h
 *
 *  Created on: Apr 12, 2020
 *      Author: ans
 */

#ifndef SOUNDWAVE_H_
#define SOUNDWAVE_H_

#include <cmath>	// M_PI, std::asin, std::cos, std::sin

// class representing an abstract sound wave that diminishes over time
class SoundWave {
public:
	enum Type {
		SOUNDWAVE_SINE = 0,
		SOUNDWAVE_SQUARE = 1,
		SOUNDWAVE_TRIANGLE = 2
	};

	SoundWave(Type type, double frequency, double length, double startTime);
	virtual ~SoundWave();

	double get(double time) const;
	bool done(double time) const;

private:
	Type type;

	double frequency;
	double length;
	double startTime;
};

#endif /* SOUNDWAVE_H_ */
