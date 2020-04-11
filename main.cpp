/*
 * main.cpp
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

//#include "ExampleNoise.h"
#include "ExampleRects.h"

int main(int argc, char * argv[]) {
	/*
	 * EXAMPLE 1: render random noise
	 *
	 * Press the UP and DOWN arrow keys to adjust the pixel size.
	 */
	//return ExampleNoise().run(argc, argv);

	/*
	 * EXAMPLE 2: render random rectangles
	 *
	 * Press ENTER to add a random rectangle (and perform clipping).
	 * Press ESC to clear all rectangles.
	 * Press 'B' to switch border rendering.
	 * Press the UP and DOWN arrow keys to adjust the pixel size.
	 */
	return ExampleRects().run(argc, argv);
}
