#include "rltk.hpp"
#include "texture.hpp"
#include <memory>

namespace rltk {

std::unique_ptr<sf::RenderWindow> main_window;
std::unique_ptr<virtual_terminal> root_console;

sf::RenderWindow * get_window() {
	return main_window.get();
}

virtual_terminal * get_root_console() {
    return root_console.get();
}

void init(const int window_width, const int window_height, const std::string window_title) {
	main_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(window_width, window_height, sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close), window_title);
    main_window->setVerticalSyncEnabled(true);
}

void run(std::function<void(double)> on_tick, const std::string root_console_font) {
    root_console = std::make_unique<virtual_terminal>(root_console_font, 0, 0);
    sf::Vector2u size_pixels = main_window->getSize();
    root_console->resize_pixels(size_pixels.x, size_pixels.y);

    double duration_ms = 0.0;
    while (main_window->isOpen())
    {
    	clock_t start_time = clock();

        sf::Event event;
        while (main_window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                main_window->close();
            }
        }

        main_window->clear();
        root_console->clear();

        on_tick(duration_ms);

        root_console->render(*main_window);

        main_window->display();

        duration_ms = ((clock() - start_time) * 1000.0) / CLOCKS_PER_SEC;
    }
}

}
