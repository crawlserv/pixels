/*
 * PixelTest.h
 *
 *  Created on: Apr 11, 2020
 *      Author: ans
 */

#ifndef PIXELTEST_H_
#define PIXELTEST_H_

#include <functional>	// std::function

struct PixelTest {
	using InitFunction = std::function<void(unsigned int, unsigned int)>;
	using FrameFunction = std::function<void()>;
	using TestFunction = std::function<bool(unsigned int, unsigned int)>;

	InitFunction init;
	FrameFunction frame;
	TestFunction test;

	operator bool() const { return init && test; }
};

#endif /* PIXELTEST_H_ */
