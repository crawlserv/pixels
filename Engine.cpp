/*
 * Engine.cpp
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

#include "Engine.h"

// constructor and destructor stubs
Engine::Engine() {}
Engine::~Engine() {}

// set whether to clear the buffer every frame
void Engine::setClearBuffer(bool clear) {
	this->window.setClearBuffer(clear);
}

// set the actual size of one pixel
void Engine::setPixelSize(unsigned char size) {
	this->window.setPixelSize(size);
}

// set additional debugging information to be shown in the window title
void Engine::setDebugText(const std::string& string) {
	this->debug = string;
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
			// show framerate in title bar
			std::ostringstream oss;

			oss.precision(2);

			oss << std::fixed << this->window.getFPS() << "fps";

			if(!(this->debug.empty()))
				oss << ", " << this->debug;

			this->window.setDebugText(oss.str());
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
