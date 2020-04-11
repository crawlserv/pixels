/*
 * ExampleNoise.cpp
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

#include "ExampleNoise.h"

ExampleNoise::ExampleNoise() : pixelSize(2) {}

ExampleNoise::~ExampleNoise() {}

// run the application
int ExampleNoise::run(int argc, char * argv[]) {
	const std::string name("noise");
	const int width = 800;
	const int height = 600;

	this->setPixelSize(this->pixelSize);
	this->setDebugText(this->randGenerator.str());

	this->createMainWindow(width, height, name);

	this->Engine::run();

	return EXIT_SUCCESS;
}

// create resources
void ExampleNoise::onCreate() {}

// update frame
void ExampleNoise::onUpdate(double elapsedTime) {
	// render noise
	const int w = this->getWindowWidth();
	const int h = this->getWindowHeight();

	if(w > 0 && h > 0) {
		for(int x = 0; x < w; ++x) {
			for(int y = 0; y < h; ++y) {
				const unsigned char r = this->randGenerator.generateByte();
				const unsigned char g = this->randGenerator.generateByte();
				const unsigned char b = this->randGenerator.generateByte();

				this->draw(x, y, r, g, b);
			}
		}
	}

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

	const unsigned char oldRandAlgo = this->randGenerator.getAlgo();
	unsigned char newRandAlgo = oldRandAlgo;

	if(this->isKeyPressed(GLFW_KEY_SPACE))
		++newRandAlgo;

	if(this->isKeyRepeated(GLFW_KEY_SPACE))
		++newRandAlgo;

	newRandAlgo %= RAND_ALGOS ;

	if(newRandAlgo != oldRandAlgo) {
		this->randGenerator.setAlgo(static_cast<Rand::Algo>(newRandAlgo));

		this->setDebugText(this->randGenerator.str());
	}
}
