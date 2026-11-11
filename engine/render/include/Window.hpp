#pragma once

#include <optional>
#include <string>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include "UiInput.hpp"
#include "UiKey.hpp"
#include "Vector2.hpp"

namespace engine::render {

class Window
{
public:
    static constexpr unsigned int FRAME_RATE_LIMIT = 60;
    static constexpr unsigned int CONFIRM_BUTTON = 0;
    static constexpr unsigned int SELECT_BUTTON = 6;
    static constexpr float STICK_THRESHOLD = 50.f;

    Window(unsigned int width, unsigned int height, const std::string &title, Vector2 worldSize);

    void processEvents(UiInput &ui);
    bool isOpen() const;
    bool hasFocus() const;
    void close();

    sf::RenderWindow &native();

private:
    Vector2 toWorld(sf::Vector2i pixel) const;
    static std::optional<UiKey> uiKeyOf(sf::Keyboard::Key key);
    void handleStick(UiInput &ui, const sf::Event::JoystickMoved &moved);
    static void pressOnce(UiInput &ui, bool up, bool down, bool &wasUp, bool &wasDown);
    static void pressSideOnce(UiInput &ui, bool left, bool right, bool &wasLeft, bool &wasRight);

    sf::RenderWindow m_window;
    bool m_stickUp;
    bool m_stickDown;
    bool m_padUp;
    bool m_padDown;
    bool m_stickLeft;
    bool m_stickRight;
    bool m_padLeft;
    bool m_padRight;
};

} // namespace engine::render
