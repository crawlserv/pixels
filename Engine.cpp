/*
 * Engine.cpp
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

#include "Engine.h"

// constructor and destructor stubs
Engine::Engine() : oldTime(0.), debugChanged(false) {}
Engine::~Engine() {}

// set whether to clear the buffer every frame
void Engine::setClearBuffer(bool clear) {
	this->window.setClearBuffer(clear);
}

// set the actual size of one pixel
void Engine::setPixelSize(unsigned char size) {
	this->window.setPixelSize(size);
}

// enable pixel testing
void Engine::setPixelTest(const PixelTest& pixelTest) {
	this->window.setPixelTest(pixelTest);
}

// disable pixel testing
void Engine::disablePixelTest() {
	this->window.setPixelTest(PixelTest());
}

// set additional debugging information to be shown in the window title
void Engine::setDebugText(const std::string& string) {
	if(string != this->debug) {
		this->debug = string;
		this->debugChanged = true;
	}
}

// create the main window
void Engine::createMainWindow(int width, int height, const std::string& title) {
	this->window.init(width, height, title);
}

// run the engine
void Engine::run() {
	while(true) {
		// update window
		if(this->window.update(std::bind(&Engine::onUpdate, this, std::placeholders::_1))) {
			const double newTime = this->window.getTime();

			if(this->debugChanged || newTime - this->oldTime > 0.25) {
				// show framerate in title bar
				std::ostringstream oss;

				oss.precision(2);

				oss << std::fixed << this->window.getFPS() << "fps";

				if(!(this->debug.empty()))
					oss << ", " << this->debug;

				this->window.setDebugText(oss.str());

				this->oldTime = newTime;
				this->debugChanged = false;
			}
		}
		else
			break;
	}
}

int Engine::getWindowWidth() const {
	return this->window.getWidth();
}

int Engine::getWindowHeight() const {
	return this->window.getHeight();
}

double Engine::getTime() const {
	return this->window.getTime();
}

void Engine::clip(int& x, int& y) {
	if(x < 0)
		x = 0;
	else if(x >= this->window.getWidth())
		x = this->window.getWidth();

	if(y < 0)
		y = 0;
	else if(y >= this->window.getHeight())
		y = this->window.getHeight();
}

void Engine::draw(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
	this->window.putPixel(x, y, r, g, b);
}

void Engine::fill(int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b) {
	this->clip(x1, y1);
	this->clip(x2, y2);

	for(int x = x1; x < x2; ++x)
		for(int y = y1; y < y2; ++y)
			this->draw(x, y, r, g, b);
}

bool Engine::isKeyPressed(unsigned int code) const {
	return this->window.isKeyPressed(code);
}

bool Engine::isKeyHeld(unsigned int code) const {
	return this->window.isKeyHeld(code);
}

bool Engine::isKeyReleased(unsigned int code) const {
	return this->window.isKeyReleased(code);
}

bool Engine::isKeyRepeated(unsigned int code) const {
	return this->window.isKeyRepeated(code);
}
