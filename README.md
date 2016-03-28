# rltk : Roguelike Toolkit - Modern (C++14) SFML-based toolkit for creating roguelikes.

It's very early days, but I hope that this can be a useful tool. Black Future was getting messy, and I needed to separate out some 
engine logic from game logic for my sanity; I realized that others might find the underlying engine code useful.

Right now, it's a very fast ASCII (code-page 437) terminal renderer compatible with fonts from libtcod and Dwarf Fortress.
Eventually, it will provide assistance with a number of game-related topics including path-finding, line-plotting,
and probably some of the entity-component-system I've been enjoying.

## Building from source

You need SFML for your platform, Boost, and cmake. Make a "build" folder, and use CMake to generate build files for your platform (I'll expand upon this later, when this is a more useful library).

## Included Examples (with lots of comments!)

I'll write proper documentation as the library evolves; I don't really want to write up a lot of docs and have to revise them
heavily as things solidify. I'm doing my best to include examples evolving towards a real roguelike. Currently, these are:

### Example 1: Hello World
![Hello World](https://raw.githubusercontent.com/thebracket/rltk/master/tutorial_images/example1.png "Hello World")

[Example 1](https://github.com/thebracket/rltk/blob/master/examples/ex1/main.cpp): demonstrates a Hello World with frame-rate.

### Example 2: Randomly moving @
![Randomly Moving @](https://raw.githubusercontent.com/thebracket/rltk/master/tutorial_images/example2.gif "Randomly Moving @")

[Example 2](https://github.com/thebracket/rltk/blob/master/examples/ex2/main.cpp): a randomly moving yellow @ on a field of white dots.

### Example 3: Bresenham's Line Pathing
![Bresenham Line Pathing](https://raw.githubusercontent.com/thebracket/rltk/master/tutorial_images/example3.gif "Bresenham Line Pathing")

[Example 3](https://github.com/thebracket/rltk/blob/master/examples/ex3/main.cpp): our @ dude again, this time using Bresenham's line to find his way. It also renders additional glyphs, as Mr @ finds his way to his destination.

### Example 4: A-Star Pathing
![A Star Pathing](https://raw.githubusercontent.com/thebracket/rltk/master/tutorial_images/example4.gif "A Star Pathing")

[Example 4](https://github.com/thebracket/rltk/blob/master/examples/ex4/main.cpp): our @ dude again, now on a defined map with obstacles and using A* to find his way to the heart. This demonstrates using templates to specialize map support - we won't force you to use a specific map representation!

### Example 5: Mouse Controlled Path-Finding
![Mouse Driven A Star Pathing](https://raw.githubusercontent.com/thebracket/rltk/master/tutorial_images/example5.gif "Mouse Driven A Star Pathing")

[Example 5](https://github.com/thebracket/rltk/blob/master/examples/ex5/main.cpp): our @ dude again, using A* pathing to find his way to the mouse
cursor. Click and he goes there.

### Example 6: Visibility
![Visibility](https://raw.githubusercontent.com/thebracket/rltk/master/tutorial_images/example6.gif "Visibility")

[Example 6](https://github.com/thebracket/rltk/blob/master/examples/ex6/main.cpp): Example 5, but now we have true visibility plotted as you wander.

### Example 7: Multi-font ASCII Layout
![ComplexGui](https://raw.githubusercontent.com/thebracket/rltk/master/tutorial_images/example7.png "Complex GUI")

[Example 7](https://github.com/thebracket/rltk/blob/master/examples/ex7/main.cpp): A complex GUI with multiple fonts and layers, dynamically resizable.

### Example 8: Owner draw, and retained-mode GUI elements
![RetainedMode](https://raw.githubusercontent.com/thebracket/rltk/master/tutorial_images/example8.gif "Retained Mode")

[Example 8](https://github.com/thebracket/rltk/blob/master/examples/ex8/main.cpp): Demonstrates an "owner draw" panel (with SFML render target callback), drawing a background image. Some ASCII consoles are spawned, and one is populated with a mouse-over, a checkbox, radio-button set, a list-box and some status bars. The other panel displays the results.

## Example
The goal is to keep it simple from the user's point of view. The following code is enough to setup an ASCII terminal,
and display **Hello World** with a frame-rate displayed (around 100 FPS on my workstation):

```c++
#include "../../rltk/rltk.hpp"
#include <sstream>

using namespace rltk;
using namespace rltk::colors;

void tick(double duration_ms) {
	std::stringstream ss;
	ss << "Frame duration: " << duration_ms << " ms (" << (1000.0/duration_ms) << " FPS).";
	console->print(1, 1, "Hello World", WHITE, BLACK);
	console->print(1, 2, ss.str(), YELLOW, BLUE);
}

int main()
{
	init(config_simple_px("../assets"));
	run(tick);
  return 0;
}
```
