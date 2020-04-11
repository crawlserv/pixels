/*
 * ExampleRects.cpp
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

#include "ExampleRects.h"

ExampleRects::ExampleRects()
		: pixelSize(2),
		  mt(this->rd()),
		  distReal(0.f, 1.f),
		  distInt(0, 255),
		  renderBorders(false) {}

ExampleRects::~ExampleRects() {}

// run the application
int ExampleRects::run(int argc, char * argv[]) {
	const std::string name("rects");
	const int width = 800;
	const int height = 600;

	this->setClearBuffer(true);
	this->setPixelSize(this->pixelSize);
	this->createMainWindow(width, height, name);

	this->Engine::run();

	return EXIT_SUCCESS;
}

// create resources
void ExampleRects::onCreate() {}

// update frame
void ExampleRects::onUpdate(double elapsedTime) {
	// render rectangles
	const int w = this->getWindowWidth();
	const int h = this->getWindowHeight();

	for(const auto& rect : this->rects) {
		const int absX1 = rect.x1 * w;
		const int absY1 = rect.y1 * h;
		const int absX2 = rect.x2 * w;
		const int absY2 = rect.y2 * h;

		if(this->renderBorders) {
			static const unsigned char borderR = 0;
			static const unsigned char borderG = 0;
			static const unsigned char borderB = 0;

			for(int x = absX1; x < absX2; ++x) {
				this->draw(x, absY1, borderR, borderG, borderB);
				this->draw(x, absY2 - 1, borderR, borderG, borderB);
			}

			for(int y = absY1; y < absY2; ++y) {
				this->draw(absX1, y, borderR, borderG, borderB);
				this->draw(absX2 - 1, y, borderR, borderG, borderB);
			}

			this->fill(absX1 + 1, absY1 + 1, absX2 - 1, absY2 - 1, rect.c.r, rect.c.g, rect.c.b);
		}
		else
			this->fill(absX1, absY1, absX2, absY2, rect.c.r, rect.c.g, rect.c.b);
	}

	// show number of rectangles
	this->setDebugText("n=" + std::to_string(this->rects.size()));

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

	if(this->isKeyPressed(GLFW_KEY_ENTER))
		this->add();

	if(this->isKeyRepeated(GLFW_KEY_ENTER))
		this->add();

	if(this->isKeyPressed(GLFW_KEY_ESCAPE))
		this->rects.clear();

	if(this->isKeyRepeated(GLFW_KEY_ESCAPE))
		this->rects.clear();

	if(this->isKeyPressed(GLFW_KEY_B))
		this->renderBorders = !(this->renderBorders);

	if(this->isKeyRepeated(GLFW_KEY_B))
		this->renderBorders = !(this->renderBorders);
}

// add one rectangle
void ExampleRects::add() {
	static const float minSize = 0.001f;

	Rect newRect;

	while(newRect.w() < minSize || newRect.h() < minSize) {
		newRect.x1 = this->distReal(this->mt);
		newRect.y1 = this->distReal(this->mt);
		newRect.x2 = this->distReal(this->mt);
		newRect.y2 = this->distReal(this->mt);
	}

	newRect.c.r = this->distInt(this->mt);
	newRect.c.g = this->distInt(this->mt);
	newRect.c.b = this->distInt(this->mt);

	if(newRect.x1 > newRect.x2)
		std::swap(newRect.x1, newRect.x2);

	if(newRect.y1 > newRect.y2)
		std::swap(newRect.y1, newRect.y2);

	Geometry::addAndSplit(this->rects, newRect, minSize);
}
