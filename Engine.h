/*
 * Engine.h
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

#ifndef ENGINE_H_
#define ENGINE_H_

#include <sstream>				// std::fixed, std::ostringstream
#include <string>				// std::string

#include "MainWindow.h"
#include "PixelTest.h"

class Engine {
public:
	Engine();
	virtual ~Engine();

	void setClearBuffer(bool clear);
	void setPixelSize(unsigned short size);
	void setPixelTest(const PixelTest& pixelTest);
	void disablePixelTest();
	void setDebugText(const std::string& string);

	void createMainWindow(int width, int height, const std::string& title);

	void run();

	// functions to overwrite
	virtual void onCreate() = 0;
	virtual void onUpdate(double elapsedTime) = 0;
	virtual void onDestroy() {};

protected:
	int getWindowWidth() const;
	int getWindowHeight() const;
	double getTime() const;

	void clip(int& x, int& y);
	void draw(int x, int y, unsigned char r, unsigned char g, unsigned char b);
	void fill(int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b);

	bool isKeyPressed(unsigned int code) const;
	bool isKeyHeld(unsigned int code) const;
	bool isKeyReleased(unsigned int code) const;
	bool isKeyRepeated(unsigned int code) const;

private:
	MainWindow window;
	double oldTime;
	std::string debug;
	bool debugChanged;
};

#endif /* ENGINE_H_ */
