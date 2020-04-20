/*
 * Pixels.h
 *
 *  Created on: Apr 11, 2020
 *      Author: ans
 */

#ifndef PIXELS_H_
#define PIXELS_H_

#pragma once

#include <stdexcept>	// std::runtime_error

class Pixels {
public:
	Pixels();
	virtual ~Pixels();

	void map(int w, int h, unsigned char b, unsigned char * ptr);
	void fill(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	void set(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	void * get();
	void unmap();

	void allocate(int w, int h, unsigned char b);
	void deallocate();

	operator bool() const;

private:
	int width;
	int height;
	unsigned char bytes;
	unsigned char * pixels;

	bool allocated;
};

#endif /* PIXELS_H_ */
