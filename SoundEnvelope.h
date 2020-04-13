/*
 * SoundEnvelope.h
 *
 *  Created on: Apr 13, 2020
 *      Author: ans
 */

#ifndef SOUNDENVELOPE_H_
#define SOUNDENVELOPE_H_

class SoundEnvelope {
public:
	struct ADSRTimes {
		double attackTime;
		double decayTime;
		double sustainTime;
		double releaseTime;

		ADSRTimes(double a, double d, double s, double r) : attackTime(a), decayTime(d), sustainTime(s), releaseTime(r) {}
	};

	SoundEnvelope(const ADSRTimes& adsr, double startAmplitude, double sustainAmplitude);
	SoundEnvelope(double length);
	virtual ~SoundEnvelope();

	void on(double time);
	void off(double time);

	double get(double time) const;
	bool done(double time) const;

private:
	ADSRTimes adsrTimes;

	double amplitudeStart;
	double amplitudeSustain;
	double timeStarted;
	double timeReleased;

	bool isOn;
};

#endif /* SOUNDENVELOPE_H_ */
