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

	/*
	 * NOTE:	Debugging will turn the pixels
	 * 			RED for which the test fails,
	 * 			instead of not drawing them.
	 */
	bool debugging;

	PixelTest() : debugging(false) {}

	operator bool() const { return init && frame && test; }
};

#endif /* PIXELTEST_H_ */
