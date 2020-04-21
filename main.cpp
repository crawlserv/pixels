/*
 * main.cpp
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

#include "Example.h"
#include "ExampleNoise.h"
#include "ExampleRects.h"
#include "ExampleSound.h"

int main(int argc, char * argv[]) {
	/*
	 * EXAMPLE 0: Load the framework but draw nothing.
	 *
	 * Press the UP and DOWN arrow keys to adjust the 'pixel' size.
	 * Press the F10-F12 keys to change the rendering mode (F10=Pixel Buffer , F11=OpenGL Points, F12=Texture).
	 */
	return Example().run(argc, argv);

	/*
	 * EXAMPLE 1: Draw every pixel with a random color changing each frame.
	 *
	 * Press SPACE to change the algorithm for creating pseudo-random numbers.
	 * Press the UP and DOWN arrow keys to adjust the 'pixel' size.
	 * Press the F10-F12 keys to change the rendering mode.
	 */
	//return ExampleNoise().run(argc, argv);

	/*
	 * EXAMPLE 2: Draw random rectangles and clip them accordingly.
	 *
	 * Press ENTER to add a single random rectangle and perform clipping against the existing rectangles.
	 * Press ESC to clear all rectangles.
	 * Press SPACE to switch rendering the borders of the rectangles.
	 * Press TAB to test for and debug overlapping pixels.
	 * Press the UP and DOWN arrow keys to adjust the 'pixel' size.
	 * Press the F10-F12 keys to change the rendering mode (F10=Pixel Buffer, F11=OpenGL Points, F12=Texture).
	 */
	//return ExampleRects().run(argc, argv);

	/*
	 * EXAMPLE 3: Play and draw semi-random sound waves.
	 *
	 * Press ENTER to add a semi-random sine sound wave.
	 * Press SPACE to add a semi-random square sound wave.
	 * Press TAB to add a semi-random triangle sound wave.
	 * Press BACKSPACE to add a semi-random sawtooth sound wave.
	 * Press N to add some semi-random noise.
	 * Press ESC to clear all sound waves.
	 * Press the RIGHT and LEFT arrow keys to adjust the resolution of the rendered sound wave.
	 * Press the UP and DOWN arrow keys to adjust the 'pixel' size.
	 * Press the F10-F12 keys to change the rendering mode (F10=Pixel Buffer, F11=OpenGL Points, F12=Texture).
	 */
	//return ExampleSound().run(argc, argv);
}
