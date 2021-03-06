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
		  noiseGenerator(Rand::RAND_ALGO_LEHMER32),
		  commandsToIntermediary(maxSoundWaves),
		  soundWavesToAudioThread(maxSoundWaves),
		  indexNextSoundWaveIntermediary(0),
		  indexNextSoundWaveAudioThread(0),
		  isClearSoundWaves(false),
		  numSoundWaves(0) {
	// setup random generators
	this->randomGenerator.setRealLimits(0.1, 1.5);		// wave lengths between 0.1 and 1.5 seconds
	this->randomGenerator.setByteLimits(0, 47);			// 48 tones over three octaves
	this->noiseGenerator.setRealLimits(-1., 1.);
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
		std::this_thread::yield();

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

	// start intermediary thread
	this->intermediary = std::thread(&ExampleSound::threadIntermediary, this);
}

// update frame
void ExampleSound::onUpdate(double elapsedTime) {
	UNUSED(elapsedTime);

	const double currentTime = this->getTime();

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
			using std::swap;

			swap(yFrom, yTo);

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

	// check for errors
	std::string writingError, errorString;

	if(this->soundSystem.isOutputUnderflowOccured())
		errorString = ", UNDERFLOW ERROR";
	else if(this->soundSystem.isOutputWritingErrorsOccured(writingError))
		errorString += ", ERROR: " + writingError;

	// show number of sound waves and current wave resolution
	this->setDebugText(
			"n="
			+ std::to_string(this->numSoundWaves)
			+ ", res="
			+ std::to_string(this->waveResolution)
			+ "ms"
			+ errorString
	);

	// handle RIGHT/LEFT arrow keys for changing the resolution of the displayed sound wave
	if(this->isKeyPressed(GLFW_KEY_RIGHT) && this->waveResolution < 100)
		++(this->waveResolution);

	if(this->isKeyRepeated(GLFW_KEY_RIGHT) && this->waveResolution < 100)
		++(this->waveResolution);

	if(this->isKeyPressed(GLFW_KEY_LEFT) && this->waveResolution > 1)
		--(this->waveResolution);

	if(this->isKeyRepeated(GLFW_KEY_LEFT) && this->waveResolution > 1)
		--(this->waveResolution);

	std::vector<Command> commands;

	// handle ENTER key for adding a sine wave
	if(this->isKeyPressed(GLFW_KEY_ENTER))
		commands.emplace_back(ACTION_ADD_SINE);

	if(this->isKeyRepeated(GLFW_KEY_ENTER))
		commands.emplace_back(ACTION_ADD_SINE);

	// handle SPACE key for adding a square wave
	if(this->isKeyPressed(GLFW_KEY_SPACE))
		commands.emplace_back(ACTION_ADD_SQUARE);

	if(this->isKeyRepeated(GLFW_KEY_SPACE))
		commands.emplace_back(ACTION_ADD_SQUARE);

	// handle TAB key for adding a triangle wave
	if(this->isKeyPressed(GLFW_KEY_TAB))
		commands.emplace_back(ACTION_ADD_TRIANGLE);

	if(this->isKeyRepeated(GLFW_KEY_TAB))
		commands.emplace_back(ACTION_ADD_TRIANGLE);

	// handle BACKSPACE key for adding a sawtooth wave
	if(this->isKeyPressed(GLFW_KEY_BACKSPACE))
		commands.emplace_back(ACTION_ADD_SAWTOOTH);

	if(this->isKeyRepeated(GLFW_KEY_BACKSPACE))
		commands.emplace_back(ACTION_ADD_SAWTOOTH);

	// handle N key for adding noise
	if(this->isKeyPressed(GLFW_KEY_N))
		commands.emplace_back(ACTION_ADD_NOISE);

	if(this->isKeyRepeated(GLFW_KEY_N))
		commands.emplace_back(ACTION_ADD_NOISE);

	// handle ESCAPE key for removing all sound waves
	if(this->isKeyPressed(GLFW_KEY_ESCAPE))
		commands.emplace_back(ACTION_CLEAR);

	if(this->isKeyRepeated(GLFW_KEY_ESCAPE))
		commands.emplace_back(ACTION_CLEAR);

	// send commands to intermediary thread
	if(!commands.empty())
		this->commandsToIntermediary.push(commands);

	// handle UP/DOWN arrow keys for changing the 'pixel' size
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

	// handle F10-F12 keys for changing the rendering mode
	const auto currentRenderingMode = this->getRenderingMode();
	auto newRenderingMode = currentRenderingMode;

	if(this->isKeyPressed(GLFW_KEY_F10))
		newRenderingMode = MainWindow::RENDERING_MODE_PBO;

	if(this->isKeyPressed(GLFW_KEY_F11))
		newRenderingMode = MainWindow::RENDERING_MODE_POINTS;

	if(this->isKeyPressed(GLFW_KEY_F12))
		newRenderingMode = MainWindow::RENDERING_MODE_TEXTURE;

	if(newRenderingMode != currentRenderingMode)
		this->setRenderingMode(newRenderingMode);
}

// clear resources
void ExampleSound::onDestroy() {
	// stop the sound system
	this->soundSystem.stop();

	// wait for the intermediary thread
	Command quit(ACTION_QUIT);

	this->commandsToIntermediary.push(quit);

	if(this->intermediary.joinable())
		this->intermediary.join();

	// clear buffers
	this->commandsToIntermediary.clear();
	this->soundWavesToAudioThread.clear();
}

// intermediary thread for creating sound waves
void ExampleSound::threadIntermediary() {
	bool running = true;

	do {
		// get commands
		std::vector<Command> commands;

		this->commandsToIntermediary.pop(commands);

		for(const auto& command : commands) {
			switch(command.action) {
			case ACTION_NONE:
				break;

			case ACTION_ADD_SINE:
				this->addSoundWave(SoundWave::SOUNDWAVE_SINE);

				break;

			case ACTION_ADD_SQUARE:
				this->addSoundWave(SoundWave::SOUNDWAVE_TRIANGLE);

				break;

			case ACTION_ADD_TRIANGLE:
				this->addSoundWave(SoundWave::SOUNDWAVE_TRIANGLE);

				break;

			case ACTION_ADD_SAWTOOTH:
				/*
				 * NOTE:	It might be better to use SoundWave::SOUNDWAVE_SAWTOOTH_OPTIMIZED,
				 * 			for sawtooth waves. Otherwise there might be 'underflow' warnings
				 * 			and choppy sound when computing too many sawtooth waves in parallel
				 * 			due to their very high calculation time involving multiple sinuses.
				 * 			Adjust analogSawToothN in SoundWave::get() to change the necessary
				 * 			computations per sample (analogSawToothN * Math::approxSinQuad(...)).
				 */

				this->addSoundWave(SoundWave::SOUNDWAVE_SAWTOOTH);

				break;

			case ACTION_ADD_NOISE:
				this->addSoundWave(SoundWave::SOUNDWAVE_NOISE_PRECALCULATED);

				break;

			case ACTION_CLEAR:
				this->clearSoundWaves();

				break;

			case ACTION_QUIT:
				running = false;

				break;

			default:
				throw std::runtime_error("Unknown command: " + std::to_string(command.action));
			}
		}

		// yield some time to other threads
		std::this_thread::yield();
	} while(running);
}

// add a sound wave
void ExampleSound::addSoundWave(SoundWave::Type type) {
	// generate semi-random frequency and random length
	constexpr double octaveBase = 110.;
	constexpr double twelfthRootOf2 = std::pow(2., 1. / 12.);

	const double frequency = octaveBase * std::pow(twelfthRootOf2, this->randomGenerator.generateByte());
	const double length = this->randomGenerator.generateReal();

	// save start time
	const double start = this->getTime();

	// set noise resolution (i.e. the percentage of actually pre-calculated samples)
	constexpr double noiseResolution = 1.;

	// set your envelope here
	//const SoundEnvelope envelope(SoundEnvelope::ADSRTimes(0., length, 0.), 1., 0.);
	const SoundEnvelope envelope(SoundEnvelope::ADRTimes(0.1, 0.01, 0.2), 1., 0.8);

	// generate noise if necessary
	std::vector<double> noise;
	double samplesPerSecond = 0.;

	if(type == SoundWave::SOUNDWAVE_NOISE_PRECALCULATED) {
		samplesPerSecond = this->soundSystem.getOutputSampleRate() / (length + envelope.getADRTimes().decayTime);

		const auto samples =
				static_cast<std::size_t>(
						(length + envelope.getADRTimes().releaseTime) * samplesPerSecond * noiseResolution
				) + 1;

		noise.reserve(samples);

		for(std::size_t n = 0; n < samples; ++n)
			noise.push_back(this->noiseGenerator.generateReal());
	}

	// add sound wave to the circular buffer for the sound thread and play it "immediately"
	SoundWave soundWaveForThread(
			SoundWave::Properties(type, frequency, length, start),
			envelope,
			nullptr,
			&noise,
			samplesPerSecond
	);

	soundWaveForThread.start(start);

	// try to add the sound wave to the circular buffer that communicates with the sound thread
	//	as long as the buffer still has capacity (i.e. is not cleared),
	while(
			!(this->soundWavesToAudioThread.push(soundWaveForThread))
			&& this->soundWavesToAudioThread.capacity()
	) std::this_thread::yield(); // in the meantime, yield some execution time to other threads

	{
		// lock sound waves for intermediary thread
		std::lock_guard<std::mutex> accessToIntermediary(this->lock);

		// add sound wave to the intermediary thread and render it immediately
		SoundWave newSoundWave(
				SoundWave::Properties(type, frequency, length, start),
				envelope,
				nullptr,
				&noise,
				samplesPerSecond
		);

		newSoundWave.start(start);

		using std::swap;

		swap(newSoundWave, this->soundWavesForIntermediary[this->indexNextSoundWaveIntermediary]);

		++(this->indexNextSoundWaveIntermediary);

		if(this->indexNextSoundWaveIntermediary == ExampleSound::maxSoundWaves)
			this->indexNextSoundWaveIntermediary = 0;
	}
}

// clear all sound waves
void ExampleSound::clearSoundWaves() {
	// clear data for the intermediary thread
	for(unsigned char i = 0; i < ExampleSound::maxSoundWaves; ++i)
		this->soundWavesForIntermediary[i].clear();

	// notify the sound thread to clear its data
	this->isClearSoundWaves.store(true);
}

// generate sound at the specified time
double ExampleSound::generateSound(unsigned int channel, double time, bool forAudioThread) {
	UNUSED(channel);

	if(forAudioThread) { /* inside the audio thread */
		// add new sound wave
		if(this->soundWavesToAudioThread.pop(this->soundWavesForAudioThread[this->indexNextSoundWaveAudioThread])) {
			++(this->indexNextSoundWaveAudioThread);

			if(this->indexNextSoundWaveAudioThread == ExampleSound::maxSoundWaves)
				this->indexNextSoundWaveAudioThread = 0;
		}

		bool changeIfValueIs = true;

		if(
				this->isClearSoundWaves.compare_exchange_weak(
						changeIfValueIs,
						false,
						std::memory_order_relaxed,
						std::memory_order_release
				)
		)
			// clear all sound waves
			for(unsigned char i = 0; i < ExampleSound::maxSoundWaves; ++i)
				this->soundWavesForAudioThread[i].clear();

		return this->generateSoundFrom(
				time,
				this->soundWavesForAudioThread,
				ExampleSound::maxSoundWaves
		);
	}

	// in main thread: only render the new value if access to the data of the intermediary thread is given
	{
		std::unique_lock<std::mutex> accessToIntermediary(this->lock, std::try_to_lock);

		if(accessToIntermediary.owns_lock())
			return this->generateSoundFrom(
					time,
					this->soundWavesForIntermediary,
					ExampleSound::maxSoundWaves,
					true
			);
	}

	return 0.;
}

// generate sound at the specified time from the specified source
double ExampleSound::generateSoundFrom(double time, SoundWave * from, std::size_t n, bool count) {
	if(!n)
		return 0.;

	if(count)
		this->numSoundWaves = 0;

	// mixing
	double result = 0.;

	for(std::size_t i = 0; i < n; ++i)
		if(from[i].exists()) {
			result += this->masterVolume * from[i].get(time);

			if(count)
				++(this->numSoundWaves);
		}

	// clipping
	if(result > this->maxVolume)
		result = this->maxVolume;

	if(result < - this->maxVolume)
		result = - this->maxVolume;

	return result;
}
