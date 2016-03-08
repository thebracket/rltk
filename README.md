# rltk
*Roguelike Toolkit - Modern (C++14) SFML-based toolkit for creating roguelikes.*

It's very early days, but I hope that this can be a useful tool. Black Future was getting messy, and I needed to separate out some 
engine logic from game logic for my sanity; I realized that others might find the underlying engine code useful.

Right now, it's a very fast ASCII (code-page 480) terminal renderer compatible with fonts from libtcod and Dwarf Fortress.
Eventually, it will provide assistance with a number of game-related topics including path-finding, line-plotting,
and probably some of the entity-component-system I've been enjoying.

## Building from source

You need SFML for your platform, and cmake. Make a "build" folder, and use CMake to generate build files for your platform (I'll expand upon
this later, when this is a more useful library).

## Included Examples (with lots of comments!)

I'll write proper documentation as the library evolves; I don't really want to write up a lot of docs and have to revise them
heavily as things solidify. I'm doing my best to include examples evolving towards a real roguelike. Currently, these are:

* [Example 1](https://github.com/thebracket/rltk/blob/master/examples/ex1/main.cpp): demonstrates a Hello World with frame-rate.

* [Example 2](https://github.com/thebracket/rltk/blob/master/examples/ex2/main.cpp): a randomly moving yellow @ on a field of white dots.

## Example
The goal is to keep it simple from the user's point of view. The following code is enough to setup an ASCII terminal,
and display **Hello World** with a frame-rate displayed (around 100 FPS on my workstation):

```c++
#include "../../rltk/rltk.hpp"
#include <sstream>

using namespace rltk;

void tick(double duration_ms) {
	std::stringstream ss;
	ss << "Frame duration: " << duration_ms << " ms (" << (1000.0/duration_ms) << " FPS).";
	root_console->print(1,1,"Hello World", WHITE, BLACK);
	root_console->print(1,2,ss.str(), YELLOW, BLUE);
}

int main()
{
	init();
	register_bitmap_font_load("8x8", "terminal8x8", "../assets/terminal8x8.png", 8, 8);
	run(tick, "8x8");
  return 0;
}
```
