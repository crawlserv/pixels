/*
 * MainWindow.h
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#define GL_GLEXT_PROTOTYPES

#include <GLFW/glfw3.h>

#include <chrono>		// std::chrono
#include <cstdlib>		// std::size_t
#include <functional>	// std::function, std::placeholders
#include <stdexcept>	// std::runtime_error
#include <string>		// std::string, std::to_string

#include "Pixels.h"
#include "PixelTest.h"

#define UNUSED(x) (void)(x)

class MainWindow {
public:
	using ResizeFunction = std::function<void(int, int)>;
	using UpdateFunction = std::function<void(double)>;

	MainWindow();
	virtual ~MainWindow();

	void init(unsigned int w, unsigned int h, const std::string& title);
	bool update(UpdateFunction onUpdate);

	int getWidth() const;
	int getHeight() const;
	double getTime() const;
	double getElapsedTime() const;
	double getFPS() const;

	bool isKeyPressed(unsigned int code) const;
	bool isKeyHeld(unsigned int code) const;
	bool isKeyReleased(unsigned int code) const;
	bool isKeyRepeated(unsigned int code) const;

	void setClearBuffer(bool clear);
	void setPixelSize(unsigned short size);
	void setPixelTest(const PixelTest& test);
	void putPixel(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b);

	void setOnResize(ResizeFunction callBack);
	void setDebugText(const std::string& text);

	MainWindow(MainWindow&) = delete;

private:
	void setProjection();
	void clearKeys();

	void initPixelBuffer();
	void beginPixelBuffer();
	void endPixelBuffer();
	void clearPixelBuffer();

	void onFramebuffer(int w, int h);
	void onKey(int key, int action);

	static void callbackError(int error, const char * description);
	static void callbackFramebuffer(GLFWwindow * window, int width, int height);
	static void callbackKey(GLFWwindow * window, int key, int scancode, int action, int mods);

	bool glfwInitialized;
	GLFWwindow * window;
	unsigned int pixelBuffer;
	std::size_t pixelBufferSize;
	Pixels pixels;

	std::string title;
	int width;
	int height;
	unsigned char bytes;
	int pixelWidth;
	int pixelHeight;
	bool clearBuffer;
	unsigned short pixelSize;
	unsigned short halfPixelSize;

	double lastTime;
	double elapsedTime;
	double fps;
	std::string debug;
	bool debugChanged;

	struct {
		bool pressed;
		bool held;
		bool released;
		bool repeated;
	} keys[GLFW_KEY_LAST];

	ResizeFunction onResize;
	PixelTest pixelTest;
};

#endif /* MAINWINDOW_H_ */
