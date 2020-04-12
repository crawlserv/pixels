/*
 * ExampleSound.cpp
 *
 *  Created on: Apr 12, 2020
 *      Author: ans
 */

#include "ExampleSound.h"

// constructor
ExampleSound::ExampleSound()
		: pixelSize(2),
		  waveResolution(10),
		  randomGenerator(Rand::RAND_ALGO_LEHMER32),
		  lastClearTime(0.),
		  test(false) {
	// setup random generator
	this->randomGenerator.setRealLimits(0.1, 1.5);		// wave lengths between 0.1 and 1.5 seconds
	this->randomGenerator.setByteLimits(0, 47);			// 48 tones over three octaves

	// reserve memory for a thousand (simultaneous) sound waves
	this->soundWavesForMain.reserve(1000);
	this->soundWavesForThread.reserve(1000);
}

// destructor
ExampleSound::~ExampleSound() {}

// run the application
int ExampleSound::run(int argc, char * argv[]) {
	// setup the engine
	const std::string name("sound");
	const int width = 800;
	const int height = 600;

	this->setClearBuffer(true);
	this->setPixelSize(this->pixelSize);
	this->createMainWindow(width, height, name);

	// setup the sound system
	this->soundSystem.setOutputFunction(
			std::bind(
					&ExampleSound::generateSound,
					this,
					std::placeholders::_1,
					true
			)
	);
	this->soundSystem.setOutputStreamName(name);

	// run the engine
	this->Engine::run();

	return EXIT_SUCCESS;
}

// create resources
void ExampleSound::onCreate() {
	// start the sound system
	this->soundSystem.start(this->getTime());
}

// update frame
void ExampleSound::onUpdate(double elapsedTime) {
	const double currentTime = this->getTime();

	// clear sound waves that have ended every second
	if(currentTime - this->lastClearTime > 1.) {
		// clear data for the sound thread
		{
			std::lock_guard<std::mutex> threadDataLock(this->lockSoundWavesForThread);

			this->soundWavesForThread.erase(
					std::remove_if(
							this->soundWavesForThread.begin(),
							this->soundWavesForThread.end(),
							[&currentTime](const auto& wave) -> bool {
								return wave.done(currentTime);
							}
					),
					this->soundWavesForThread.end()
			);
		}

		// clear data for the main thread
		this->soundWavesForMain.erase(
				std::remove_if(
						this->soundWavesForMain.begin(),
						this->soundWavesForMain.end(),
						[&currentTime](const auto& wave) -> bool {
							return wave.done(currentTime);
						}
				),
				this->soundWavesForMain.end()
		);

		this->lastClearTime = currentTime;
	}

	// render sound wave
	const auto w = this->getWindowWidth();
	const auto h = this->getWindowHeight();
	const double res = static_cast<double>(this->waveResolution) / 1000.;

	const double halfHeight = static_cast<double>(h) / 2;

	for(int x = 0; x < w; ++x) {
		const double sound = this->generateSound(currentTime + static_cast<double>(x) / w * res);

		int yFrom = halfHeight;
		int yTo = std::lround(halfHeight + sound * halfHeight);
		bool swapped = false;

		if(yFrom > yTo) {
			std::swap(yFrom, yTo);
			swapped = true;
		}

		for(int y = yFrom; y <= yTo; ++y) {
			double red = 0.;

			if(swapped)
				red = static_cast<double>(yTo - y) / halfHeight;
			else
				red = static_cast<double>(y - yFrom) / halfHeight;

			this->draw(
					x,
					y,
					std::lround(255 * red),
					std::lround(255 - 255 * red),
					0
			);
		}
	}

	// show number of sound waves and current resolution
	this->setDebugText(
			"n="
			+ std::to_string(this->soundWavesForMain.size())
			+ ", res="
			+ std::to_string(this->waveResolution)
			+ "ms"
	);

	// handle keys
	const unsigned short oldPixelSize = this->pixelSize;

	if(this->isKeyPressed(GLFW_KEY_UP) && this->pixelSize < 100)
		++(this->pixelSize);

	if(this->isKeyRepeated(GLFW_KEY_UP) && this->pixelSize < 100)
		++(this->pixelSize);

	if(this->isKeyPressed(GLFW_KEY_DOWN) && this->pixelSize > 1)
		--(this->pixelSize);

	if(this->isKeyRepeated(GLFW_KEY_DOWN) && this->pixelSize > 1)
		--(this->pixelSize);

	if(this->pixelSize != oldPixelSize)
		this->setPixelSize(this->pixelSize);

	if(this->isKeyPressed(GLFW_KEY_RIGHT) && this->waveResolution < 100)
		++(this->waveResolution);

	if(this->isKeyRepeated(GLFW_KEY_RIGHT) && this->waveResolution < 100)
		++(this->waveResolution);

	if(this->isKeyPressed(GLFW_KEY_LEFT) && this->waveResolution > 1)
		--(this->waveResolution);

	if(this->isKeyRepeated(GLFW_KEY_LEFT) && this->waveResolution > 1)
		--(this->waveResolution);

	if(this->isKeyPressed(GLFW_KEY_ENTER))
		this->addSoundWave(SoundWave::SOUNDWAVE_SINE);

	if(this->isKeyRepeated(GLFW_KEY_ENTER))
		this->addSoundWave(SoundWave::SOUNDWAVE_SINE);

	if(this->isKeyPressed(GLFW_KEY_SPACE))
		this->addSoundWave(SoundWave::SOUNDWAVE_SQUARE);

	if(this->isKeyRepeated(GLFW_KEY_SPACE))
		this->addSoundWave(SoundWave::SOUNDWAVE_SQUARE);

	if(this->isKeyPressed(GLFW_KEY_TAB))
		this->addSoundWave(SoundWave::SOUNDWAVE_TRIANGLE);

	if(this->isKeyRepeated(GLFW_KEY_TAB))
		this->addSoundWave(SoundWave::SOUNDWAVE_TRIANGLE);

	if(this->isKeyPressed(GLFW_KEY_ESCAPE))
		this->clearSoundWaves();

	if(this->isKeyRepeated(GLFW_KEY_ESCAPE))
		this->clearSoundWaves();

	this->test = this->isKeyHeld(GLFW_KEY_T);
}

// clear resources
void ExampleSound::onDestroy() {
	// stop the sound system
	this->soundSystem.stop();

	// clear the sound waves
	std::vector<SoundWave>().swap(this->soundWavesForMain);
	std::vector<SoundWave>().swap(this->soundWavesForThread);
}

// add a sound wave
void ExampleSound::addSoundWave(SoundWave::Type type) {
	// generate random pitch and length
	const double octaveBase = 110.;
	const double twelfthRootOf2 = std::pow(2., 1. / 12.);
	const double frequency = octaveBase * std::pow(twelfthRootOf2, this->randomGenerator.generateByte());
	const double length = this->randomGenerator.generateReal();
	const double start = this->getTime();

	// add sound wave to the main thread
	this->soundWavesForMain.emplace_back(type, frequency, length, start);

	// add sound wave to the sound thread
	{
		std::lock_guard<std::mutex> threadDataLock(this->lockSoundWavesForThread);

		this->soundWavesForThread.emplace_back(type, frequency, length, start);
	}
}

// clear all sound waves
void ExampleSound::clearSoundWaves() {
	// clear data for the main thread
	std::vector<SoundWave>().swap(this->soundWavesForMain);

	// clear data for the sound thread
	{
		std::lock_guard<std::mutex> threadDataLock(this->lockSoundWavesForThread);

		std::vector<SoundWave>().swap(this->soundWavesForThread);
	}
}

// generate sound at the specified time
double ExampleSound::generateSound(double time, bool forThread) {
	if(forThread) {
		std::lock_guard<std::mutex> threadDataLock(this->lockSoundWavesForThread);

		return this->generateSoundFrom(time, this->soundWavesForThread);
	}

	return this->generateSoundFrom(time, this->soundWavesForMain);
}

// generate sound at the specified time from the specified source
double ExampleSound::generateSoundFrom(double time, const std::vector<SoundWave>& from) {
	if(from.empty())
		return 0.;

	double result = 0.;

	for(const auto& wave : from)
		result += 0.75 * wave.get(time);

	if(result > 0.75)
		result = 0.75;

	if(result < -0.75)
		result = -0.75;

	return result;
}