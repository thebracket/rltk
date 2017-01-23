#include "rltk.hpp"
#include "texture.hpp"
#include <memory>

namespace rltk {

std::unique_ptr<sf::RenderWindow> main_window;
std::unique_ptr<virtual_terminal> console;
std::unique_ptr<gui_t> gui;

namespace main_detail {
bool use_root_console;
bool taking_screenshot = false;
std::string screenshot_filename = "";
}

sf::RenderWindow * get_window() {
	return main_window.get();
}

gui_t * get_gui() {
    return gui.get();
}

void init(const config_simple &config) {
    register_font_directory(config.font_path);
    bitmap_font * font = get_font(config.root_font);
    if (!config.fullscreen) {
        main_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(config.width * font->character_size.first, 
            config.height * font->character_size.second), 
            config.window_title, sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close);
    } else {
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        main_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(desktop.width, desktop.height, desktop.bitsPerPixel)
            , config.window_title, sf::Style::Fullscreen);
    }
    main_window->setVerticalSyncEnabled(true);
    main_detail::use_root_console = true;

    console = std::make_unique<virtual_terminal>(config.root_font, 0, 0);
    sf::Vector2u size_pixels = main_window->getSize();
    console->resize_pixels(size_pixels.x, size_pixels.y);
}

void init(const config_simple_px &config) {
    register_font_directory(config.font_path);
    if (!config.fullscreen) {
        main_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(config.width_px, config.height_px), 
            config.window_title, sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close);
    } else {
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        main_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(desktop.width, desktop.height, desktop.bitsPerPixel)
            , config.window_title, sf::Style::Fullscreen);
    }
    main_window->setVerticalSyncEnabled(true);
    main_detail::use_root_console = true;

    console = std::make_unique<virtual_terminal>(config.root_font, 0, 0);
    sf::Vector2u size_pixels = main_window->getSize();
    console->resize_pixels(size_pixels.x, size_pixels.y);
}

void init(const config_advanced &config) {
    register_font_directory(config.font_path);
    if (!config.fullscreen) {
        main_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(config.width_px, config.height_px), 
            config.window_title, sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close);
    } else {
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        main_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(desktop.width, desktop.height, desktop.bitsPerPixel)
            , config.window_title, sf::Style::Fullscreen);
    }
    main_window->setVerticalSyncEnabled(true);
    main_detail::use_root_console = false;

    gui = std::make_unique<gui_t>(config.width_px, config.height_px);
}

std::function<bool(sf::Event)> optional_event_hook = nullptr;
std::function<void()> optional_display_hook = nullptr;

void run(std::function<void(double)> on_tick) {    
    reset_mouse_state();

    double duration_ms = 0.0;
    while (main_window->isOpen())
    {
    	clock_t start_time = clock();

        sf::Event event;
        while (main_window->pollEvent(event))
        {
            bool handle_events = true;
            if (optional_event_hook) {
                handle_events = optional_event_hook(event);
            }
            if (handle_events) {
                if (event.type == sf::Event::Closed) {
                    main_window->close();
                } else if (event.type == sf::Event::Resized) {
                    main_window->setView(sf::View(sf::FloatRect(0.f, 0.f, static_cast<float>(event.size.width),
                                                                static_cast<float>(event.size.height))));
                    if (main_detail::use_root_console) console->resize_pixels(event.size.width, event.size.height);
                    if (gui) gui->on_resize(event.size.width, event.size.height);
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
                } else if (event.type == sf::Event::KeyPressed) {
                    enqueue_key_pressed(event);
                }
            }
        }

        main_window->clear();
        //if (main_detail::use_root_console) console->clear();

        on_tick(duration_ms);

        if (main_detail::use_root_console) {
            console->render(*main_window);
        } else {
            gui->render(*main_window);
        }

        if (optional_display_hook) {
            main_window->pushGLStates();
            main_window->resetGLStates();
            optional_display_hook();
            main_window->popGLStates();
        }
        main_window->display();
        if (main_detail::taking_screenshot) {
            sf::Image screen = rltk::get_window()->capture();
            screen.saveToFile(main_detail::screenshot_filename);
            main_detail::screenshot_filename = "";
            main_detail::taking_screenshot = false;
        }

        duration_ms = ((clock() - start_time) * 1000.0) / CLOCKS_PER_SEC;
    }
}

void request_screenshot(const std::string &filename) {
    main_detail::taking_screenshot = true;
    main_detail::screenshot_filename = filename;
}

}
