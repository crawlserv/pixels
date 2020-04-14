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
		  initialized(false),
		  running(false),
		  secondsOffset(0.),
		  secondsPerFrame(0.),
		  soundIo(nullptr),
		  soundIoOutputDevice(nullptr),
		  soundIoOutStream(nullptr),
		  defaultOutputDeviceIndex(-1),
		  outputDeviceIndex(-1),
		  outputSampleRate(44100),
		  outputChannels(0),
		  outputMaxFrames(2048),
		  outputLatency(0.1) {
	// create soundio context
	this->soundIo = soundio_create();

	if(!(this->soundIo))
		throw std::runtime_error("soundio_create failed");

	// connect to the backend
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

// get the index of the default output device or -1 if none is available
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

// get the name of the underlying backend used for communicating with the sound device(s)
std::string Sound::getBackend() const {
	if(!(this->soundIo))
		throw std::runtime_error("Sound::getBackend(): soundIo == nullptr");

	switch(this->soundIo->current_backend) {
	case SoundIoBackendNone:
		return "<none>";

	case SoundIoBackendJack:
		return "Jack";

	case SoundIoBackendPulseAudio:
		return "PulseAudio";

	case SoundIoBackendAlsa:
		return "ALSA";

	case SoundIoBackendCoreAudio:
		return "Core Audio";

	case SoundIoBackendWasapi:
		return "Wasapi";

	case SoundIoBackendDummy:
		return "<dummy>";
	}

	return "<unknown>";
}

// set the current output device by its index (e.g. when received from Sound::listOutputDevices)
void Sound::setOutputDeviceByIndex(unsigned int index) {
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

// set the sample rate (default: 44100, zero means device/library default)
void Sound::setOutputSampleRate(unsigned int sampleRate) {
	this->outputSampleRate = sampleRate;
}

// set the number of channels (default: 0, zero means device/library default)
void Sound::setOutputChannels(unsigned int channels) {
	this->outputChannels = channels;
}

// set the maximum amount of frames to output at once (default: 1024, zero means unlimited)
//	NOTE: The same amount of frames will be pre-buffered and this setting therefore affects the latency.
//		  Although zero means unlimited, a maximum of outputSampleRate / 10 frames (for 100ms) will be buffered.
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

// check whether sound system has been succesfully started (thread-safe!)
bool Sound::isStarted() const {
	return this->initialized;
}

// get the actual output sample rate chosen by the device
// 	NOTE:	The sound system need to be completely started before you can use this function.
//			You can use Sound::isStarted() to wait until it is ready.
int Sound::getOutputSampleRate() const {
	if(!(this->soundIoOutStream)) {
		std::cerr
				<<	"WARNING: Sound::getChannelType(): "
					"Sound system needs to be started before getting the type of a channel"
				<<	std::endl;

		return 0;
	}

	return this->soundIoOutStream->sample_rate;
}

// get the name of the currently selected layout
// 	NOTE:	The sound system need to be completely started before you can use this function.
//			You can use Sound::isStarted() to wait until it is ready.
std::string Sound::getOutputLayoutName() const {
	if(!(this->soundIoOutStream)) {
		std::cerr
				<<	"WARNING: Sound::getChannelType(): "
					"Sound system needs to be started before getting the type of a channel"
				<<	std::endl;

		return 0;
	}

	return this->soundIoOutStream->layout.name;
}

// get the number of output channels
// 	NOTE:	The sound system need to be completely started before you can use this function.
//			You can use Sound::isStarted() to wait until it is ready.
int Sound::getOutputChannels() const {
	if(!(this->soundIoOutStream)) {
		std::cerr
				<<	"WARNING: Sound::getOutputChannelType(): "
					"Sound system needs to be started before getting the type of a channel"
				<<	std::endl;

		return 0;
	}

	return this->soundIoOutStream->layout.channel_count;
}

// get the actual software latency (might return zero for low latencies depending on the device)
// 	NOTE:	The sound system need to be completely started before you can use this function.
//			You can use Sound::isStarted() to wait until it is ready.
double Sound::getOutputLatency() const {
	if(!(this->soundIoOutStream)) {
		std::cerr
				<<	"WARNING: Sound::getOutputChannelType(): "
					"Sound system needs to be started before getting the type of a channel"
				<<	std::endl;

		return 0.;
	}

	return this->soundIoOutStream->software_latency;
}

// get the type of the channel with the specified index
// 	NOTE:	The sound system need to be completely started before you can use this function.
//			You can use Sound::isStarted() to wait until it is ready.
Sound::Channel Sound::getOutputChannelType(unsigned int channel) const {
	if(!(this->soundIoOutStream)) {
		std::cerr
				<<	"WARNING: Sound::getOutputChannelType(): "
					"Sound system needs to be started before getting the type of a channel"
				<<	std::endl;

		return CHANNEL_NONE;
	}

	if(this->soundIoOutStream->layout.channel_count < 0)
		return CHANNEL_NONE;

	if(channel > static_cast<unsigned int>(this->soundIoOutStream->layout.channel_count))
		return CHANNEL_NONE;

	switch(this->soundIoOutStream->layout.channels[channel]) {
	case SoundIoChannelIdInvalid:
		return CHANNEL_NONE;

	case SoundIoChannelIdFrontLeft:
		return CHANNEL_FRONT_LEFT;

	case SoundIoChannelIdFrontRight:
		return CHANNEL_FRONT_RIGHT;

	case SoundIoChannelIdFrontRightCenter:
		return CHANNEL_FRONT_RIGHT_CENTER;

	case SoundIoChannelIdFrontCenter:
		return CHANNEL_FRONT_CENTER;

	case SoundIoChannelIdBackLeft:
		return CHANNEL_BACK_LEFT;

	case SoundIoChannelIdBackRight:
		return CHANNEL_BACK_RIGHT;

	case SoundIoChannelIdFrontLeftCenter:
		return CHANNEL_FRONT_LEFT;

	case SoundIoChannelIdBackCenter:
		return CHANNEL_BACK_CENTER;

	case SoundIoChannelIdSideLeft:
		return CHANNEL_SIDE_LEFT;

	case SoundIoChannelIdSideRight:
		return CHANNEL_SIDE_RIGHT;

	case SoundIoChannelIdTopCenter:
		return CHANNEL_TOP_CENTER;

	case SoundIoChannelIdTopFrontLeft:
		return CHANNEL_TOP_FRONT_LEFT;

	case SoundIoChannelIdTopFrontCenter:
		return CHANNEL_TOP_FRONT_CENTER;

	case SoundIoChannelIdTopFrontRight:
		return CHANNEL_TOP_FRONT_RIGHT;

	case SoundIoChannelIdTopBackLeft:
		return CHANNEL_TOP_BACK_LEFT;

	case SoundIoChannelIdTopBackCenter:
		return CHANNEL_TOP_BACK_CENTER;

	case SoundIoChannelIdTopBackRight:
		return CHANNEL_TOP_BACK_RIGHT;

	case SoundIoChannelIdLfe:
		return CHANNEL_SUBWOOFER;

	default:
		return CHANNEL_OTHER;
	}
}

// get the name of the channel with the specified index
// 	NOTE:	The sound system need to be completely started before you can use this function.
//			You can use Sound::isStarted() to wait until it is ready.
std::string Sound::getOutputChannelName(unsigned int channel) const {
	if(!(this->soundIoOutStream)) {
		std::cerr
				<<	"WARNING: Sound::getChannelType(): "
					"Sound system needs to be started before getting the type of a channel"
				<<	std::endl;

		return "<unknown>";
	}

	if(this->soundIoOutStream->layout.channel_count < 0)
		return "<none>";

	if(channel > static_cast<unsigned int>(this->soundIoOutStream->layout.channel_count))
		return "<none>";

	switch(this->soundIoOutStream->layout.channels[channel]) {
	case SoundIoChannelIdInvalid:
		return "<none>";

	case SoundIoChannelIdFrontLeft:
		return "Front left";

	case SoundIoChannelIdFrontRight:
		return "Front right";

	case SoundIoChannelIdFrontRightCenter:
		return "Front right center";

	case SoundIoChannelIdFrontCenter:
		return "Front center";

	case SoundIoChannelIdBackLeft:
		return "Back left";

	case SoundIoChannelIdBackRight:
		return "Back right";

	case SoundIoChannelIdFrontLeftCenter:
		return "Front left";

	case SoundIoChannelIdBackCenter:
		return "Back center";

	case SoundIoChannelIdSideLeft:
		return "Side leftt";

	case SoundIoChannelIdSideRight:
		return "Side right";

	case SoundIoChannelIdTopCenter:
		return "Top center";

	case SoundIoChannelIdTopFrontLeft:
		return "Top front left";

	case SoundIoChannelIdTopFrontCenter:
		return "Top front center";

	case SoundIoChannelIdTopFrontRight:
		return "Top front right";

	case SoundIoChannelIdTopBackLeft:
		return "Top back left";

	case SoundIoChannelIdTopBackCenter:
		return "Top back center";

	case SoundIoChannelIdTopBackRight:
		return "Top back right";

	case SoundIoChannelIdLfe:
		return "Subwoofer";

	default:
		return "<other>";
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

	if(this->outputChannels)
		this->soundIoOutStream->layout = this->getLayout();

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
	this->secondsPerFrame = 1. / this->soundIoOutStream->sample_rate;

	// done
	this->initialized = true;
}

// clear thread-related resources
void Sound::threadClear() {
	this->initialized = false;

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
	if(!(this->soundIoOutStream))
		throw std::runtime_error("Sound::onWrite(): soundIoOutStream == nullptr");

	const auto * pointerToLayout = &(this->soundIoOutStream->layout);
	SoundIoChannelArea * pointerToAreas = nullptr;

	if(frameCountMin < 0)
		throw std::runtime_error("Sound::onWrite(): frameCountMin < 0");

	if(frameCountMax < 0)
		throw std::runtime_error("Sound::onWrite(): frameCountMax < 0");

	if(pointerToLayout->channel_count <= 0)
		throw std::runtime_error("Sound::onWrite(): no channels to write sound to");

	const unsigned int channelCount = static_cast<unsigned int>(pointerToLayout->channel_count);

	// write a decent number of frames
	auto framesLeft = frameCountMax;

	if(this->outputMaxFrames && framesLeft > this->outputMaxFrames && frameCountMin < framesLeft) {
		if(this->outputMaxFrames > frameCountMin)
			framesLeft = this->outputMaxFrames;
		else
			framesLeft = frameCountMin;
	}

	// loop through the frames
	while(framesLeft) {
		auto frameCount = framesLeft;

		// start writing to the stream
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
			// get sample from the callback function
			for(unsigned int channel = 0; channel < channelCount; ++channel) {
				const auto sample = this->output(channel, this->secondsOffset + frame * this->secondsPerFrame);

				this->write(pointerToAreas[channel].ptr, sample > 1. ? 1. : sample);

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
	if(!(this->soundIo))
		throw std::runtime_error("Sound::setOutputDevice(): this->soundio == nullptr");


	bool restart = false;

	if(this->running) {
		this->stop();

		restart = true;
	}

	if(index < 0)
		this->outputDeviceIndex = this->defaultOutputDeviceIndex;
	else if(index >= soundio_output_device_count(this->soundIo))
		throw std::runtime_error("Sound::setOutputDevice(): Invalid output device #" + std::to_string(index));
	else
		this->outputDeviceIndex = index;

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

// get the pointer to a layout fitting the specified options (i.e. channels) or to the default if none was found
const SoundIoChannelLayout& Sound::getLayout() const {
	if(!(this->soundIoOutputDevice))
		throw std::runtime_error("The sound system needs to be started before getting information about the output layout");

	for(auto layout = 0; layout < this->soundIoOutputDevice->layout_count; ++layout)
		if(
				this->soundIoOutputDevice->layouts[layout].channel_count > 0
				&& static_cast<unsigned int>(
						this->soundIoOutputDevice->layouts[layout].channel_count
				) == this->outputChannels
		)
			return this->soundIoOutputDevice->layouts[layout];

	return this->soundIoOutputDevice->current_layout;
}

// write a 16-bit integer sample
void Sound::writeSampleS16(void * target, double sample) {
	int16_t * buffer = static_cast<int16_t *>(target);
	double range = static_cast<double>(std::numeric_limits<int16_t>::max())
					- static_cast<double>(std::numeric_limits<int16_t>::min());

	*buffer = static_cast<int16_t>(sample * range / 2.);
}

// write a 32-bit integer sample
void Sound::writeSampleS32(void * target, double sample) {
	int32_t * buffer = static_cast<int32_t *>(target);
	double range = static_cast<double>(std::numeric_limits<int32_t>::max())
					- static_cast<double>(std::numeric_limits<int32_t>::min());

	*buffer = static_cast<int32_t>(sample * range / 2.);
}

// write a 32-bit float sample
void Sound::writeSampleF32(void * target, double sample) {
	float * buffer = static_cast<float *>(target);

	*buffer = static_cast<float>(sample);
}

// write a 64-bit float sample
void Sound::writeSampleF64(void * target, double sample) {
	double * buffer = static_cast<double *>(target);

	*buffer = sample;
}
