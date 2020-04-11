/*
 * ExampleNoise.h
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

#ifndef EXAMPLENOISE_H_
#define EXAMPLENOISE_H_

#include "Engine.h"

#include <cstdlib>		// EXIT_SUCCESS
#include <random>		// std::mt19937, std::random_device, std::uniform_int_distribution

class ExampleNoise : Engine {
public:
	ExampleNoise();
	virtual ~ExampleNoise();

	int run(int argc, char * argv[]);

private:
	void onCreate() override;
	void onUpdate(double elapsedTime) override;

	unsigned short pixelSize;
	std::random_device rd;
	std::mt19937 mt;
	std::uniform_int_distribution<unsigned char> dist;
};

#endif /* EXAMPLENOISE_H_ */
