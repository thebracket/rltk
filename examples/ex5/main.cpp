#include <iostream>
/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Example 5: Extend example 4 to use the mouse to guide player movement.
 */

// You need to include the RLTK header
#include "../../rltk/rltk.hpp"

// We're using a vector to represent the map
#include <vector>

// We're also going to be using a shared_ptr to a map. Why shared? Because the library
// hands it off to you and it's up to you to use it; this provides some safety that it
// will be disposed when you are done with it.
#include <memory>

// For convenience, import the whole rltk namespace. You may not want to do this
// in larger projects, to avoid naming collisions.
using namespace rltk;
using namespace rltk::colors;

// A default-defined random number generator. You can specify a seed to get
// the same results each time, but for now we're keeping it simple.
random_number_generator rng;

// For now, we always want our "dude" to be a yellow @ - so he's constexpr
const vchar dude{'@', YELLOW, BLACK};
// We're also going to render our destination as a pink heart. Aww.
const vchar destination_glyph{3, MAGENTA, BLACK};
// We now need to represent walls and floors, too
const vchar wall_tile{'#', WHITE, BLACK};
const vchar floor_tile{'.', GREY, BLACK}; // Note that "floor" is taken as a name in C++!

// Now we define a structure to represent a location. In this case, it's a simple
// x/y coordinate.
struct location_t {
	int x=-1; // I like to set uninitialized values to something invalid for help with debugging
	int y=-1;

	// For convenience, we're overriding the quality operator. This gives a very
	// quick and natural looking way to say "are these locations the same?"
	bool operator==(location_t &rhs) { return (x==rhs.x && y==rhs.y); }

	location_t() {}
	location_t(const int X, const int Y) : x(X), y(Y) {}
};

// Now we define our basic map. Why a struct? Because a struct is just a class with
// everything public in it!
struct map_t {
	map_t(const int &w, const int &h) : width(w), height(h) {
		// Resize the vector to hold the whole map; this way it won't reallocate
		walkable.resize(w*h);

		// Set the entire map to walkable
		std::fill(walkable.begin(), walkable.end(), true);

		// We want the perimeter to be solid
		for (int x=0; x<width; ++x) { 
			walkable[at(x,0)]=false;
			walkable[at(x,height-1)]=false;
		}
		for (int y=0; y<height; ++y) {
			walkable[at(0,y)] = false;
			walkable[at(width-1,y)] = false;
		}

		// Every tile other than 10,10 (starting) has a 33% chance of being solid. We've
		// made it more likely to have obstacles, since we're no longer relying on the RNG
		// to find our way.	
		for (int y=1; y<height-2; ++y) {
			for (int x=1; x<width-2; ++x) {
				if (rng.roll_dice(1,3)==1) walkable[at(x,y)] = false;
			}
		}
		walkable[at(10,10)] = true;
	}

	// Calculate the vector offset of a grid location
	inline int at(const int &x, const int &y) { return (y*width)+x; }

	// The width and height of the map
	const int width, height;

	// The actual walkable storage vector
	std::vector<bool> walkable;
};

// The A* library returns a navigation path with a template specialization to our location_t.
// Store the path here. Normally, you'd use "auto" for this type, it is a lot less typing!
std::shared_ptr<navigation_path<location_t>> path;

// We're using 1024x768, with 8 pixel wide chars. That gives a console grid of
// 128 x 96. We'll go with that for the map, even though in reality the screen
// might change. Worrying about that is for a future example!
constexpr int MAP_WIDTH = 128;
constexpr int MAP_HEIGHT = 96;
map_t map(MAP_WIDTH, MAP_HEIGHT);

// Instead of raw ints, we'll use the location structure to represent where our
// dude is. Using C++14 initialization, it's nice and clean.
location_t dude_position {10,10};

// We'll also use a location_t to represent the intended destination.
location_t destination {10,10};

// The A* library also requires a helper class to understand your map format.
struct navigator {
	// This lets you define a distance heuristic. Manhattan distance works really well, but
	// for now we'll just use a simple euclidian distance squared.
	// The geometry system defines one for us.
	static float get_distance_estimate(location_t &pos, location_t &goal) {
		float d = distance2d_squared(pos.x, pos.y, goal.x, goal.y);
		return d;
	}

	// Heuristic to determine if we've reached our destination? In some cases, you'd not want
	// this to be a simple comparison with the goal - for example, if you just want to be
	// adjacent to (or even a preferred distance from) the goal. In this case, 
	// we're trying to get to the goal rather than near it.
	static bool is_goal(location_t &pos, location_t &goal) {
		return pos == goal;
	}

	// This is where we calculate where you can go from a given tile. In this case, we check
	// all 8 directions, and if the destination is walkable return it as an option.
	static bool get_successors(location_t pos, std::vector<location_t> &successors) {
		//std::cout << pos.x << "/" << pos.y << "\n";

		if (map.walkable[map.at(pos.x-1, pos.y-1)]) successors.push_back(location_t(pos.x-1, pos.y-1));
		if (map.walkable[map.at(pos.x, pos.y-1)]) successors.push_back(location_t(pos.x, pos.y-1));
		if (map.walkable[map.at(pos.x+1, pos.y-1)]) successors.push_back(location_t(pos.x+1, pos.y-1));

		if (map.walkable[map.at(pos.x-1, pos.y)]) successors.push_back(location_t(pos.x-1, pos.y));
		if (map.walkable[map.at(pos.x+1, pos.y)]) successors.push_back(location_t(pos.x+1, pos.y));

		if (map.walkable[map.at(pos.x-1, pos.y+1)]) successors.push_back(location_t(pos.x-1, pos.y+1));
		if (map.walkable[map.at(pos.x, pos.y+1)]) successors.push_back(location_t(pos.x, pos.y+1));
		if (map.walkable[map.at(pos.x+1, pos.y+1)]) successors.push_back(location_t(pos.x+1, pos.y+1));
		return true;
	}

	// This function lets you set a cost on a tile transition. For now, we'll always use a cost of 1.0.
	static float get_cost(location_t &position, location_t &successor) {
		return 1.0f;
	}

	// This is a simple comparison to determine if two locations are the same. It just passes
	// through to the location_t's equality operator in this instance (we didn't do that automatically)
	// because there are times you might want to behave differently.
	static bool is_same_state(location_t &lhs, location_t &rhs) {
		return lhs == rhs;
	}
};

// Lets go really fast!
constexpr double tick_duration = 1.0;
double tick_time = 0.0;

// Tick is called every frame. The parameter specifies how many ms have elapsed
// since the last time it was called.
void tick(double duration_ms) {
	// Iterate over the whole map, rendering as appropriate
	for (int y=0; y<MAP_HEIGHT; ++y) {
		for (int x=0; x<MAP_WIDTH; ++x) {
			if (map.walkable[map.at(x,y)]) {
				console->set_char(console->at(x,y), floor_tile);
			} else {
				console->set_char(console->at(x,y), wall_tile);
			}
		}
	}

	// Increase the tick time by the frame duration. If it has exceeded
	// the tick duration, then we move the @.
	tick_time += duration_ms;
	if (tick_time > tick_duration) {
		// Are we there yet?
		if (dude_position == destination) {
			// Now we poll the mouse to determine where we want to go
			int mouse_x, mouse_y;
			std::tie(mouse_x, mouse_y) = get_mouse_position();
			const int terminal_x = mouse_x / 8;
			const int terminal_y = mouse_y / 8;

			const bool walkable = map.walkable[map.at(terminal_x, terminal_y)]; 
			if (walkable && get_mouse_button_state(rltk::button::LEFT)) {
				destination.x = terminal_x;
				destination.y = terminal_y;

				// Now determine how to get there
				if (path) path.reset();
				path = find_path<location_t, navigator>(dude_position, destination);
				if (!path->success) {
					destination = dude_position;
					std::cout << "RESET: THIS ISN'T MEANT TO HAPPEN!\n";
				}
			} else if (walkable) {
				if (path) path.reset();
				path = find_path<location_t, navigator>(dude_position, location_t{terminal_x, terminal_y});
			}
		} else {
			// Follow the breadcrumbs!
			if (path) {
				location_t next_step = path->steps.front();
				dude_position.x = next_step.x;
				dude_position.y = next_step.y;
				path->steps.pop_front();
			}
		}

		// Important: we clear the tick count after the update.
		tick_time = 0.0;
	}

	// Render our planned path. We're using auto and a range-for to avoid typing all
	// the iterator stuff
	if (path) {
		// We're going to show off a bit and "lerp" the color along the path; the red
		// lightens as it approaches the destination. This is a preview of some of the
		// color functions.
		const float n_steps = static_cast<float>(path->steps.size());
		float i = 0;
		for (auto step : path->steps) {
			const float lerp_amount = i / n_steps;
			vchar highlight;
			if (dude_position == destination) {
				highlight = { 177, lerp(DARK_GREEN, LIGHTEST_GREEN, lerp_amount), BLACK };
			} else {
				highlight = { 177, lerp(DARK_RED, LIGHTEST_RED, lerp_amount), BLACK };				
			}
			console->set_char(console->at(step.x, step.y), highlight);
			++i;
		}
	}

	// Render our destination
	console->set_char(console->at(destination.x, destination.y), destination_glyph);

	// Finally, we render the @ symbol. dude_x and dude_y are in terminal coordinates.
	console->set_char(console->at(dude_position.x, dude_position.y), dude);
}

// Your main function
int main()
{
	// Initialize with defaults
	init(config_simple_px("../assets"));

	// Enter the main loop. "tick" is the function we wrote above.
	run(tick);

    return 0;
}
