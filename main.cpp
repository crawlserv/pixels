/*
 * main.cpp
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

//#include "ExampleNoise.h"
//#include "ExampleRects.h"
#include "ExampleSound.h"

int main(int argc, char * argv[]) {
	/*
	 * EXAMPLE 1: render random noise
	 *
	 * Press SPACE to change the algorithm for creating pseudo-random numbers.
	 * Press the UP and DOWN arrow keys to adjust the 'pixel' size.
	 */
	//return ExampleNoise().run(argc, argv);

	/*
	 * EXAMPLE 2: render random rectangles
	 *
	 * Press ENTER to add a single random rectangle and perform clipping against the existing rectangles.
	 * Press ESC to clear all rectangles.
	 * Press SPACE to switch rendering the borders of the rectangles.
	 * Press TAB to test for and debug overlapping pixels.
	 * Press the UP and DOWN arrow keys to adjust the 'pixel' size.
	 */
	//return ExampleRects().run(argc, argv);

	/*
	 * EXAMPLE 3: render and output random sound waves.
	 *
	 * Press ENTER to add a random sine sound wave.
	 * Press SPACE to add a random square sound wave.
	 * Press TAB to add a random triangle sound wave.
	 * Press the UP and DOWN arrow keys to adjust the 'pixel' size.
	 * Press the RIGHT and LEFT arrow keys to adjust the resolution of the sound wave.
	 */
	return ExampleSound().run(argc, argv);
}
