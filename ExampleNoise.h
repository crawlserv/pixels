/*
 * ExampleNoise.h
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

#ifndef EXAMPLENOISE_H_
#define EXAMPLENOISE_H_

#include "Engine.h"
#include "Rand.h"

#include <cstdlib>		// EXIT_SUCCESS
#include <string>		// std::string

class ExampleNoise : Engine {
public:
	ExampleNoise();
	virtual ~ExampleNoise();

	int run(int argc, char * argv[]);

private:
	void onCreate() override;
	void onUpdate(double elapsedTime) override;

	unsigned short pixelSize;

	Rand randGenerator;
};

#endif /* EXAMPLENOISE_H_ */
