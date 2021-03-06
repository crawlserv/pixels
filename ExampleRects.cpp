/*
 * ExampleRects.cpp
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

#include "ExampleRects.h"

ExampleRects::ExampleRects()
		: pixelTestW(0),
		  pixelSize(2),
		  randomGenerator(Rand::RAND_ALGO_LEHMER32),
		  renderBorders(false),
		  testPixels(false) {}

ExampleRects::~ExampleRects() {}

// run the application
int ExampleRects::run(int argc, char * argv[]) {
	UNUSED(argc);
	UNUSED(argv);

	const std::string name("rects");

	constexpr int width = 800;
	constexpr int height = 600;

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
	UNUSED(elapsedTime);

	// render rectangles
	const int w = this->getWindowWidth();
	const int h = this->getWindowHeight();

	if(this->testPixels) // draw backwards if pixel testing is enabled
		for(auto it = this->rects.rbegin(); it != this->rects.rend(); ++it)
			this->render(w, h, *it);
	else
		for(const auto& rect : this->rects)
			this->render(w, h, rect);

	// show number of rectangles and whether pixel testing is enabled
	std::string debugStr("n=");

	debugStr += std::to_string(this->rects.size());

	if(this->testPixels)
		debugStr += ", testing pixels";

	this->setDebugText(debugStr);

	// handle ENTER key for adding a rectangle
	if(this->isKeyPressed(GLFW_KEY_ENTER))
		this->add();

	if(this->isKeyRepeated(GLFW_KEY_ENTER))
		this->add();

	// handle ESCAPE key for removing all rectangles
	if(this->isKeyPressed(GLFW_KEY_ESCAPE))
		this->rects.clear();

	if(this->isKeyRepeated(GLFW_KEY_ESCAPE))
		this->rects.clear();

	// handle SPACE key for toggling drawing the borders of the rectangles
	if(this->isKeyPressed(GLFW_KEY_SPACE))
		this->renderBorders = !(this->renderBorders);

	// handle TAB key for toggling pixel testing
	if(this->isKeyPressed(GLFW_KEY_TAB)) {
		this->testPixels = !(this->testPixels);

		if(this->testPixels) {
			PixelTest pixelTest;

			pixelTest.debugging = true;
			pixelTest.init = std::bind(&ExampleRects::pixelTestInit, this, std::placeholders::_1, std::placeholders::_2);
			pixelTest.frame = std::bind(&ExampleRects::pixelTestFrame, this);
			pixelTest.test = std::bind(&ExampleRects::pixelTestTest, this, std::placeholders::_1, std::placeholders::_2);

			this->setPixelTest(pixelTest);
		}
		else
			this->disablePixelTest();
	}

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

// add one rectangle
void ExampleRects::add() {
	constexpr double minSize = 0.001f;

	Rect newRect;

	while(newRect.w() < minSize || newRect.h() < minSize) {
		newRect.x1 = this->randomGenerator.generateReal();
		newRect.y1 = this->randomGenerator.generateReal();
		newRect.x2 = this->randomGenerator.generateReal();
		newRect.y2 = this->randomGenerator.generateReal();
	}

	newRect.c.r = this->randomGenerator.generateByte();
	newRect.c.g = this->randomGenerator.generateByte();
	newRect.c.b = this->randomGenerator.generateByte();

	if(newRect.x1 > newRect.x2)
		std::swap(newRect.x1, newRect.x2);

	if(newRect.y1 > newRect.y2)
		std::swap(newRect.y1, newRect.y2);

	Geometry::addAndSplit(this->rects, newRect, minSize);
}

// render one rectangle
void ExampleRects::render(int w, int h, const Rect& rect) {
	const int absX1 = static_cast<int>(std::lround(rect.x1 * w));
	const int absY1 = static_cast<int>(std::lround(rect.y1 * h));
	const int absX2 = static_cast<int>(std::lround(rect.x2 * w));
	const int absY2 = static_cast<int>(std::lround(rect.y2 * h));

	if(this->renderBorders) {
		constexpr unsigned char borderR = 0;
		constexpr unsigned char borderG = 0;
		constexpr unsigned char borderB = 0;

		for(int x = absX1; x < absX2; ++x) {
			if(absY1 < h)
				this->draw(x, absY1, borderR, borderG, borderB);

			if(absY2 > 0)
				this->draw(x, absY2 - 1, borderR, borderG, borderB);
		}

		for(int y = absY1; y < absY2; ++y) {
			if(absX1 < w)
				this->draw(absX1, y, borderR, borderG, borderB);

			if(absX2 > 0)
				this->draw(absX2 - 1, y, borderR, borderG, borderB);
		}

		if(absX2 > 0 && absY2 > 0)
			this->fill(absX1 + 1, absY1 + 1, absX2 - 1, absY2 - 1, rect.c.r, rect.c.g, rect.c.b);
		else if(absX2 > 0)
			this->fill(absX1 + 1, absY1 + 1, absX2 - 1, 0, rect.c.r, rect.c.g, rect.c.b);
		else if(absY2 > 0)
			this->fill(absX1 + 1, absY1 + 1, 0, absY2 - 1, rect.c.r, rect.c.g, rect.c.b);
	}
	else
		this->fill(absX1, absY1, absX2, absY2, rect.c.r, rect.c.g, rect.c.b);
}

// pixel test initialization
void ExampleRects::pixelTestInit(unsigned int w, unsigned int h) {
	this->pixelTest.assign(w * h, false);

	this->pixelTestW = w;
}

// pixel test frame
void ExampleRects::pixelTestFrame() {
	this->pixelTest.assign(this->pixelTest.size(), false);
}

// pixel test
bool ExampleRects::pixelTestTest(unsigned int x, unsigned int y) {
	const auto i = y * this->pixelTestW + x;

	if(this->pixelTest.at(i))
		return false;

	this->pixelTest.at(i) = true;

	return true;
}
