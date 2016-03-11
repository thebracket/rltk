#include "rltk.hpp"
#include "texture.hpp"
#include <memory>

namespace rltk {

std::unique_ptr<sf::RenderWindow> main_window;
std::unique_ptr<virtual_terminal> console;

sf::RenderWindow * get_window() {
	return main_window.get();
}

void init(const int window_width, const int window_height, const std::string window_title) {
	main_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(window_width, window_height, sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close), window_title);
    main_window->setVerticalSyncEnabled(true);
}

void run(std::function<void(double)> on_tick, const std::string root_console_font) {
    console = std::make_unique<virtual_terminal>(root_console_font, 0, 0);
    sf::Vector2u size_pixels = main_window->getSize();
    console->resize_pixels(size_pixels.x, size_pixels.y);
    reset_mouse_state();

    double duration_ms = 0.0;
    while (main_window->isOpen())
    {
    	clock_t start_time = clock();

        sf::Event event;
        while (main_window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                main_window->close();
            } else if (event.type == sf::Event::Resized) {
                console->resize_pixels(event.size.width, event.size.height);
                main_window->setView(sf::View(sf::FloatRect(0.f, 0.f, event.size.width, event.size.height)));
            } else if (event.type == sf::Event::LostFocus) {
                set_window_focus_state(false);
            } else if (event.type == sf::Event::GainedFocus) {
                set_window_focus_state(true);
            } else if (event.type == sf::Event::MouseButtonPressed) {                
                set_mouse_button_state(event.mouseButton.button, true);
            } else if (event.type == sf::Event::MouseButtonReleased) {
                set_mouse_button_state(event.mouseButton.button, false);
            } else if (event.type == sf::Event::MouseMoved) {
                set_mouse_position(event.mouseMove.x, event.mouseMove.y);
            }
        }

        main_window->clear();
        console->clear();

        on_tick(duration_ms);

        console->render(*main_window);

        main_window->display();

        duration_ms = ((clock() - start_time) * 1000.0) / CLOCKS_PER_SEC;
    }
}

}
