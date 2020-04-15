/*
 * SoundEnvelope.cpp
 *
 *  Created on: Apr 13, 2020
 *      Author: ans
 */

#include "SoundEnvelope.h"

// constructor for setting values
SoundEnvelope::SoundEnvelope(const ADRTimes& adsr, double startAmplitude, double sustainAmplitude)
		: adrTimes(adsr),
		  amplitudeStart(startAmplitude),
		  amplitudeSustain(sustainAmplitude),
		  timeStarted(0.),
		  timeReleased(0.),
		  isOn(false) {}

// constructor for a simple envelope of the specified length (decay only; amplitude from 1. to 0.)
SoundEnvelope::SoundEnvelope(double length)
		: SoundEnvelope(ADRTimes(0., length, 0.), 1., 0.) {}

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

		if(relTime < this->adrTimes.attackTime)
			// attack (A) phase
			result = (relTime / this->adrTimes.attackTime) * this->amplitudeStart;
		else if(relTime < this->adrTimes.attackTime + this->adrTimes.decayTime)
			// decay (D) phase
			result = (relTime - this->adrTimes.attackTime) / this->adrTimes.decayTime
						* (this->amplitudeSustain - this->amplitudeStart)
						+ this->amplitudeStart;
		else
			// sustain (S) phase
			result = this->amplitudeSustain;
	}
	else if(this->adrTimes.releaseTime && time < this->timeReleased + this->adrTimes.releaseTime)
		// release (R) phase (or finished)
		result = ((time - this->timeReleased) / this->adrTimes.releaseTime)
				  * (0. - this->amplitudeSustain) + this->amplitudeSustain;

	if(result < epsilon)
		return 0.;

	return result;
}

// get whether the envelope has been completely processed at the specified time
bool SoundEnvelope::done(double time) const {
	return !(this->isOn) && time > this->timeReleased + this->adrTimes.releaseTime;
}

// get the attack, decay and release time of the envelope
const SoundEnvelope::ADRTimes& SoundEnvelope::getADRTimes() const {
	return this->adrTimes;
}
