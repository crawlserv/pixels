/*
 * ExampleSound.h
 *
 *  Created on: Apr 12, 2020
 *      Author: ans
 */

#ifndef EXAMPLESOUND_H_
#define EXAMPLESOUND_H_

#include "Engine.h"
#include "Rand.h"
#include "Sound.h"
#include "SoundWave.h"

#include <algorithm>	// std::remove_if, std::swap
#include <chrono>		// std::chrono
#include <cmath>		// std::fmod, std::lround, std::pow
#include <cstdlib>		// EXIT_SUCCESS
#include <functional>	// std::bind, std::placeholders
#include <iostream>		// std::cout, std::endl
#include <mutex>		// std::lock_guard, std::mutex
#include <string>		// std::string, std::to_string
#include <thread>		// std::this_thread
#include <vector>		// std::vector

class ExampleSound : Engine {
public:
	ExampleSound();
	virtual ~ExampleSound();

	int run(int argc, char * argv[]);

private:
	void onCreate() override;
	void onUpdate(double elapsedTime) override;
	void onDestroy() override;

	void addSoundWave(SoundWave::Type type);
	void clearSoundWaves();

	double generateSound(unsigned int channel, double time, bool forThread = false);
	double generateSoundFrom(double time, const std::vector<SoundWave>& from);

	unsigned short pixelSize;
	unsigned short waveResolution;
	Rand randomGenerator;
	Rand noiseGenerator;
	Sound soundSystem;

	std::vector<SoundWave> soundWavesForThread;
	std::vector<SoundWave> soundWavesForMain;

	std::mutex lockSoundWavesForThread;

	double lastClearTime;
};

#endif /* EXAMPLERECTS_H_ */
