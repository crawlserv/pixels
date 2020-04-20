/*
 * Pixels.cpp
 *
 *  Created on: Apr 11, 2020
 *      Author: ans
 */

#include "Pixels.h"

// constructor and destructor
Pixels::Pixels() : width(0), height(0), bytes(0), pixels(nullptr), allocated(false) {}
Pixels::~Pixels() {
	this->deallocate();

	this->pixels = nullptr;
}

// set pointer to the mapped pixels (ptr), as well as their total width (w), height (h) and bytes per pixel (b)
void Pixels::map(int w, int h, unsigned char b, unsigned char * ptr) {
	this->width = w;
	this->height = h;
	this->bytes = b;
	this->pixels = ptr;
}

// fill all the mapped pixel with one color (r, g, b, a)
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

// get the pointer to the pixels
void * Pixels::get() {
	return this->pixels;
}

// unset the pointer to the mapped pixels
void Pixels::unmap() {
	this->pixels = nullptr;
}

// allocate pixel buffer in system memory for the specified width (w), height(h) and bytes per pixel (b)
void Pixels::allocate(int w, int h, unsigned char b) {
	this->deallocate();

	this->width = w;
	this->height = h;
	this->bytes = b;
	this->pixels = new unsigned char[this->width * this->height * this->bytes];

	if(this->pixels) {
		this->allocated = true;

		// set values for empty texture
		for(int x = 0; x < this->width; ++x)
			for(int y = 0; y < this->height; ++y) {
				const auto offset = y * this->width * this->bytes + x * this->bytes;

				this->pixels[offset] = 0;
				this->pixels[offset + 1] = 0;
				this->pixels[offset + 2] = 0;
				this->pixels[offset + 3] = 255;
			}
	}
}

// deallocate pixel buffer if necessary
void Pixels::deallocate() {
	if(this->allocated) {
		delete[] this->pixels;

		this->pixels = nullptr;

		this->allocated = false;
	}
}

// check whether pixels have been mapped, i.e. a pointer to them has been set
Pixels::operator bool() const {
	return this->pixels != nullptr;
}
