/*
 * Pixels.cpp
 *
 *  Created on: Apr 11, 2020
 *      Author: ans
 */

#include "Pixels.h"

#include <stdexcept>	// std::runtime_error

Pixels::Pixels() : width(0), height(0), bytes(0), pixels(nullptr) {}
Pixels::~Pixels() { this->pixels = nullptr; }

void Pixels::map(int w, int h, unsigned char b, unsigned char * ptr) {
	this->width = w;
	this->height = h;
	this->bytes = b;
	this->pixels = ptr;
}

void Pixels::fill(unsigned char r, unsigned char g, unsigned char b) {
	for(auto x = 0; x < this->width; ++x)
		for(auto y = 0; y < this->height; ++y) {
			const auto offset = y * this->width * this->bytes + x * this->bytes;

			this->pixels[offset] = r;
			this->pixels[offset + 1] = g;
			this->pixels[offset + 2] = b;
		}
}

void Pixels::set(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
	if(x > this->width - 1)
		throw std::runtime_error(std::to_string(x) + " > " + std::to_string(this->width - 1));

	const auto offset = y * this->width * this->bytes + x * this->bytes;

	this->pixels[offset] = r;
	this->pixels[offset + 1] = g;
	this->pixels[offset + 2] = b;
}

void Pixels::unmap() {
	this->pixels = nullptr;
}

Pixels::operator bool() const {
	return this->pixels != nullptr;
}

