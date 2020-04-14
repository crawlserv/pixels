/*
 * SoundEnvelope.cpp
 *
 *  Created on: Apr 13, 2020
 *      Author: ans
 */

#include "SoundEnvelope.h"

// constructor for setting values
SoundEnvelope::SoundEnvelope(const ADSRTimes& adsr, double startAmplitude, double sustainAmplitude)
		: adsrTimes(adsr),
		  amplitudeStart(startAmplitude),
		  amplitudeSustain(sustainAmplitude),
		  timeStarted(0.),
		  timeReleased(0.),
		  isOn(false) {}

// constructor for a simple envelope of the specified length (decay only; amplitude from 1. to 0.)
SoundEnvelope::SoundEnvelope(double length)
		: SoundEnvelope(ADSRTimes(0., length, 0.), 1., 0.) {}

// destructor stub
SoundEnvelope::~SoundEnvelope() {}

// trigger the sound at the specified time
void SoundEnvelope::on(double time) {
	if(!(this->isOn)) {
		this->timeStarted = time;
		this->isOn = true;
	}
}

// release the sound at the specified time
void SoundEnvelope::off(double time) {
	if(this->isOn) {
		this->timeReleased = time;
		this->isOn = false;
	}
}

// get the amplitude of the envelope at the specified time
double SoundEnvelope::get(double time) const {
	constexpr double epsilon = 0.0001;

	double result = 0.;

	if(this->isOn) {
		const double relTime = time - this->timeStarted;

		if(relTime < epsilon)
			return 0.;

		if(relTime < this->adsrTimes.attackTime)
			// attack (A) phase
			result = (relTime / this->adsrTimes.attackTime) * this->amplitudeStart;
		else if(relTime < this->adsrTimes.attackTime + this->adsrTimes.decayTime)
			// decay (D) phase
			result = (relTime - this->adsrTimes.attackTime) / this->adsrTimes.decayTime
						* (this->amplitudeSustain - this->amplitudeStart)
						+ this->amplitudeStart;
		else
			// sustain (S) phase
			result = this->amplitudeSustain;
	}
	else if(this->adsrTimes.releaseTime && time < this->timeReleased + this->adsrTimes.releaseTime)
		// release (R) phase (or finished)
		result = ((time - this->timeReleased) / this->adsrTimes.releaseTime)
				  * (0. - this->amplitudeSustain) + this->amplitudeSustain;

	if(result < epsilon)
		return 0.;

	return result;
}

// get whether the envelope has been completely processed at the specified time
bool SoundEnvelope::done(double time) const {
	return !(this->isOn) && time > this->timeReleased + this->adsrTimes.releaseTime;
}
