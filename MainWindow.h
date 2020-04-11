/*
 * MainWindow.h
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <GLFW/glfw3.h>

#include <chrono>		// std::chrono
#include <functional>	// std::function, std::placeholders
#include <stdexcept>	// std::runtime_error
#include <string>		// std::string, std::to_string
#include <thread>		// std::this_thread

#include <iostream>

class MainWindow {
public:
	using resizeFunction = std::function<void(int, int)>;
	using updateFunction = std::function<void(double)>;

	MainWindow();
	virtual ~MainWindow();

	void init(unsigned int w, unsigned int h, const std::string& title);
	bool update(updateFunction onUpdate);

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
	void putPixel(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b);

	void setOnResize(resizeFunction callBack);
	void setDebugText(const std::string& text);

	MainWindow(MainWindow&) = delete;

private:
	void setProjection();
	void clearKeys();

	void onFramebuffer(int w, int h);
	void onKey(int key, int action);

	static void callbackError(int error, const char * description);
	static void callbackFramebuffer(GLFWwindow * window, int width, int height);
	static void callbackKey(GLFWwindow * window, int key, int scancode, int action, int mods);

	bool glfwInitialized;
	GLFWwindow * window;

	std::string title;
	int width;
	int height;
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

	resizeFunction onResize;
};

#endif /* MAINWINDOW_H_ */