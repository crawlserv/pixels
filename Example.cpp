/*
 * Example.cpp
 *
 *  Created on: Apr 18, 2020
 *      Author: ans
 */

#include "Example.h"

Example::Example() : pixelSize(2) {}

Example::~Example() {}

// run the application
int Example::run(int argc, char * argv[]) {
	UNUSED(argc);
	UNUSED(argv);

	const std::string name("null");

	constexpr int width = 800;
	constexpr int height = 600;

	//this->setClearBuffer(true);
	this->setPixelSize(this->pixelSize);

	this->createMainWindow(width, height, name);

	this->Engine::run();

	return EXIT_SUCCESS;
}

// create resources
void Example::onCreate() {}

// update frame
void Example::onUpdate(double elapsedTime) {
	UNUSED(elapsedTime);

	/*
	 * <put rendering code here>
	 */

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
