#include "Window.hpp"

#include <SFML/Graphics/View.hpp>

#include "RenderError.hpp"

namespace engine::render {

Window::Window(unsigned int width, unsigned int height, const std::string &title, Vector2 worldSize)
    : m_window(sf::VideoMode({width, height}), title), m_stickUp(false), m_stickDown(false), m_padUp(false),
      m_padDown(false), m_stickLeft(false), m_stickRight(false), m_padLeft(false), m_padRight(false)
{
    if (!m_window.isOpen())
        throw RenderError("Window: cannot open the window");
    m_window.setFramerateLimit(FRAME_RATE_LIMIT);
    m_window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {worldSize.x(), worldSize.y()})));
}

void Window::processEvents(UiInput &ui)
{
    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
        } else if (const auto *moved = event->getIf<sf::Event::MouseMoved>()) {
            ui.movePointer(toWorld(moved->position));
        } else if (const auto *pressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (pressed->button == sf::Mouse::Button::Left)
                ui.pressPointer(toWorld(pressed->position));
        } else if (const auto *released = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (released->button == sf::Mouse::Button::Left)
                ui.releasePointer();
        } else if (const auto *text = event->getIf<sf::Event::TextEntered>()) {
            if (text->unicode <= static_cast<char32_t>(UiInput::LAST_PRINTABLE))
                ui.typeCharacter(static_cast<char>(text->unicode));
        } else if (const auto *button = event->getIf<sf::Event::JoystickButtonPressed>()) {
            ui.pressRawButton(button->button);
            if (button->button == CONFIRM_BUTTON)
                ui.pressKey(UiKey::Enter);
            else if (button->button == SELECT_BUTTON)
                ui.pressKey(UiKey::Escape);
        } else if (const auto *stick = event->getIf<sf::Event::JoystickMoved>()) {
            handleStick(ui, *stick);
        } else if (const auto *key = event->getIf<sf::Event::KeyPressed>()) {
            ui.pressRawKey(static_cast<int>(key->scancode));
            if (const auto uiKey = uiKeyOf(key->code))
                ui.pressKey(*uiKey);
        }
    }
}

bool Window::isOpen() const
{
    return m_window.isOpen();
}

bool Window::hasFocus() const
{
    return m_window.hasFocus();
}

void Window::close()
{
    m_window.close();
}

sf::RenderWindow &Window::native()
{
    return m_window;
}

Vector2 Window::toWorld(sf::Vector2i pixel) const
{
    const sf::Vector2f world = m_window.mapPixelToCoords(pixel);
    return {world.x, world.y};
}

std::optional<UiKey> Window::uiKeyOf(sf::Keyboard::Key key)
{
    switch (key) {
    case sf::Keyboard::Key::Backspace:
        return UiKey::Backspace;
    case sf::Keyboard::Key::Enter:
        return UiKey::Enter;
    case sf::Keyboard::Key::Escape:
        return UiKey::Escape;
    case sf::Keyboard::Key::Tab:
        return UiKey::Tab;
    case sf::Keyboard::Key::Up:
        return UiKey::Up;
    case sf::Keyboard::Key::Down:
        return UiKey::Down;
    case sf::Keyboard::Key::F3:
        return UiKey::Debug;
    case sf::Keyboard::Key::F1:
        return UiKey::Console;
    case sf::Keyboard::Key::Left:
        return UiKey::Left;
    case sf::Keyboard::Key::Right:
        return UiKey::Right;
    default:
        return std::nullopt;
    }
}

void Window::handleStick(UiInput &ui, const sf::Event::JoystickMoved &moved)
{
    if (moved.axis == sf::Joystick::Axis::Y)
        pressOnce(ui, moved.position<-STICK_THRESHOLD, moved.position> STICK_THRESHOLD, m_stickUp, m_stickDown);
    else if (moved.axis == sf::Joystick::Axis::PovY)
        pressOnce(ui, moved.position > STICK_THRESHOLD, moved.position < -STICK_THRESHOLD, m_padUp, m_padDown);
    else if (moved.axis == sf::Joystick::Axis::X)
        pressSideOnce(ui, moved.position<-STICK_THRESHOLD, moved.position> STICK_THRESHOLD, m_stickLeft, m_stickRight);
    else if (moved.axis == sf::Joystick::Axis::PovX)
        pressSideOnce(ui, moved.position<-STICK_THRESHOLD, moved.position> STICK_THRESHOLD, m_padLeft, m_padRight);
}

void Window::pressOnce(UiInput &ui, bool up, bool down, bool &wasUp, bool &wasDown)
{
    if (up && !wasUp)
        ui.pressKey(UiKey::Up);
    if (down && !wasDown)
        ui.pressKey(UiKey::Down);
    wasUp = up;
    wasDown = down;
}

void Window::pressSideOnce(UiInput &ui, bool left, bool right, bool &wasLeft, bool &wasRight)
{
    if (left && !wasLeft)
        ui.pressKey(UiKey::Left);
    if (right && !wasRight)
        ui.pressKey(UiKey::Right);
    wasLeft = left;
    wasRight = right;
}

} // namespace engine::render
