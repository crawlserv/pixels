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
		  windowPointer(nullptr),
		  renderingMode(RENDERING_MODE_PBO),
		  pboId(0),
		  textureId(0),
		  width(0),
		  height(0),
		  bytes(4),
		  pixelWidth(0),
		  pixelHeight(0),
		  clearBuffer(false),
		  pixelSize(1),
		  halfPixelSize(0),
		  rendering(false),
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
	this->destroyRenderingTarget();

	// destroy window if necessary
	if(this->windowPointer) {
		glfwDestroyWindow(this->windowPointer);

		this->windowPointer = nullptr;
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
	this->windowPointer = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);

	if(!(this->windowPointer))
		throw std::runtime_error("glfwCreateWindow failed");

	this->title = title;

	// set additional window options
	glfwSetWindowUserPointer(this->windowPointer, this);
	glfwSetFramebufferSizeCallback(this->windowPointer, MainWindow::callbackFramebuffer);
	glfwSetKeyCallback(this->windowPointer, MainWindow::callbackKey);

	// get size of framebuffer
	glfwGetFramebufferSize(this->windowPointer, &(this->width), &(this->height));

	// make OpenGL context current and disable vertical synchronization
	glfwMakeContextCurrent(this->windowPointer);
	glfwSwapInterval(0);

	// set additional OpenGL options
	glDisable(GL_DITHER);
	glDisable(GL_MULTISAMPLE);
	glShadeModel(GL_FLAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glClearColor(0., 0., 0., 1.);

	// initialize rendering target and set projection
	this->initRenderingTarget();
	this->setProjection();

	// save starting time
	this->lastTime = glfwGetTime();
}

// tick in window loop to process window events, return whether window has been closed
bool MainWindow::update() {
	// check whether window has been closed
	if(glfwWindowShouldClose(this->windowPointer))
		return false;

	// check whether debugging string has been changed
	if(this->debugChanged) {
		if(this->debug.empty())
			glfwSetWindowTitle(this->windowPointer, this->title.c_str());
		else {
			std::string fullTitle(this->title);

			fullTitle += " [" + this->debug + "]";

			glfwSetWindowTitle(this->windowPointer, fullTitle.c_str());
		}

		this->debugChanged = false;
	}

	// poll for window events
	glfwPollEvents();

	// begin rendering to pixel buffer
	this->beginRendering();

	// update frame
	this->onUpdate(this->getElapsedTime());

	// end rendering to pixel buffer
	this->endRendering();

	// clear keys
	this->clearKeys();

	// flush the buffer
	glfwSwapBuffers(this->windowPointer);

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

// get the current rendering mode
MainWindow::RenderingMode MainWindow::getRenderingMode() const {
	return this->renderingMode;
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

// set the rendering mode
void MainWindow::setRenderingMode(RenderingMode mode) {
	// end rendering if necessary
	if(this->rendering)
		this->endRendering();

	// destroy old rendering target
	this->destroyRenderingTarget();

	this->renderingMode = mode;

	// initialize new rendering target
	this->initRenderingTarget();

	// start rendering if necessary
	if(this->rendering)
		this->beginRendering();
}

// set whether to clear the rendering target to black every frame
void MainWindow::setClearBuffer(bool clear) {
	this->clearBuffer = clear;
}

// set the actual size of one pixel
void MainWindow::setPixelSize(unsigned short size) {
	this->pixelSize = size;
	this->halfPixelSize = size / 2;

	this->setProjection();

	if(this->renderingMode == RENDERING_MODE_POINTS && this->rendering) {
		this->endRendering();

		glPointSize(this->pixelSize);

		this->beginRendering();
	}
}

// set a test for pixels before drawing them
void MainWindow::setPixelTest(const PixelTest& test) {
	this->pixelTest = test;

	if(this->pixelTest)
		this->pixelTest.init(this->width, this->height);
}

// write one pixel into the buffer / draw it onto the screen
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

				if(this->pixelTest.test(putX, putY)) {
					if(this->renderingMode == RENDERING_MODE_POINTS) {
						glColor4ub(r, g, b, a);
						glVertex2i(x * this->pixelSize + this->halfPixelSize, y * this->pixelSize + this->halfPixelSize);
					}
					else
						this->pixels.set(
								putX,
								putY,
								r,
								g,
								b,
								a
						);
				}
				else if(this->pixelTest.debugging) {
					if(this->renderingMode == RENDERING_MODE_POINTS) {
						glColor4ub(r, g, b, a);
						glVertex2i(x * this->pixelSize + this->halfPixelSize, y * this->pixelSize + this->halfPixelSize);
					}
					else
						this->pixels.set(
								putX,
								putY,
								255,
								0,
								0,
								255
						);
				}
			}
	else
		for(unsigned short relX = 0; relX < limitX; ++relX)
			for(unsigned short relY = 0; relY < limitY; ++relY) {
				if(this->renderingMode == RENDERING_MODE_POINTS) {
					glColor4ub(r, g, b, a);
					glVertex2i(x * this->pixelSize + this->halfPixelSize, y * this->pixelSize + this->halfPixelSize);
				}
				else
					this->pixels.set(
							offsetX + relX,
							offsetY + relY,
							r,
							g,
							b,
							a
					);
			}
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
	glOrtho(0, this->width, 0, this->height, 1, -1);

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

// initialize rendering target
void MainWindow::initRenderingTarget() {
	// destroy old rendering target if necessary
	this->destroyRenderingTarget();

	// initialize or reset pixel test
	if(this->pixelTest)
		this->pixelTest.init(this->width, this->height);

	switch(this->renderingMode) {
	case RENDERING_MODE_PBO:
		// generate pixel buffer object
		glGenBuffers(1, &(this->pboId));

		// check pixel buffer object
		if(this->pboId <= 0) {
			const auto errorCode = glGetError();

			switch(errorCode) {
			case GL_NO_ERROR:
				throw std::runtime_error(
						"Could not create pixel buffer"
				);

			default:
				throw std::runtime_error(
						"Could not create pixel buffer: "
						+ MainWindow::glErrorString(errorCode)
				);
			}
		}

		// reserve memory for pixel buffer object
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, this->pboId);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, this->width * this->height * this->bytes, nullptr, GL_STREAM_DRAW);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		// enable texturing
		glEnable(GL_TEXTURE_2D);

		// generate texture
		glGenTextures(1, &(this->textureId));

		// check texture
		if(this->textureId <= 0) {
			const auto errorCode = glGetError();

			switch(errorCode) {
			case GL_NO_ERROR:
				throw std::runtime_error(
						"Could not create texture"
				);

			default:
				throw std::runtime_error(
						"Could not create texture: "
						+ MainWindow::glErrorString(errorCode)
				);
			}
		}

		// set texture properties and reserve memory for texture
		glBindTexture(GL_TEXTURE_2D, this->textureId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glBindTexture(GL_TEXTURE_2D, 0);

		// clear pixel buffer object
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, this->pboId);

		this->pixels.map(
				this->width,
				this->height,
				this->bytes,
				static_cast<unsigned char *>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY))
		);

		if(!(this->pixels))
			throw std::runtime_error("Could not map memory of pixel buffer object");

		this->pixels.fill(0, 0, 0, 255);

		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

		this->pixels.unmap();

		break;

	case RENDERING_MODE_POINTS:
		// set pixel size
		glPointSize(this->pixelSize);

		// set blending (not supported yet)
		//glEnable(GL_BLEND);

		// clear both frame buffers (front and back)
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(this->windowPointer);
		glClear(GL_COLOR_BUFFER_BIT);

		break;

	case RENDERING_MODE_TEXTURE:
		// enable texturing
		glEnable(GL_TEXTURE_2D);

		// generate and bind texture
		glGenTextures(1, &(this->textureId));
		glBindTexture(GL_TEXTURE_2D, this->textureId);

		// set texture properties
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

		// allocate memory for rendering data
		this->pixels.allocate(this->width, this->height, this->bytes);

		break;
	}
}

// start rendering a single frame
void MainWindow::beginRendering() {
	// notify pixel test of coming frame
	if(this->pixelTest)
		this->pixelTest.frame();

	this->rendering = true;

	switch(this->renderingMode) {
	case RENDERING_MODE_PBO:
		// bind pixel buffer object
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, this->pboId);

		// map memory of pixel buffer object
		this->pixels.map(
				this->width,
				this->height,
				this->bytes,
				static_cast<unsigned char *>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY))
		);

		if(!(this->pixels))
			throw std::runtime_error("Could not map memory of pixel buffer object");

		// clear the pixel buffer object if necessary
		if(this->clearBuffer)
			this->pixels.fill(0, 0, 0, 255);

		break;

	case RENDERING_MODE_POINTS:
		// clear the OpenGL framebuffer if necessary
		if(this->clearBuffer)
			glClear(GL_COLOR_BUFFER_BIT);

		glBegin(GL_POINTS);

		break;

	case RENDERING_MODE_TEXTURE:
		// clear the texture if necessary
		if(this->clearBuffer)
			this->pixels.fill(0, 0, 0, 255);

		break;
	}
}

// render textured quad
void MainWindow::renderQuad() {
	glBegin(GL_QUADS);

	glTexCoord2f(0., 0.);
	glVertex2i(0, 0);

	glTexCoord2f(0., 1.);
	glVertex2i(0, this->height);

	glTexCoord2f(1., 1.);
	glVertex2i(this->width, this->height);

	glTexCoord2f(1., 0.);
	glVertex2i(this->width, 0);

	glEnd();
}

// finish up rendering a single frame
void MainWindow::endRendering() {
	switch(this->renderingMode) {
	case RENDERING_MODE_PBO:
		// unmap memory of pixel buffer object
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

		this->pixels.unmap();

		//glDrawPixels(this->width, this->height, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		// bind and update texture
		glBindTexture (GL_TEXTURE_2D, this->textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->pixels.get());

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		// render textured quad
		this->renderQuad();

		// unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);

		break;

	case RENDERING_MODE_POINTS:
		glEnd();

		break;

	case RENDERING_MODE_TEXTURE:
		// update texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->pixels.get());

		// render textured quad
		this->renderQuad();

		break;
	}

	this->rendering = false;
}

// destroy rendering target
void MainWindow::destroyRenderingTarget() {
	switch(this->renderingMode) {
	case RENDERING_MODE_PBO:
		if(this->textureId > 0) {
			glDeleteTextures(1, &(this->textureId));

			this->textureId = 0;
		}

		if(this->pboId > 0) {
			glDeleteBuffers(1, &(this->pboId));

			this->pboId = 0;
		}

		break;

	case RENDERING_MODE_POINTS:
		glDisable(GL_BLEND);

		break;

	case RENDERING_MODE_TEXTURE:
		glBindTexture(GL_TEXTURE_2D, 0);

		if(this->textureId > 0) {
			glDeleteTextures(1, &(this->textureId));

			this->textureId = 0;
		}

		this->pixels.deallocate();

		break;
	}
}

// in-class callback for framebuffer creation and changes
void MainWindow::onFramebuffer(int w, int h) {
	if(this->width == w && this->height == h)
		return;

	this->width = w;
	this->height = h;

	// initialize rendering target and set projection
	this->initRenderingTarget();
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

// get the last OpenGL error as a string
std::string MainWindow::glErrorString(GLenum errorCode) {
	switch(errorCode) {
	case GL_NO_ERROR:
		return "GL_NO_ERROR";

	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";

	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";

	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";

	case GL_STACK_OVERFLOW:
		return "GL_STACK_OVERFLOW";

	case GL_STACK_UNDERFLOW:
		return "GL_STACK_UNDERFLOW";

	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";

	case GL_TABLE_TOO_LARGE:
		return "GL_TABLE_TOO_LARGE";

	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "GL_INVALID_FRAMEBUFFER_OPERATION";

	default:
		return "UNKNOWN ERROR";
	}
}
