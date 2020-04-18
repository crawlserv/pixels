/*
 * ExampleSound.h
 *
 *  Created on: Apr 12, 2020
 *      Author: ans
 */

#ifndef EXAMPLESOUND_H_
#define EXAMPLESOUND_H_

#pragma once

#include "ConcurrentCircular.h"
#include "Engine.h"
#include "Rand.h"
#include "Sound.h"
#include "SoundEnvelope.h"
#include "SoundWave.h"

#include <algorithm>	// std::remove_if, std::swap
#include <atomic>		// std::atomic
#include <chrono>		// std::chrono
#include <cmath>		// std::fmod, std::lround, std::pow
#include <cstddef>		// std::size_t
#include <cstdlib>		// EXIT_SUCCESS
#include <functional>	// std::bind, std::placeholders
#include <iostream>		// std::cout, std::endl
#include <mutex>		// std::lock_guard, std::mutex
#include <string>		// std::string, std::to_string
#include <thread>		// std::this_thread
#include <vector>		// std::vector

#define UNUSED(x) (void)(x)

class ExampleSound : Engine {
	enum Action {
		ACTION_NONE,
		ACTION_ADD_SINE,
		ACTION_ADD_SQUARE,
		ACTION_ADD_TRIANGLE,
		ACTION_ADD_SAWTOOTH,
		ACTION_ADD_NOISE,
		ACTION_CLEAR,
		ACTION_UPDATE,
		ACTION_QUIT
	};

	struct Command {
		double time;
		Action action;

		Command(double time, Action action) : time(time), action(action) {}
		Command(Action action) : Command(0., action) {}
		Command() : Command(ACTION_NONE) {}
	};

public:
	ExampleSound();
	virtual ~ExampleSound();

	int run(int argc, char * argv[]);

	void setMasterVolume(double master);
	void setMaxVolume(double max);

private:
	void onCreate() override;
	void onUpdate(double elapsedTime) override;
	void onDestroy() override;

	void threadIntermediary();

	void updateSoundWaves(double time);
	void addSoundWave(SoundWave::Type type);
	void clearSoundWaves();

	double generateSound(unsigned int channel, double time, bool forThread = false);
	double generateSoundFrom(double time, std::vector<SoundWave>& from);

	unsigned short pixelSize;
	unsigned short waveResolution;
	double masterVolume;
	double maxVolume;

	Rand randomGenerator;
	Rand noiseGenerator;
	Sound soundSystem;

	/*
	 * NOTE:	The main thread, the intermediary thread and the sound thread use parallel data structures.
	 *
	 * 			New commands will be sent by the main thread to the intermediary thread
	 * 			and new sound waves by the intermediary thread to the sound thread via two
	 * 			thread-safe, yet non-blocking, circular (FIFO) buffers to avoid locking,
	 * 			which is discouraged in real time sound programming.
	 *
	 */

	std::thread intermediary;
	std::mutex lock;

	std::atomic<std::size_t> numberOfSoundWaves;

	std::vector<SoundWave> soundWavesForIntermediary;
	std::vector<SoundWave> soundWavesForAudioThread;

	ConcurrentCircular<Command> commandsToIntermediary;
	ConcurrentCircular<SoundWave> soundWavesToAudioThread;

	std::atomic<bool> isUpdateSoundWaves;
	std::atomic<bool> isClearSoundWaves;

	double lastClearTime;
	double lastRenderValue;
};

#endif /* EXAMPLERECTS_H_ */
