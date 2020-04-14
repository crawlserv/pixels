/*
 * SoundWave.cpp
 *
 *  Created on: Apr 12, 2020
 *      Author: ans
 */

#include "SoundWave.h"

// constructor: set properties of the sound wave, the encompassing envelope and pre-calculate values
SoundWave::SoundWave(
		const Properties& properties,
		const SoundEnvelope& envelope,
		Rand * noiseGeneratorPointer
)	: properties(properties),
	  soundEnvelope(envelope),
	  period(1. / properties.frequency),
	  angularVelocity(0.),
	  noiseGeneratorPointer(noiseGeneratorPointer),
	  waveVolume(1.),
	  analogSawToothN(15) {
	// set type-specific default volumes and precalculate angularVelocity if necessary
	if(this->properties.type == SOUNDWAVE_SAWTOOTH_OPTIMIZED)
		this->waveVolume = 0.7;

	if(this->properties.type == SOUNDWAVE_SQUARE)
		this->waveVolume = 0.6;

	if(this->properties.type != SOUNDWAVE_SAWTOOTH_OPTIMIZED && this->properties.type != SOUNDWAVE_NOISE)
		this->angularVelocity = properties.frequency * 2. * M_PI;

	// change real distribution of noise generator if necessary
	if(this->properties.type == SOUNDWAVE_NOISE && this->noiseGeneratorPointer)
		this->noiseGeneratorPointer->setRealLimits(-1., 1.);
}

// constructor for using a default envelope (sustain only)
SoundWave::SoundWave(
		const Properties& properties,
		Rand * noiseGeneratorPointer
)	: SoundWave(properties, SoundEnvelope(properties.length), noiseGeneratorPointer) {}

// destructor stub
SoundWave::~SoundWave() {}

// start the sound wave at the specified time
void SoundWave::start(double time) {
	this->soundEnvelope.on(time);
}

// stop the sound wave at the specified time
void SoundWave::stop(double time) {
	this->soundEnvelope.off(time);
}

// get the sound wave value at the specified time
double SoundWave::get(double time) {
	constexpr double epsilon = 0.0001;

	if(time < this->properties.startTime)
		return 0.;

	if(time > this->properties.startTime + this->properties.length)
		this->soundEnvelope.off(time);

	double volume = this->soundEnvelope.get(time) * this->waveVolume;

	if(volume < epsilon)
		return 0.;

	switch(this->properties.type) {
	case SOUNDWAVE_SINE:
		// generate sine wave

		/*
		 * NOTE:	To approximate the sine wave as exact as possible (without using std::sin),
		 * 			we use a fairly accurate approximation (i.e. with a short Taylor series) here.
		 */

		return volume * Math::approxSinTaylor(this->angularVelocity * time);

	case SOUNDWAVE_SQUARE:
		// generate square wave
		if(Math::approxSinCubic(this->angularVelocity * time) > 0.)
			return volume;
		else
			return - volume;

	case SOUNDWAVE_TRIANGLE:
	{
		// generate triangle wave
		auto approxSin = Math::approxSinCubic(this->angularVelocity * time);

		/*
		 * NOTE:	For using std::asin without breaking the sound wave,
		 * 			the approximate sine value cannot not be outside [-1.,1].
		 */

		if(approxSin > 1.)
			approxSin = 1.;
		else if(approxSin < -1.)
			approxSin = -1.;

		return volume
				* M_2_PI
				* std::asin(approxSin);
	}

	case SOUNDWAVE_SAWTOOTH:
	{
		// generate "smooth" sawtooth wave
		double result = 0.;

		for(double n = 1.; n < analogSawToothN; n++)
			/*
			 * NOTE:	Because of the many sine calculations, we are using their
			 * 			(probably) fastest (i.e. quadratic) approximation here.
			 */
			result -= (Math::approxSinQuad(n * this->angularVelocity * time)) / n;

		return volume * M_2_PI * result;
	}

	case SOUNDWAVE_SAWTOOTH_OPTIMIZED:
		// generate sawtooth wave in an optimized way
		return volume
				* M_2_PI
				* (
						this->properties.frequency
						* M_PI
						* std::fmod(time, this->period) - M_PI_2
				);

	case SOUNDWAVE_NOISE:
		if(this->noiseGeneratorPointer)
			return volume * this->noiseGeneratorPointer->generateReal();
	}

	return 0.;
}

// get whether the sound wave has ended at the specified time
bool SoundWave::done(double time) const {
	return this->soundEnvelope.done(time);
}

// set the ADSR (attack, decay, sustain, release) envelope of the sound wave
void SoundWave::setEnvelope(const SoundEnvelope& envelope) {
	this->soundEnvelope = envelope;
}

// set the master volume of the wave
//	NOTE: values outside of [0;1] will be clamped to avoid illegal output
void SoundWave::setWaveVolume(double volume) {
	constexpr double epsilon = 0.0001;

	if(volume + epsilon > 1.)
		this->waveVolume = 1.;
	else if(volume < epsilon)
		this->waveVolume = 0.;
	else
		this->waveVolume = volume;
}

// set the N for analog sawtooth waves, i.e. how many sine waves will be added up for the wave
//	NOTE: high values can lead to 'underflow' and break the sound output
void SoundWave::setAnalogSawToothN(unsigned int n) {
	this->analogSawToothN = n;
}

// get the type of the wave as a string
std::string SoundWave::getTypeString() const {
	switch(this->properties.type) {
	case SOUNDWAVE_SINE:
		return "Sine";

	case SOUNDWAVE_SQUARE:
		return "Square";

	case SOUNDWAVE_TRIANGLE:
		return "Triangle";

	case SOUNDWAVE_SAWTOOTH:
		return "Sawtooth (analog)";

	case SOUNDWAVE_SAWTOOTH_OPTIMIZED:
		return "Sawtooth (optimized)";

	case SOUNDWAVE_NOISE:
		return "Pseudo-random noise";
	}

	return "<unknown>";
}
