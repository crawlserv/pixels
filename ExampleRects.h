/*
 * ExampleRects.h
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

#ifndef EXAMPLERECTS_H_
#define EXAMPLERECTS_H_

#pragma once

#include "Engine.h"
#include "Geometry.h"
#include "Rand.h"

#include <cmath>		// std::lround
#include <cstdlib>		// EXIT_SUCCESS
#include <functional>	// std::bind, std::placeholders
#include <string>		// std::string, std::to_string
#include <utility>		// std::swap
#include <vector>		// std::vector

#define UNUSED(x) (void)(x)

class ExampleRects : Engine {
	template<typename T>
	struct Color {
		T r;
		T g;
		T b;

		Color() : r(0), g(0), b(0) {}
		Color(T _r, T _g, T _b) : r(_r), g(_g), b(_b) {}
	};

	using Rect = Geometry::Rectangle<double, Color<unsigned char>>;

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
	Rand randomGenerator;

	bool renderBorders;
	bool testPixels;

	std::vector<Rect> rects;
};

#endif /* EXAMPLERECTS_H_ */
