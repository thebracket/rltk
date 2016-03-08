#include "rltk.hpp"
#include <SFML/Graphics.hpp>

namespace rltk {

void run(std::function<void()> on_tick, const int window_width, const int window_height, const std::string window_title) {
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), window_title);
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        on_tick();
        window.draw(shape);
        window.display();
    }
}

}
