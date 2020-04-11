/*
 * ExampleRects.h
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

#ifndef EXAMPLERECTS_H_
#define EXAMPLERECTS_H_

#include "Engine.h"
#include "Geometry.h"

#include <algorithm>// std::replace
#include <cmath>	// std::roundf
#include <cstdlib>	// EXIT_SUCCESS
#include <random>	// std::mt19937, std::random_device, std::uniform_int_distribution
#include <vector>	// std::vector

class ExampleRects : Engine {
	template<typename T>
	struct Color {
		T r;
		T g;
		T b;

		Color() : r(0), g(0), b(0) {}
		Color(T _r, T _g, T _b) : r(_r), g(_g), b(_b) {}
	};

	using Rect = Geometry::Rectangle<float, Color<unsigned char>>;

public:
	ExampleRects();
	virtual ~ExampleRects();

	int run(int argc, char * argv[]);

private:
	void onCreate() override;
	void onUpdate(double elapsedTime) override;

	void add();
	void render(int w, int h, const Rect& rect);

	void pixelTestInit(unsigned int w, unsigned int h);
	void pixelTestFrame();
	bool pixelTestTest(unsigned int x, unsigned int y);
	std::vector<bool> pixelTest;
	unsigned int pixelTestW;

	unsigned short pixelSize;
	std::random_device rd;
	std::mt19937 mt;
	std::uniform_real_distribution<float> distReal;
	std::uniform_int_distribution<unsigned char> distInt;

	bool renderBorders;
	bool testPixels;

	std::vector<Rect> rects;
};

#endif /* EXAMPLERECTS_H_ */
