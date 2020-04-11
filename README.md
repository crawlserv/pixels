# pixels

A playground for pixel-by-pixel software rendering.

## ExampleNoise

Draw all the pixels with random colors changing each frame.

```c++
// (main.cpp)
#include "ExampleNoise.h"

int main(int argc, char * argv[]) {
  return ExampleNoise().run(argc, argv);
}
```

* Press the UP and DOWN arrow keys to adjust the 'pixel' size.

![ExampleNoise](screens/noise.png)

## ExampleRects

Draw random rectangles and clip them accordingly.

```c++
// (main.cpp)
#include "ExampleRects.h"

int main(int argc, char * argv[]) {
  return ExampleRects().run(argc, argv);
}
```

* Press ENTER to add a single random rectangle and perform clipping against the existing rectangles.
* Press ESC to clear all rectangles.
* Press SPACE to switch rendering the borders of the rectangles.
* Press the UP and DOWN arrow keys to adjust the 'pixel' size.

![ExampleRects](screens/rects.png)

## Requirements

On Linux, the following libraries or their substitutes need to be installed and linked against:

* [OpenGL](https://www.opengl.org/) (`libGL`)
* [GLFW](https://www.glfw.org/) (`libglfw3`)
* [X11](https://www.x.org/) (`libX11`)
* [POSIX Threads](https://en.wikipedia.org/wiki/POSIX_Threads) (`libpthread`)
* `libdl`

Tested with `g++ (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0`.
