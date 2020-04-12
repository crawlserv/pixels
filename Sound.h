/*
 * Sound.h
 *
 *  Created on: Apr 12, 2020
 *      Author: ans
 */

#ifndef SOUND_H_
#define SOUND_H_

#include <soundio/soundio.h>

#include <atomic>		// std::atomic
#include <functional>	// std::function
#include <iostream>		// std::cerr, std::endl
#include <limits>		// std::numeric_limits
#include <stdexcept>	// std::runtime_error
#include <string>		// std::string, std::to_string
#include <thread>		// std::thread
#include <vector>		// std::vector

class Sound {
public:
	using OutputFunction = std::function<double(double)>;
	using WriteFunction = std::function<void(void *, double)>;

	struct Device {
		int index;
		std::string id;
		std::string name;

		Device() : index(-1) {}
		Device(int _index, std::string _id, std::string _name)
				: index(_index), id(_id), name(_name) {}
	};

	Sound();
	virtual ~Sound();

	std::vector<Device> listOutputDevices() const;
	int getDefaultOutputDeviceIndex() const;
	std::string getOutputDeviceId() const;
	std::string getOutputDeviceName() const;

	void setOutputDeviceByIndex(int index);
	void setOutputDeviceById(const std::string& id);
	void setOutputFunction(OutputFunction callBack);
	void setOutputStreamName(const std::string& streamName);
	void setOutputSampleRate(unsigned int sampleRate);
	void setOutputMaxFrames(unsigned int maxFrames);

	void start(double startTimeInSeconds);
	void stop();

private:
	void thread();
	void threadInit();
	void threadClear();

	void onDevicesChanged();
	void onBackendDisconnected(int error);
	void onWrite(int frameCountMin, int frameCountMax);
	void onUnderflow();

	static void callbackDevicesChanged(SoundIo * soundIo);
	static void callbackBackendDisconnected(SoundIo * soundIo, int error);
	static void callbackWrite(SoundIoOutStream * soundIoOutStream, int frameCountMin, int frameCountMax);
	static void callbackUnderflow(SoundIoOutStream * soundIoOutStream);

	int outputDeviceIdToIndex(const std::string& id);
	void setOutputDevice(int index);

	static void writeSampleS16(void * target, double sample);
	static void writeSampleS32(void * target, double sample);
	static void writeSampleF32(void * target, double sample);
	static void writeSampleF64(void * target, double sample);

	bool started;
	bool connected;
	std::atomic<bool> running;
	std::thread soundThread;
	double secondsOffset;

	SoundIo * soundIo;
	SoundIoDevice * soundIoOutputDevice;
	SoundIoOutStream * soundIoOutStream;

	int defaultOutputDeviceIndex;
	int outputDeviceIndex;
	std::string outputDeviceId;
	std::string outputDeviceName;
	std::string outputStreamName;
	int outputSampleRate;

	OutputFunction output;
	WriteFunction write;
};

#endif /* SOUND_H_ */
