#include "rltk.hpp"
#include "texture.hpp"
#include <memory>

namespace rltk {

std::unique_ptr<sf::RenderWindow> main_window;

sf::RenderWindow * get_window() {
	return main_window.get();
}

void run(std::function<void(double)> on_tick, const int window_width, const int window_height, const std::string window_title) {

	std::make_unique<sf::RenderWindow>(sf::VideoMode(window_width, window_height, sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close), window_title);

    main_window->setVerticalSyncEnabled(true);

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
        sf::Vector2u size_pixels = main_window->getSize();

        on_tick(duration_ms);

        main_window->display();

        duration_ms = ((clock() - start_time) * 1000.0) / CLOCKS_PER_SEC;
    }
}

}
