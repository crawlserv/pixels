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
		  masterVolume(0.7),
		  maxVolume(0.8),
		  randomGenerator(Rand::RAND_ALGO_LEHMER32),
		  noiseGeneratorMain(Rand::RAND_ALGO_LEHMER32),
		  noiseGeneratorThread(Rand::RAND_ALGO_LEHMER32),
		  lastClearTime(0.) {
	// setup random generator
	this->randomGenerator.setRealLimits(0.1, 1.5);		// wave lengths between 0.1 and 1.5 seconds
	this->randomGenerator.setByteLimits(0, 47);			// 48 tones over three octaves

	// reserve memory for up to a hundred (simultaneous) sound waves
	this->soundWavesForMain.reserve(100);
	this->soundWavesForThread.reserve(100);
}

// destructor
ExampleSound::~ExampleSound() {}

// run the application
int ExampleSound::run(int argc, char * argv[]) {
	UNUSED(argc);
	UNUSED(argv);

	// setup the engine
	const std::string name("sound");

	constexpr int width = 800;
	constexpr int height = 600;

	this->setClearBuffer(true);
	this->setPixelSize(this->pixelSize);
	this->createMainWindow(width, height, name);

	// setup the sound system
	this->soundSystem.setOutputStreamName(name);
	this->soundSystem.setOutputFunction(
			std::bind(
					&ExampleSound::generateSound,
					this,
					std::placeholders::_1,
					std::placeholders::_2,
					true
			)
	);

	// run the engine
	this->Engine::run();

	return EXIT_SUCCESS;
}

// set the general master volume (default: 0.7)
void ExampleSound::setMasterVolume(double master) {
	if(master < 0.)
		throw std::runtime_error("Invalid master volume");

	if(master > 1.)
		this->masterVolume = 1.;
	else
		this->masterVolume = master;
}

// set the maximum volume for clipping (default: 0.8)
void ExampleSound::setMaxVolume(double max) {
	if(max < 0.)
		throw std::runtime_error("Invalid maximum volume");

	if(max > 0.995)
		this->maxVolume = 0.995;
	else
		this->maxVolume = max;
}

// create resources
void ExampleSound::onCreate() {
	// start the sound system
	this->soundSystem.start(this->getTime());

	// wait for the sound system to be ready
	while(!(this->soundSystem.isStarted()))
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// query for information about the sound output and print it to stdout
	std::cout << "device=" << this->soundSystem.getOutputDeviceName() << std::endl;
	std::cout << "samplerate=" << this->soundSystem.getOutputSampleRate() << std::endl;

	const auto latency = this->soundSystem.getOutputLatency();

	if(latency > 0.)
		std::cout << "latency=" << latency << "s" << std::endl;
	else
		std::cout << "latency=<unknown>" << std::endl;

	std::cout << "layout=" << this->soundSystem.getOutputLayoutName() << std::endl;

	const auto channels = this->soundSystem.getOutputChannels();

	std::cout << "channels=" << channels << std::endl;

	for(auto channel = 0; channel < channels; ++channel)
		std::cout << "\t#" << channel << ": " << this->soundSystem.getOutputChannelName(channel) << std::endl;
}

// update frame
void ExampleSound::onUpdate(double elapsedTime) {
	UNUSED(elapsedTime);

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
		const double sound = this->generateSound(0, currentTime + static_cast<double>(x) / w * res);

		int yFrom = static_cast<int>(std::lround(halfHeight));
		int yTo = static_cast<int>(std::lround(halfHeight + sound * halfHeight));
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
					static_cast<unsigned char>(std::lround(255 * red)),
					static_cast<unsigned char>(std::lround(255 - 255 * red)),
					0
			);
		}
	}

	// show number of sound waves and current wave resolution
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

	/*
	 * NOTE:	It might be better to use SoundWave::SOUNDWAVE_SAWTOOTH_OPTIMIZED,
	 * 			for sawtooth waves. Otherwise there might be 'underflow' warnings
	 * 			and choppy sound when computing too many sawtooth waves in parallel
	 * 			due to their very high calculation time involving multiple sinuses.
	 * 			Adjust analogSawToothN in SoundWave::get() to change the necessary
	 * 			computations per sample (being analogSawToothN * Math::approxSin(...)).
	 */
	if(this->isKeyPressed(GLFW_KEY_BACKSPACE))
		this->addSoundWave(SoundWave::SOUNDWAVE_SAWTOOTH);

	if(this->isKeyRepeated(GLFW_KEY_BACKSPACE))
		this->addSoundWave(SoundWave::SOUNDWAVE_SAWTOOTH);

	if(this->isKeyPressed(GLFW_KEY_N))
		this->addSoundWave(SoundWave::SOUNDWAVE_NOISE);

	if(this->isKeyRepeated(GLFW_KEY_N))
		this->addSoundWave(SoundWave::SOUNDWAVE_NOISE);

	if(this->isKeyPressed(GLFW_KEY_ESCAPE))
		this->clearSoundWaves();

	if(this->isKeyRepeated(GLFW_KEY_ESCAPE))
		this->clearSoundWaves();
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
	// generate semi-random frequency and random length
	constexpr double octaveBase = 110.;
	constexpr double twelfthRootOf2 = std::pow(2., 1. / 12.);

	const double frequency = octaveBase * std::pow(twelfthRootOf2, this->randomGenerator.generateByte());
	const double length = this->randomGenerator.generateReal();
	const double start = this->getTime();

	// set your envelope here
	//const SoundEnvelope envelope(SoundEnvelope::ADSRTimes(0., length, 0.), 1., 0.);
	const SoundEnvelope envelope(SoundEnvelope::ADSRTimes(0.1, 0.01, 0.2), 1., 0.8);

	/*
	 * NOTE:	Noise will be generated on-the-fly and therefore won't be rendered correctly,
	 * 			another pseudo-random noise will be rendered instead substituting for the
	 * 			actual noise that is being sent to the output sound device.
	 */

	// add sound wave to the main thread and render it immediately
	this->soundWavesForMain.emplace_back(
			SoundWave::Properties(type, frequency, length, start),
			envelope,
			&(this->noiseGeneratorMain)
	);

	this->soundWavesForMain.back().start(start);

	// add sound wave to the sound thread and play it immediately
	{
		std::lock_guard<std::mutex> threadDataLock(this->lockSoundWavesForThread);

		this->soundWavesForThread.emplace_back(
				SoundWave::Properties(type, frequency, length, start),
				envelope,
				&(this->noiseGeneratorThread)
		);

		this->soundWavesForThread.back().start(start);
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
double ExampleSound::generateSound(unsigned int channel, double time, bool forThread) {
	UNUSED(channel);

	if(forThread) {
		/*
		 * NOTE:	It is considered bad practise to use locking inside sound output code,
		 * 			because it runs the risk of temporary breakdowns in the sound output.
		 * 			Keep that in mind if you want to dive deeper into audio programming !
		 */

		std::lock_guard<std::mutex> threadDataLock(this->lockSoundWavesForThread);

		return this->generateSoundFrom(time, this->soundWavesForThread);
	}

	return this->generateSoundFrom(time, this->soundWavesForMain);
}

// generate sound at the specified time from the specified source
double ExampleSound::generateSoundFrom(double time, std::vector<SoundWave>& from) {
	if(from.empty())
		return 0.;

	// mixing
	double result = 0.;

	for(auto& wave : from)
		result += this->masterVolume * wave.get(time);

	// clipping
	if(result > this->maxVolume)
		result = this->maxVolume;

	if(result < - this->maxVolume)
		result = - this->maxVolume;

	return result;
}
