/*
 * Example.h
 *
 *  Created on: Apr 18, 2020
 *      Author: ans
 */

#ifndef EXAMPLE_H_
#define EXAMPLE_H_

#pragma once

#include "Engine.h"

#include <cstdlib>		// EXIT_SUCCESS
#include <string>		// std::string

#define UNUSED(x) (void)(x)

class Example : Engine {
public:
	Example();
	virtual ~Example();

	int run(int argc, char * argv[]);

private:
	void onCreate() override;
	void onUpdate(double elapsedTime) override;

	unsigned short pixelSize;
};

#endif /* EXAMPLE_H_ */
