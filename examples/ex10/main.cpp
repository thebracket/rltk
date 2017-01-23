/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Example 10: Not really an example yet, playing with getting the ECS working.
 */

// You need to include the RLTK header
#include "../../rltk/rltk.hpp"

// We're using a stringstream to build the hello world message.
#include <sstream>
#include <algorithm>

// For convenience, import the whole rltk namespace. You may not want to do this
// in larger projects, to avoid naming collisions.
using namespace rltk;
using namespace rltk::colors;
using std::size_t;

constexpr int map_width = 100;
constexpr int map_height = 100;
constexpr int map_size = map_width * map_height;
int map_idx(const int x, const int y) { return (y * map_width) + x; }
std::vector<int> map_tiles;
std::vector<uint8_t> visible;
std::vector<bool> revealed;
random_number_generator rng;

size_t player_id;

struct position { 
	position() {}
	position(const int X, const int Y) : x(X), y(Y) {}

	int x, y;	

	void bounds_check() {
		if (x < 0) x = 0;
		if (x > map_width) x = map_width;
		if (y < 0) y = 0;
		if (y > map_height) y = map_height;
	}
};

struct renderable { 
	renderable() {}
	renderable(const char Glyph, const color_t foreground) : glyph(Glyph), fg(foreground) {}
	int glyph; 
	color_t fg=colors::WHITE; 
	color_t bg=colors::BLACK; 
};

struct navigator_helper {
	static int get_x(const position &loc) { return loc.x; }
	static int get_y(const position &loc) { return loc.y; }
	static position get_xy(const int &x, const int &y) { return position{x,y}; }
};

// Clipping info
int left_x, right_x, top_y, bottom_y;

struct actor_moved_message : base_message_t {
	actor_moved_message() {}
	actor_moved_message(entity_t * ACTOR, const int fx, const int fy, const int dx, const int dy) :
		mover(ACTOR), from_x(fx), from_y(fy), destination_x(dx), destination_y(dy) {}

	entity_t * mover;
	int from_x, from_y, destination_x, destination_y;
};

struct player_moved_message : base_message_t {};

struct camera_system : public base_system {
	virtual void configure() override {
		system_name = "Camera System";
		// Create the player
		auto player = create_entity()
			->assign(position{map_width/2, map_height/2})
			->assign(renderable('@', colors::YELLOW));
		player_id = player->id;
	}

	virtual void update(const double duration_ms) override {
		if (console->dirty) {
			console->clear();

			// Find the camera
			auto camera_loc = entity(player_id)->component<position>();
			left_x = camera_loc->x - (console->term_width / 2);
			right_x = camera_loc->x + (console->term_width / 2);
			top_y = camera_loc->y - (console->term_height/2);
			bottom_y = camera_loc->y + (console->term_height/2)+1;

			for (int y=top_y; y<bottom_y; ++y) {
				for (int x=left_x; x<right_x; ++x) {
					if (x >= 0 && x < map_width && y >= 0 && y < map_height) {
						vchar map_char{'.', color_t(0,0,64), colors::BLACK};
						const int map_index = map_idx(x,y);

						if (revealed[map_index]) {
							switch (map_tiles[map_index]) {
								case 0 : map_char.glyph = '.'; break;
								case 1 : map_char.glyph = '#'; break;
								default : map_char.glyph = 'E'; // This shouldn't happen
							}
							map_char.foreground = color_t(96,96,96);
						}
						if (visible[map_index] > 0) {
							uint8_t brightness =(visible[map_index]*16) + 127;
							map_char.foreground = color_t(brightness, brightness, brightness);
						} else {
							if (map_tiles[map_index] == 0) map_char.glyph = ' ';
						}
						console->set_char(x-left_x, y-top_y, map_char);
					}
				}
			}
		}		
	}
};

struct actor_render_system : public base_system {
	virtual void configure() override {
		system_name = "Actor Render System";
	}

	virtual void update(const double duration_ms) override {
		each<position, renderable>([] (entity_t &entity, position &pos, renderable &render) {
			const int map_index = map_idx(pos.x, pos.y);
			if (visible[map_index]) {
				console->set_char(pos.x-left_x, pos.y-top_y, vchar{ render.glyph, render.fg, render.bg });
			}
		});
	}
};

struct player_system : public base_system {
	virtual void configure() override {
		system_name = "Player System";
		subscribe<actor_moved_message>([this](actor_moved_message &msg) {
			if (map_tiles[map_idx(msg.destination_x, msg.destination_y)] == 0) {
				msg.mover->component<position>()->x = msg.destination_x;
				msg.mover->component<position>()->y = msg.destination_y;
				msg.mover->component<position>()->bounds_check();
				console->dirty = true;
				emit(player_moved_message{});
			}
		});

		subscribe_mbox<key_pressed_t>();
	}

	virtual void update(const double duration_ms) override {
		auto camera_loc = entity(player_id)->component<position>();

		// Loop through the keyboard input list
		std::queue<key_pressed_t> * messages = mbox<key_pressed_t>();
		while (!messages->empty()) {
			key_pressed_t e = messages->front();
			messages->pop();
			
			if (e.event.key.code == sf::Keyboard::Left) emit(actor_moved_message{ entity(player_id), camera_loc->x, camera_loc->y, camera_loc->x - 1, camera_loc->y });
			if (e.event.key.code == sf::Keyboard::Right) emit(actor_moved_message{ entity(player_id), camera_loc->x, camera_loc->y, camera_loc->x + 1, camera_loc->y });
			if (e.event.key.code == sf::Keyboard::Up) emit(actor_moved_message{ entity(player_id), camera_loc->x, camera_loc->y, camera_loc->x, camera_loc->y-1 });
			if (e.event.key.code == sf::Keyboard::Down) emit(actor_moved_message{ entity(player_id), camera_loc->x, camera_loc->y, camera_loc->x, camera_loc->y+1 });
			if (e.event.key.code == sf::Keyboard::F1) {
				std::string timings = ecs_profile_dump();
				std::cout << timings << "\n";
			}
		}
	}
};

struct visibility_system : public base_system {
	virtual void configure() override {
		system_name = "Visibility System";
		subscribe<player_moved_message>([this](player_moved_message &msg) {
			auto camera_loc = entity(player_id)->component<position>();
			position camera_loc_deref = *camera_loc;

			std::fill(visible.begin(), visible.end(), 0);
			visibility_sweep_2d<position, navigator_helper>(camera_loc_deref, 10,
				[](position reveal) {
				reveal.bounds_check();
				const int idx = map_idx(reveal.x, reveal.y);
				++visible[idx];
				if (visible[idx] > 8) visible[idx] = 8;
				revealed[idx] = true;
			},	[](position visibility_check) {
				visibility_check.bounds_check();
				return (map_tiles[map_idx(visibility_check.x, visibility_check.y)] == 0);
			});
		});
	}

	bool firstrun = true;

	virtual void update(const double duration_ms) override {
		if (firstrun) {
			emit(player_moved_message{});
			firstrun = false;
		}
	}
};

// Tick is called every frame. The parameter specifies how many ms have elapsed
// since the last time it was called.
void tick(double duration_ms) {
	ecs_tick(duration_ms);
}

// Your main function
int main()
{
	// Initialize the library. Here, we are providing plenty of into so you can see what is
	// available. There's also the option to use config_simple_px to configure by pixels
	// rather than characters.
	// The first parameter is the path to the font files.
	// The second and third parameters specify the desired console size in screen characters (80x25).
	// The fourth parameter is the window title.
	// The final parameter says that we'd like the default console to use an 8x16 VGA font. Not so great for games, but easy to read!
	init(config_simple("../assets", 80, 50, "RLTK Hello World", "8x8"));
	
	// Zero the map other than the edges
	map_tiles.resize(map_size);
	visible.resize(map_size);
	revealed.resize(map_size);
	std::fill(map_tiles.begin(), map_tiles.end(), 0);
	std::fill(visible.begin(), visible.end(), false);
	std::fill(revealed.begin(), revealed.end(), false);
	for (int i=0; i<map_width; ++i) {
		map_tiles[map_idx(i, 0)] = 1;
		map_tiles[map_idx(i, map_height-1)] = 1;
	}
	for (int i=0; i<map_width; ++i) {
		map_tiles[map_idx(0, i)] = 1;
		map_tiles[map_idx(map_width-1, i)] = 1;
	}
	// Random debris
	for (int y=1; y<map_height-1; ++y) {
		for (int x=1; x<map_width-1; ++x) {
			if (rng.roll_dice(1,4)==1 && (x!=map_width/2 || y!=map_height/2)) map_tiles[map_idx(x,y)]=1;
		}
	}

	// Create our systems
	add_system<player_system>();
	add_system<visibility_system>();
	add_system<camera_system>();
	add_system<actor_render_system>();

	// Enter the main loop. "tick" is the function we wrote above.
	ecs_configure();
	run(tick);

    return 0;
}
