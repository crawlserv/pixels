/*
 * SoundEnvelope.h
 *
 *  Created on: Apr 13, 2020
 *      Author: ans
 */

#ifndef SOUNDENVELOPE_H_
#define SOUNDENVELOPE_H_

#pragma once

class SoundEnvelope {
public:
	struct ADRTimes {
		double attackTime;
		double decayTime;
		double releaseTime;

		ADRTimes(double a, double d, double r) : attackTime(a), decayTime(d), releaseTime(r) {}
		ADRTimes() : ADRTimes(0., 0., 0.) {}
	};

	SoundEnvelope(const ADRTimes& adr, double startAmplitude, double sustainAmplitude);
	SoundEnvelope();
	SoundEnvelope(double length);
	virtual ~SoundEnvelope();

	void on(double time);
	void off(double time);

	double get(double time) const;
	bool done(double time) const;

	const ADRTimes& getADRTimes() const;

private:
	ADRTimes adrTimes;

	double amplitudeStart;
	double amplitudeSustain;
	double timeStarted;
	double timeReleased;

	bool isOn;
};

#endif /* SOUNDENVELOPE_H_ */
