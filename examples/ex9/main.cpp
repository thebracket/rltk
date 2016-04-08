#include <iostream>
/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Example 9: This is example 6, but using two consoles. One for the map, and one sparse. This allows
 * for some trickery to speed up map rendering (we're only redrawing when we need to). We also use smooth
 * movement, which many people may or may not like - but is an important feature to offer. Finally, we're
 * 'bouncing' the @ left and right to demonstrate rotation.
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

// Now we define a structure to represent a location. In this case, it's a simple
// x/y coordinate.
struct location_t {
	float x=-1.0f; // I like to set uninitialized values to something invalid for help with debugging
	float y=-1.0f;

	// For convenience, we're overriding the quality operator. This gives a very
	// quick and natural looking way to say "are these locations the same?"
	bool operator==(location_t &rhs) { return (std::floor(x)==std::floor(rhs.x) && std::floor(y)==std::floor(rhs.y)); }

	location_t() {}
	location_t(const int X, const int Y) : x(X), y(Y) {}
};

// Now we define our basic map. Why a struct? Because a struct is just a class with
// everything public in it!
struct map_t {
	map_t(const int &w, const int &h) : width(w), height(h) {
		// Resize the vectors to hold the whole map; this way it won't reallocate
		walkable.resize(w*h);
		revealed.resize(w*h);
		visible.resize(w*h);

		// Set the entire map to walkable, not visible and not revealed
		std::fill(walkable.begin(), walkable.end(), true);
		std::fill(revealed.begin(), revealed.end(), false);
		std::fill(visible.begin(), visible.end(), false);

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
				if ((x != 10 && y != 10) && rng.roll_dice(1,3)==1) walkable[at(x,y)] = false;
			}
		}
	}

	// Calculate the vector offset of a grid location
	inline int at(const int &x, const int &y) { return (y*width)+x; }

	// The width and height of the map
	const int width, height;

	// The actual walkable storage vector
	std::vector<bool> walkable;

	// Revealed: has a tile been shown yet?
	std::vector<bool> revealed;

	// Visible: is a tile currently visible?
	std::vector<bool> visible;
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

	// We're using the Bresneham's line optimization for pathing this time, which requires a few extra
	// static methods. These are designed to translate between your map format and co-ordinates used by
	// the library (we don't want to force you to structure things a certain way).
	static int get_x(const location_t &loc) { return loc.x; }
	static int get_y(const location_t &loc) { return loc.y; }
	static location_t get_xy(const int &x, const int &y) { return location_t{x,y}; }
	static bool is_walkable(const location_t &loc) { return map.walkable[map.at(loc.x, loc.y)]; }
};

// Lets go really fast!
constexpr double tick_duration = 0.0;
double tick_time = 0.0;

// Helper function: calls the RLTK visibility sweep 2D algorithm with lambdas to
// assist in understanding our map format.
inline void visibility_sweep() {
	visibility_sweep_2d<location_t, navigator>(dude_position, 10, 
		[] (location_t reveal) { 
			map.revealed[map.at(reveal.x, reveal.y)] = true;
			map.visible[map.at(reveal.x, reveal.y)] = true;
		},
		[] (auto test_visibility) { return map.walkable[map.at(test_visibility.x, test_visibility.y)]; }
	);
}

// Tick is called every frame. The parameter specifies how many ms have elapsed
// since the last time it was called.
void tick(double duration_ms) {
	int angle = 0;

	// Increase the tick time by the frame duration. If it has exceeded
	// the tick duration, then we move the @.
	tick_time += duration_ms;
	if (tick_time > tick_duration) {
		// Are we there yet?
		if (dude_position == destination) {
			// Now we poll the mouse to determine where we want to go
			// This requests the mouse position in PIXELS, and ties it into our mouse_x/mouse_y variables.
			int mouse_x, mouse_y;
			std::tie(mouse_x, mouse_y) = get_mouse_position();

			// Since we're using an 8x8, it's just a matter of dividing by 8 to find the terminal-character
			// coordinates. There will be a helper function for this once we get into retained GUIs.
			const int terminal_x = mouse_x / 8;
			const int terminal_y = mouse_y / 8;

			// If the mouse is pointing at a walkable location, and the left button is down - path to the mouse.
			if (map.walkable[map.at(terminal_x, terminal_y)] && get_mouse_button_state(rltk::button::LEFT)) {
				destination.x = terminal_x;
				destination.y = terminal_y;

				// Now determine how to get there
				if (path) path.reset();
				path = find_path<location_t, navigator>(dude_position, destination);
				if (!path->success) {
					destination = dude_position;
					std::cout << "RESET: THIS ISN'T MEANT TO HAPPEN!\n";
				}
			} else {
				// If the mouse is not clicked, then path to the mouse cursor for display only
				if (path) path.reset();
				path = find_path_2d<location_t, navigator>(dude_position, location_t{terminal_x, terminal_y});
			}
		} else {
			// Follow the breadcrumbs!
			if (path) {
				location_t next_step = path->steps.front();
				//dude_position.x = next_step.x;
				//dude_position.y = next_step.y;
				if (dude_position.x > next_step.x) { dude_position.x -= 0.25f; angle = 315; }
				if (dude_position.x < next_step.x) { dude_position.x += 0.25f; angle = 45; }
				if (dude_position.y > next_step.y) dude_position.y -= 0.25f;
				if (dude_position.y < next_step.y) dude_position.y += 0.25f;
				if (std::floor(dude_position.x) == next_step.x && std::floor(dude_position.y) == next_step.y) path->steps.pop_front();

				// Update the map visibility
				std::fill(map.visible.begin(), map.visible.end(), false);
				visibility_sweep();
				term(1)->dirty = true;
			}
		}

		// Important: we clear the tick count after the update.
		tick_time = 0.0;
	}

	// Render our planned path. We're using auto and a range-for to avoid typing all
	// the iterator stuff
	sterm(2)->clear();

	if (path) {
		// We're going to show off a bit and "lerp" the color along the path; the red
		// lightens as it approaches the destination. This is a preview of some of the
		// color functions.
		const float n_steps = path->steps.size();
		float i = 0;
		for (auto step : path->steps) {
			const float lerp_amount = i / n_steps;
			vchar highlight;
			// If we're at our destination, we are showing possible paths - highlight green;
			// otherwise, highlight red to indicate that we are en route.
			if (dude_position == destination) {
				highlight = { 177, lerp(DARK_GREEN, LIGHTEST_GREEN, lerp_amount), BLACK };
			} else {
				highlight = { 177, lerp(DARK_RED, LIGHTEST_RED, lerp_amount), BLACK };				
			}
			sterm(2)->add(xchar( 177, highlight.foreground, static_cast<float>(step.x), static_cast<float>(step.y) ));
			++i;
		}
	}

	// Render our destination
	term(1)->set_char(term(1)->at(destination.x, destination.y), destination_glyph);

	// Finally, we render the @ symbol. dude_x and dude_y are in terminal coordinates.
	//term(1)->set_char(term(1)->at(dude_position.x, dude_position.y), dude);
	sterm(2)->add(xchar(
		'@', YELLOW, static_cast<float>(dude_position.x), static_cast<float>(dude_position.y), angle
	));

	// Iterate over the whole map, rendering as appropriate
	if (term(1)->dirty) {
		for (int y=0; y<MAP_HEIGHT; ++y) {
			for (int x=0; x<MAP_WIDTH; ++x) {
				const int map_idx = map.at(x,y); // Caching so we don't keep doing the calculation
				if (map.walkable[map_idx]) {
					if (map.visible[map_idx]) {
						// Visible tile: render full color
						term(1)->set_char(map_idx, vchar{'.', GREEN, BLACK});
					} else if (map.revealed[map_idx]) {
						// Revealed tile: render grey
						term(1)->set_char(map_idx, vchar{'.', LIGHTER_GREY, BLACK});
					} else {
						// We haven't seen it yet - darkest gray
						term(1)->set_char(map_idx, vchar{'.', DARK_GREY, BLACK});
					}
				} else {
					if (map.visible[map_idx]) {
						// Visible tile: render full color
						term(1)->set_char(map_idx, vchar{'#', CYAN, BLACK});
					} else if (map.revealed[map_idx]) {
						// Revealed tile: render grey
						term(1)->set_char(map_idx, vchar{'#', GREY, BLACK});
					} else {
						// We haven't seen it yet - darkest gray
						term(1)->set_char(map_idx, vchar{'#', DARKER_GREY, BLACK});
					}
				}
			}
		}
	}	
}

void resize_map(layer_t * l, int w, int h) {
	// Use the whole window
	l->w = w;
	l->h = h;
}

// Your main function
int main()
{
	// Initialize with defaults
	init(config_advanced("../assets"));
	gui->add_layer(1, 0, 0, 1024, 768, "8x8", resize_map);
	gui->add_sparse_layer(2, 0, 0, 1024, 768, "8x8", resize_map); // Our sparse layer

	// We do a visibility sweep to start, so your starting position is revealed
	visibility_sweep();

	// Enter the main loop. "tick" is the function we wrote above.
	run(tick);

    return 0;
}
