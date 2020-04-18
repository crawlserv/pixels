/*
 * Pixels.cpp
 *
 *  Created on: Apr 11, 2020
 *      Author: ans
 */

#include "Pixels.h"

#include <stdexcept>	// std::runtime_error

// constructor and destructor
Pixels::Pixels() : width(0), height(0), bytes(0), pixels(nullptr) {}
Pixels::~Pixels() { this->pixels = nullptr; }

// set pointer to the mapped pixels (ptr), as well as their total width (w), height (h) and bytes per pixel (b)
void Pixels::map(int w, int h, unsigned char b, unsigned char * ptr) {
	this->width = w;
	this->height = h;
	this->bytes = b;
	this->pixels = ptr;
}

// fill all the mapped pixel with one color (r, g, b)
void Pixels::fill(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	for(auto x = 0; x < this->width; ++x)
		for(auto y = 0; y < this->height; ++y) {
			const auto offset = y * this->width * this->bytes + x * this->bytes;

			this->pixels[offset] = r;
			this->pixels[offset + 1] = g;
			this->pixels[offset + 2] = b;
			this->pixels[offset + 3] = a;
		}
}

// set one of the mapped pixel (x, y) to the specified color (r, g, b, a)
void Pixels::set(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	const auto offset = y * this->width * this->bytes + x * this->bytes;

	this->pixels[offset] = r;
	this->pixels[offset + 1] = g;
	this->pixels[offset + 2] = b;
	this->pixels[offset + 3] = a;
}

// unset the pointer to the mapped pixels
void Pixels::unmap() {
	this->pixels = nullptr;
}

// check whether pixels have been mapped, i.e. a pointer to them has been set
Pixels::operator bool() const {
	return this->pixels != nullptr;
}

