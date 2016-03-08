#include "rltk.hpp"
#include <SFML/Graphics.hpp>

namespace rltk {

void run(std::function<void(double)> on_tick, const int window_width, const int window_height, const std::string window_title) {
    sf::RenderWindow window(sf::VideoMode(window_width, window_height, sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close), window_title);

    double duration_ms = 0.0;
    while (window.isOpen())
    {
    	clock_t start_time = clock();

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        on_tick(duration_ms);
        window.display();

        duration_ms = ((clock() - start_time) * 1000.0) / CLOCKS_PER_SEC;
    }
}

}
