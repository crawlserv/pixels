/*
 * Sound.cpp
 *
 *  Created on: Apr 12, 2020
 *      Author: ans
 */

#include "Sound.h"

// constructor initializing the libsoundio library
Sound::Sound()
		: started(false),
		  connected(false),
		  running(false),
		  secondsOffset(0.),
		  secondsPerFrame(0.),
		  soundIo(nullptr),
		  soundIoOutputDevice(nullptr),
		  soundIoOutStream(nullptr),
		  defaultOutputDeviceIndex(-1),
		  outputDeviceIndex(-1),
		  outputSampleRate(44100),
		  outputMaxFrames(0),
		  outputLatency(0.1) {
	// create soundio context
	this->soundIo = soundio_create();

	if(!(this->soundIo))
		throw std::runtime_error("soundio_create failed");

	// connect to backend
	int error = soundio_connect(this->soundIo);

	if(error)
		throw std::runtime_error(soundio_strerror(error));

	// set state, callbacks and user pointer
	this->connected = true;
	this->soundIo->on_devices_change = Sound::callbackDevicesChanged;
	this->soundIo->on_backend_disconnect = Sound::callbackBackendDisconnected;
	this->soundIo->userdata = static_cast<void *>(this);

	// flush events
	soundio_flush_events(this->soundIo);

	// get the default output device
	this->defaultOutputDeviceIndex = soundio_default_output_device_index(this->soundIo);

	if(this->defaultOutputDeviceIndex < 0)
		throw std::runtime_error("soundio_default_output_device_index failed");

	// use the default output device for now
	this->setOutputDevice(this->defaultOutputDeviceIndex);
}

// destructor
Sound::~Sound() {
	// stop the thread
	this->stop();

	// clear thread-related stuff (e.g. if the thread has been terminated by an exception)
	this->threadClear();

	// disconnect from the backend and destroy the soundio context
	if(this->soundIo) {
		if(this->connected) {
			soundio_disconnect(this->soundIo);

			this->connected = false;
		}

		soundio_destroy(this->soundIo);

		this->soundIo = nullptr;
	}
}

// write all available output devices into a vector and return it
std::vector<Sound::Device> Sound::listOutputDevices() const {
	// check context and state
	if(!(this->soundIo))
		throw std::runtime_error("listDevices(): soundio not initialized");

	const auto count = soundio_output_device_count(this->soundIo);

	if(!count)
		throw std::runtime_error("Could not find any output devices");

	std::vector<Device> result;

	result.reserve(count);

	for(auto n = 0; n < count; ++n) {
		auto * device = soundio_get_output_device(this->soundIo, n);

		if(!device)
			throw std::runtime_error(
					"soundio_get_output_device failed for device #"
					+ std::to_string(n)
		);

		result.emplace_back(
				n,
				device->id,
				device->name
		);

		soundio_device_unref(device);

		device = nullptr;
	}

	return result;
}

// get the index of the default output device
int Sound::getDefaultOutputDeviceIndex() const {
	return this->defaultOutputDeviceIndex;
}

// get the ID of the currently selected device (e.g. for saving it into a settings file)
std::string Sound::getOutputDeviceId() const {
	if(this->outputDeviceId.empty())
		throw std::runtime_error("No device selected");

	return this->outputDeviceId;
}

// get the name of the currently selected device
std::string Sound::getOutputDeviceName() const {
	return this->outputDeviceName;
}

// set the current output device by its index (e.g. when received from Sound::listOutputDevices)
void Sound::setOutputDeviceByIndex(int index) {
	this->setOutputDevice(index);
}

// set the current output device by its ID (e.g. when read from a settings file)
void Sound::setOutputDeviceById(const std::string& id) {
	const auto index = this->outputDeviceIdToIndex(id);

	if(index == -1)
		throw std::runtime_error("Could not find device with \'" + id + "\'");

	this->setOutputDevice(index);
}

// set the output function used for generating sound
void Sound::setOutputFunction(OutputFunction callBack) {
	this->output = callBack;
}

// set the name of the output stream (default: empty, empty string is API default ("SoundIoOutStream")
void Sound::setOutputStreamName(const std::string& streamName) {
	this->outputStreamName = streamName;
}

// set the sample rate (default: 44100, zero means device default)
void Sound::setOutputSampleRate(unsigned int sampleRate) {
	this->outputSampleRate = sampleRate;
}

// set the maximum amount of frames to output at once (default: 0, zero means device default)
//	WARNING: Low numbers will lead to 'underflow' and a warning will be printed to std::cerr
//			 that there has not been enough data to write to the output sound device !
//			 Very low numbers will break the audio output completely !
void Sound::setOutputMaxFrames(unsigned int maxFrames) {
	this->outputMaxFrames = maxFrames;
}

// set the desired software latency for the sound output (in seconds, default: 0.1, zero means device default)
//	WARNING: The device default might be a very high latency (e.g. 2s), better use the in-class default !
//			 Very low numbers will lead to 'underflow' and a warning will be printed to std::cerr
//			 that there has not been enough data to write to the output sound device !
void Sound::setOutputLatency(double latency) {
	this->outputLatency = latency;
}

// start the sound system in an extra thread
void Sound::start(double startTimeInSeconds) {
	if(this->running) {
		std::cerr << "Sound::start(): Sound system is already running." << std::endl;

		return;
	}

	this->started = true;
	this->secondsOffset = startTimeInSeconds;
	this->running = true;

	this->soundThread = std::thread(&Sound::thread, this);
}

// stop the sound system, notify the soundio library and wait for the thread
void Sound::stop() {
	if(this->started) {
		this->started = false;
		this->running = false;

		soundio_wakeup(this->soundIo);

		if(this->soundThread.joinable())
			this->soundThread.join();
	}
}

// thread function for the actual sound output
void Sound::thread() {
	// initialize thread-related resources
	this->threadInit();

	// handle sound events while the thread is running
	while(this->running)
		soundio_wait_events(this->soundIo);

	// clear thread-related resources
	this->threadClear();
}

// initialize thread-related resources
void Sound::threadInit() {
	// get the output device
	this->soundIoOutputDevice = soundio_get_output_device(this->soundIo, this->outputDeviceIndex);

	if(!(this->soundIoOutputDevice)) {
		std::cerr << "Sound::threadInit(): soundio_get_output_device failed." << std::endl;

		this->running = false;

		return;
	}

	// create the output stream
	this->soundIoOutStream = soundio_outstream_create(this->soundIoOutputDevice);

	if(!(this->soundIoOutStream)) {
		std::cerr << "Sound::threadInit(): soundio_outstream_create failed." << std::endl;

		this->running = false;

		return;
	}

	// set output options
	this->soundIoOutStream->format = SoundIoFormatFloat32NE;

	if(!(this->outputStreamName.empty()))
		this->soundIoOutStream->name = this->outputStreamName.c_str();

	if(this->outputSampleRate)
		this->soundIoOutStream->sample_rate = this->outputSampleRate;

	this->soundIoOutStream->write_callback = Sound::callbackWrite;
	this->soundIoOutStream->underflow_callback = Sound::callbackUnderflow;

	if(this->outputLatency > 0.)
		this->soundIoOutStream->software_latency = this->outputLatency;

	// choose output format
	if(soundio_device_supports_format(this->soundIoOutputDevice, SoundIoFormatFloat64NE)) {
		this->soundIoOutStream->format = SoundIoFormatFloat64NE;
		this->write = writeSampleF64;
	}
	else if(soundio_device_supports_format(this->soundIoOutputDevice, SoundIoFormatFloat32NE)) {
		this->soundIoOutStream->format = SoundIoFormatFloat32NE;
		this->write = writeSampleF32;
	}
	else if(soundio_device_supports_format(this->soundIoOutputDevice, SoundIoFormatS32NE)) {
		this->soundIoOutStream->format = SoundIoFormatS32NE;
		this->write = writeSampleS32;
	}
	else if(soundio_device_supports_format(this->soundIoOutputDevice, SoundIoFormatS16NE)) {
		this->soundIoOutStream->format = SoundIoFormatS16NE;
		this->write = writeSampleS16;
	}
	else {
		std::cerr	<< "Sound::threadInit():"
						" Sound device does not support"
						" the available output formats."
					<< std::endl;

		this->running = false;

		return;
	}

	// open the output stream
	const auto errorOpen = soundio_outstream_open(this->soundIoOutStream);

	if(errorOpen) {
		std::cerr << "Sound::threadInit(): " << soundio_strerror(errorOpen) << "." << std::endl;

		this->running = false;

		return;
	}

	if(this->soundIoOutStream->layout_error)
		std::cerr
				<< "Sound::threadInit(): "
				<< soundio_strerror(this->soundIoOutStream->layout_error)
				<< std::endl;

	// start the output stream
	const auto errorStart = soundio_outstream_start(this->soundIoOutStream);

	if(errorStart) {
		std::cerr << "Sound::threadInit(): " << soundio_strerror(errorStart) << "." << std::endl;

		this->running = false;

		return;
	}

	// calculate the number of seconds per sample
	this->secondsPerFrame = 1.f / this->soundIoOutStream->sample_rate;
}

// clear thread-related resources
void Sound::threadClear() {
	if(this->soundIoOutStream) {
		soundio_outstream_destroy(this->soundIoOutStream);

		this->soundIoOutStream = nullptr;
	}

	if(this->soundIoOutputDevice) {
		soundio_device_unref(this->soundIoOutputDevice);

		this->soundIoOutputDevice = nullptr;
	}
}

// devices have been changed: check whether the output device remains valid
void Sound::onDevicesChanged() {
	// get default device
	this->defaultOutputDeviceIndex = soundio_default_output_device_index(this->soundIo);

	if(this->defaultOutputDeviceIndex < 0) {
		this->running = false;

		std::cerr << "Sound::onDevicesChanged(): Could not get new default output device." << std::endl;

		return;
	}

	if(!(this->outputDeviceId.empty())) {
		// check whether the index of the current device has changed
		const auto index = this->outputDeviceIdToIndex(this->outputDeviceId);

		// check whether current device has been lost
		if(index == -1)
			// reset to default device
			this->setOutputDevice(this->defaultOutputDeviceIndex);
		else
			// reset device if necessary
			if(index != this->outputDeviceIndex)
				this->setOutputDevice(index);
	}
}

// backend has been disconnected: shutdown the sound system
void Sound::onBackendDisconnected(int error) {
	this->running = false;

	std::cerr << "Sound system disconnected: " << soundio_strerror(error) << "." << std::endl;
}

// write to the sound output device using the provided callback function
void Sound::onWrite(int frameCountMin, int frameCountMax) {
	const auto * pointerToLayout = &(this->soundIoOutStream->layout);
	SoundIoChannelArea * pointerToAreas = nullptr;

	if(frameCountMin < 0)
		throw std::runtime_error("Sound::onWrite(): frameCountMin < 0");

	if(frameCountMax < 0)
		throw std::runtime_error("Sound::onWrite(): frameCountMax < 0");

	// write a decent number of frames
	auto framesLeft = frameCountMax;

	if(this->outputMaxFrames && framesLeft > this->outputMaxFrames && frameCountMin < framesLeft) {
		if(this->outputMaxFrames > frameCountMin)
			framesLeft = this->outputMaxFrames;
		else
			framesLeft = frameCountMin;
	}

	// loop through frames
	while(framesLeft) {
		auto frameCount = framesLeft;

		// start writing to stream
		const auto beginError = soundio_outstream_begin_write(this->soundIoOutStream, &pointerToAreas, &frameCount);

		if(beginError) {
			// cancel writing on error
			std::cerr << "Sound::onWrite(): " << soundio_strerror(beginError) << "." << std::endl;

			return;
		}

		if(!frameCount)
			// no frames to write
			return;

		for(auto frame = 0; frame < frameCount; ++frame) {
			// get sample from callback function
			const double sample = this->output(this->secondsOffset + frame * this->secondsPerFrame);

			for(auto channel = 0; channel < pointerToLayout->channel_count; ++channel) {
				this->write(pointerToAreas[channel].ptr, sample);

				pointerToAreas[channel].ptr += pointerToAreas[channel].step;
			}
		}

		this->secondsOffset += this->secondsPerFrame * frameCount;

		const auto endError = soundio_outstream_end_write(this->soundIoOutStream);

		if(endError) {
			std::cerr << "Sound::onWrite(): " << soundio_strerror(endError) << "." << std::endl;

			this->running = false;

			return;
		}

		framesLeft -= frameCount;
	}
}

// buffer has underflow
void Sound::onUnderflow() {
	std::cerr << "UNDERFLOW WARNING: Not enough data to write to the output sound device." << std::endl;
}

// delegate on devices changed event into the class
void Sound::callbackDevicesChanged(SoundIo * soundIo) {
	if(!soundIo)
		throw std::runtime_error("callbackDevicesChanged(): soundIo == nullptr");

	if(!soundIo->userdata)
		throw std::runtime_error("callbackDevicesChanged(): soundIo->userdata == nullptr");

	static_cast<Sound *>(soundIo->userdata)->onDevicesChanged();
}

// delegate on backend disconnected event into class
void Sound::callbackBackendDisconnected(SoundIo * soundIo, int error) {
	if(!soundIo)
		throw std::runtime_error("callbackBackendDisconnected(): soundIo == nullptr");

	if(!soundIo->userdata)
		throw std::runtime_error("callbackBackendDisconnected(): soundIo->userdata == nullptr");

	static_cast<Sound *>(soundIo->userdata)->onBackendDisconnected(error);
}

// delegate write request into class
void Sound::callbackWrite(SoundIoOutStream * soundIoOutStream, int frameCountMin, int frameCountMax) {
	if(!soundIoOutStream)
		throw std::runtime_error("callbackWrite(): soundIoOutStream == nullptr");

	if(!(soundIoOutStream->device))
		throw std::runtime_error("callbackWrite(): soundIoOutStream->device == nullptr");

	if(!(soundIoOutStream->device->soundio))
		throw std::runtime_error("callbackWrite(): soundIoOutStream->device->soundio == nullptr");

	if(!(soundIoOutStream->device->soundio->userdata))
		throw std::runtime_error("callbackWrite(): soundIoOutStream->device->soundio->userdata == nullptr");

	static_cast<Sound *>(soundIoOutStream->device->soundio->userdata)->onWrite(frameCountMin, frameCountMax);
}

// delegate underflow event into class
void Sound::callbackUnderflow(SoundIoOutStream * soundIoOutStream) {
	if(!soundIoOutStream)
		throw std::runtime_error("callbackUnderflow(): soundIoOutStream == nullptr");

	if(!(soundIoOutStream->device))
		throw std::runtime_error("callbackUnderflow(): soundIoOutStream->device == nullptr");

	if(!(soundIoOutStream->device->soundio))
		throw std::runtime_error("callbackUnderflow(): soundIoOutStream->device->soundio == nullptr");

	if(!(soundIoOutStream->device->soundio->userdata))
		throw std::runtime_error("callbackUnderflow(): soundIoOutStream->device->soundio->userdata == nullptr");

	static_cast<Sound *>(soundIoOutStream->device->soundio->userdata)->onUnderflow();
}

// helper function to find the index of the output device with the specified ID
int Sound::outputDeviceIdToIndex(const std::string& id) {
	const auto count = soundio_output_device_count(this->soundIo);

	if(!count) {
		this->running = false;

		std::cerr << "Sound::outputDeviceIdToIndex(): Could not find any output devices." << std::endl;

		return -1;
	}

	int index = -1;

	for(auto n = 0; n < count; ++n) {
		auto * device = soundio_get_output_device(this->soundIo, n);

		if(!device) {
			this->running = false;

			std::cerr << "Sound::outputDeviceIdToIndex(): Could not get output device #" << n << "." << std::endl;

			return -1;
		}

		const std::string deviceId(device->id);

		soundio_device_unref(device);

		device = nullptr;

		if(deviceId == id) {
			index = n;

			break;
		}
	}

	return index;
}

// helper function to set the current output device and restart the thread if necessary
void Sound::setOutputDevice(int index) {
	bool restart = false;

	if(this->running) {
		this->stop();

		restart = true;
	}

	this->outputDeviceIndex = this->defaultOutputDeviceIndex;

	auto * tmpDevice = soundio_get_output_device(this->soundIo, this->outputDeviceIndex);

	if(!tmpDevice) {
		std::cerr	<< "soundio_get_output_device failed for default device #"
					<< this->outputDeviceIndex
					<< "."
					<< std::endl;

		return;
	}

	this->outputDeviceId = tmpDevice->id;
	this->outputDeviceName = tmpDevice->name;

	soundio_device_unref(tmpDevice);

	tmpDevice = nullptr;

	if(restart)
		this->start(this->secondsOffset);
}

// write a 16-bit integer sample
void Sound::writeSampleS16(void * target, double sample) {
	int16_t * buffer = static_cast<int16_t *>(target);
	double range = static_cast<double>(std::numeric_limits<int16_t>::max())
					- static_cast<double>(std::numeric_limits<int16_t>::min());

	*buffer = sample * range / 2.;
}

// write a 32-bit integer sample
void Sound::writeSampleS32(void * target, double sample) {
	int32_t * buffer = static_cast<int32_t *>(target);
	double range = static_cast<double>(std::numeric_limits<int32_t>::max())
					- static_cast<double>(std::numeric_limits<int32_t>::min());

	*buffer = sample * range / 2.;
}

// write a 32-bit float sample
void Sound::writeSampleF32(void * target, double sample) {
	float * buffer = static_cast<float *>(target);

	*buffer = sample;
}

// write a 64-bit float sample
void Sound::writeSampleF64(void * target, double sample) {
	double * buffer = static_cast<double *>(target);

	*buffer = sample;
}
