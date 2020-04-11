# pixels

A playground for pixel-by-pixel software rendering.

## ExampleNoise

Draw all the pixels with chaning random colors each frame.

```c++
// main.cpp

#include "ExampleNoise.h"

int main(int argc, char * argv[]) {
  return ExampleNoise().run(argc, argv);
}
```

* Press the UP and DOWN arrow keys to adjust the pixel size.

![ExampleNoise](screens/noise.png)

## ExampleRects

Draw random rectangles and clip them accordingly.

```c++
// main.cpp

#include "ExampleRects.h"

int main(int argc, char * argv[]) {
  return ExampleRects().run(argc, argv);
}
```

* Press ENTER to add a single random rectangle and perform clipping against the existing rectangles.
* Press ESC to clear all rectangles.
* Press 'B' to switch border rendering.
* Press the UP and DOWN arrow keys to adjust the pixel size.

![ExampleRects](screens/rects.png)

