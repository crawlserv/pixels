/*
 * MainWindow.cpp
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

#include "MainWindow.h"

// constructor: set default values
MainWindow::MainWindow()
		: glfwInitialized(false),
		  window(nullptr),
		  pixelBuffer(0),
		  pixelBufferSize(0),
		  width(0),
		  height(0),
		  bytes(4),
		  pixelWidth(0),
		  pixelHeight(0),
		  clearBuffer(false),
		  pixelSize(1),
		  halfPixelSize(0),
		  lastTime(0.),
		  elapsedTime(0.),
		  fps(0.),
		  debugChanged(false) {
	for(int n = 0; n < GLFW_KEY_LAST; ++n) {
		keys[n].pressed = false;
		keys[n].held = false;
		keys[n].released = false;
		keys[n].repeated = false;
	}

	glfwSetErrorCallback(MainWindow::callbackError);
}

// destructor
MainWindow::~MainWindow() {
	// destroy pixel buffer if necessary
	this->clearPixelBuffer();

	// destroy window if necessary
	if(this->window) {
		glfwDestroyWindow(this->window);

		this->window = nullptr;
	}

	// terminate GLFW if necessary
	if(this->glfwInitialized) {
		glfwTerminate();

		this->glfwInitialized = false;
	}
}

// initialize window
void MainWindow::init(unsigned int w, unsigned int h, const std::string& title) {
	// initialize GLFW
	if(!(this->glfwInitialized) && !glfwInit())
		throw std::runtime_error("glfwInit failed");

	this->glfwInitialized = true;

	// create window
	this->window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);

	if(!(this->window))
		throw std::runtime_error("glfwCreateWindow failed");

	this->title = title;

	// set additional window options
	glfwSetWindowUserPointer(this->window, this);
	glfwSetFramebufferSizeCallback(this->window, MainWindow::callbackFramebuffer);
	glfwSetKeyCallback(this->window, MainWindow::callbackKey);

	// get size of framebuffer
	glfwGetFramebufferSize(this->window, &(this->width), &(this->height));

	// make OpenGL context current and disable vertical synchronization
	glfwMakeContextCurrent(this->window);
	glfwSwapInterval(0);

	// set additional OpenGL options
	glDisable(GL_DITHER);
	glDisable(GL_MULTISAMPLE);
	glShadeModel(GL_FLAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glClearColor(0., 0., 0., 1.);

	// initialize pixel buffer if necessary
	this->initPixelBuffer();

	// set projection
	this->setProjection();

	// save starting time
	this->lastTime = glfwGetTime();
}

// tick in window loop to process window events, return whether window has been closed
bool MainWindow::update() {
	// check whether window has been closed
	if(glfwWindowShouldClose(this->window))
		return false;

	// check whether debugging string has been changed
	if(this->debugChanged) {
		if(this->debug.empty())
			glfwSetWindowTitle(this->window, this->title.c_str());
		else {
			std::string fullTitle(this->title);

			fullTitle += " [" + this->debug + "]";

			glfwSetWindowTitle(this->window, fullTitle.c_str());
		}

		this->debugChanged = false;
	}

	// poll for window events
	glfwPollEvents();

	// begin rendering to pixel buffer
	this->beginPixelBuffer();

	// update frame
	this->onUpdate(this->getElapsedTime());

	// end rendering to pixel buffer
	this->endPixelBuffer();

	// clear keys
	this->clearKeys();

	// flush the buffer
	glfwSwapBuffers(this->window);

	// calculate the framerate
	double currentTime = glfwGetTime();

	this->elapsedTime = currentTime - this->lastTime;

	this->lastTime = currentTime;
	this->fps = 1. / this->elapsedTime;

	return true;
}

// get the width of the framebuffer in pixels
int MainWindow::getWidth() const {
	return this->pixelWidth;
}

// get the height of the framebuffer in pixels
int MainWindow::getHeight() const {
	return this->pixelHeight;
}


// get time since start in seconds
double MainWindow::getTime() const {
	return glfwGetTime();
}

// get elapsed time since last frame in seconds
double MainWindow::getElapsedTime() const {
	return this->elapsedTime;
}

// get framerate in frames per second
double MainWindow::getFPS() const {
	return this->fps;
}

// check whether the specified key has been pressed this frame
bool MainWindow::isKeyPressed(unsigned int code) const {
	if(code <= 0 || code > GLFW_KEY_LAST)
		throw std::runtime_error("isKeyPressed: illegal key code");

	return this->keys[code - 1].pressed;
}

// check whether the specified key has been held
bool MainWindow::isKeyHeld(unsigned int code) const {
	if(code <= 0 || code > GLFW_KEY_LAST)
		throw std::runtime_error("isKeyHeld: illegal key code");

	return this->keys[code - 1].held;
}

// check whether the specified key has been released this frame
bool MainWindow::isKeyReleased(unsigned int code) const {
	if(code <= 0 || code > GLFW_KEY_LAST)
		throw std::runtime_error("isKeyReleased: illegal key code");

	return this->keys[code - 1].released;
}

// check whether the specified key has been held long enough to be repeated
bool MainWindow::isKeyRepeated(unsigned int code) const {
	if(code <= 0 || code > GLFW_KEY_LAST)
		throw std::runtime_error("isKeyRepeated: illegal key code");

	return this->keys[code - 1].repeated;
}

// set whether to clear the buffer on every frame
void MainWindow::setClearBuffer(bool clear) {
	this->clearBuffer = clear;
}

// set the actual size of one pixel
void MainWindow::setPixelSize(unsigned short size) {
	this->pixelSize = size;
	this->halfPixelSize = size / 2;

	this->setProjection();
}

// set a test for pixels before drawing them
void MainWindow::setPixelTest(const PixelTest& test) {
	this->pixelTest = test;

	if(this->pixelTest)
		this->pixelTest.init(this->width, this->height);
}

// write one pixel into the buffer
void MainWindow::putPixel(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	const auto offsetX = x * this->pixelSize;
	const auto offsetY = y * this->pixelSize;
	int limitX = this->pixelSize;
	int limitY = this->pixelSize;

	if(static_cast<int>(offsetX + limitX) > this->width)
		limitX = this->width - offsetX;

	if(static_cast<int>(offsetY + limitY) > this->height)
		limitY = this->height - offsetY	;

	if(this->pixelTest)
		for(unsigned short relX = 0; relX < limitX; ++relX)
			for(unsigned short relY = 0; relY < limitY; ++relY) {
				const auto putX = offsetX + relX;
				const auto putY = offsetY + relY;

				if(this->pixelTest.test(putX, putY))
					this->pixels.set(
							putX,
							putY,
							r,
							g,
							b,
							a
					);
				else if(this->pixelTest.debugging)
					this->pixels.set(
							putX,
							putY,
							255,
							0,
							0,
							255
					);
			}
	else
		for(unsigned short relX = 0; relX < limitX; ++relX)
			for(unsigned short relY = 0; relY < limitY; ++relY)
				this->pixels.set(
						offsetX + relX,
						offsetY + relY,
						r,
						g,
						b,
						a
				);
}

// set callback function for updating the content
void MainWindow::setOnUpdate(UpdateFunction callBack) {
	this->onUpdate = callBack;
}

// set callback function for resize
void MainWindow::setOnResize(ResizeFunction callBack) {
	this->onResize = callBack;
}

// set the debug text to be shown in the window title bar (after "pixels")
void MainWindow::setDebugText(const std::string& text) {
	this->debug = text;
	this->debugChanged = true;
}

// set projection (use window coordinates)
void MainWindow::setProjection() {
	// calculate pixels
	if(this->width % this->pixelSize > 0)
		this->pixelWidth = this->width / this->pixelSize + 1;
	else
		this->pixelWidth = this->width / this->pixelSize;

	if(this->height % this->pixelSize > 0)
		this->pixelHeight = this->height / this->pixelSize + 1;
	else
		this->pixelHeight = this->height / this->pixelSize;

	// set viewport
	glViewport(0, 0, this->width, this->height);

	// set projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, this->width, this->height, 0, 1, -1);

	// set model
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// reset keys (only pressed and released)
void MainWindow::clearKeys() {
	for(int n = 0; n < GLFW_KEY_LAST; ++n) {
		if(this->keys[n].pressed)
			this->keys[n].pressed = false;

		if(this->keys[n].released)
			this->keys[n].released = false;

		if(this->keys[n].repeated)
			this->keys[n].repeated = false;
	}
}

// initialize (and bind) pixel buffer
void MainWindow::initPixelBuffer() {
	// delete old pixel buffer if necessary
	this->clearPixelBuffer();

	// initialize or reset pixel test
	if(this->pixelTest)
		this->pixelTest.init(this->width, this->height);

	// calculate new pixel buffer size
	this->pixelBufferSize = this->width * this->height * this->bytes;

	// generate pixel buffer
	glGenBuffers(1, &(this->pixelBuffer));

	// check pixel buffer
	if(!(this->pixelBuffer))
		throw std::runtime_error("Could not create pixel buffer");

	// reserve memory for pixel buffer
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, this->pixelBuffer);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, this->pixelBufferSize, nullptr, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

// bind pixel buffer
void MainWindow::beginPixelBuffer() {
	// notify pixel test of coming frame
	if(this->pixelTest)
		this->pixelTest.frame();

	// bind buffer
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, this->pixelBuffer);

	// map pixels
	this->pixels.map(
			this->width,
			this->height,
			this->bytes,
			static_cast<unsigned char *>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY))
	);

	if(!(this->pixels))
		throw std::runtime_error("Could not map to pixel buffer");

	// clear pixel buffer if necessary
	if(this->clearBuffer)
		this->pixels.fill(0, 0, 0, 255);
}

// unbind pixel buffer and copy it to GPU
void MainWindow::endPixelBuffer() {
	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

	this->pixels.unmap();

	glDrawPixels(this->width, this->height, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

// clear pixel buffer
void MainWindow::clearPixelBuffer() {
	if(this->pixelBuffer) {
		glDeleteBuffers(1, &(this->pixelBuffer));

		this->pixelBuffer = 0;
	}
}

// in-class callback for framebuffer creation and changes
void MainWindow::onFramebuffer(int w, int h) {
	if(this->width == w && this->height == h)
		return;

	this->width = w;
	this->height = h;

	this->initPixelBuffer();

	this->setProjection();

	if(this->onResize)
		this->onResize(w, h);
}

// in-class callback for key events
void MainWindow::onKey(int key, int action) {
	if(key > 0 && key <= GLFW_KEY_LAST) {
		switch(action) {
		case GLFW_PRESS:
			this->keys[key - 1].pressed = true;
			this->keys[key - 1].held = true;

			break;

		case GLFW_RELEASE:
			this->keys[key - 1].held = false;
			this->keys[key - 1].released = true;

			break;

		case GLFW_REPEAT:
			this->keys[key - 1].held = true;
			this->keys[key - 1].repeated = true;

			break;
		}
	}
}

// callback for GLFW errors
void MainWindow::callbackError(int error, const char * description) {
	throw std::runtime_error(
			"GLFW error #"
			+ std::to_string(error)
			+ ": "
			+ description
	);
}

// callback for framebuffer changes (use GLFW user pointer to jump into class)
void MainWindow::callbackFramebuffer(GLFWwindow * window, int width, int height) {
	static_cast<MainWindow *>(glfwGetWindowUserPointer(window))->onFramebuffer(width, height);
}

// callback for key event (use GLFW user pointer to jump into class)
void MainWindow::callbackKey(GLFWwindow * window, int key, int scancode, int action, int mods) {
	UNUSED(scancode);
	UNUSED(mods);
	static_cast<MainWindow *>(glfwGetWindowUserPointer(window))->onKey(key, action);
}
