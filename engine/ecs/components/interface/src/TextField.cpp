#include "TextField.hpp"

#include <utility>

namespace engine::ecs {

TextField::TextField(std::string name, std::size_t maxLength, bool masked, std::string placeholder, float width,
    float height, std::string submit)
    : m_name(std::move(name)), m_content(), m_maxLength(maxLength), m_masked(masked),
      m_placeholder(std::move(placeholder)), m_submit(std::move(submit)), m_width(width), m_height(height),
      m_active(false)
{
}

const std::string &TextField::name() const
{
    return m_name;
}

const std::string &TextField::content() const
{
    return m_content;
}

void TextField::setContent(std::string content)
{
    m_content = std::move(content);
    if (m_content.size() > m_maxLength)
        m_content.resize(m_maxLength);
}

void TextField::append(const std::string &text)
{
    for (char character : text) {
        if (m_content.size() < m_maxLength)
            m_content.push_back(character);
    }
}

void TextField::erase()
{
    if (!m_content.empty())
        m_content.pop_back();
}

const std::string &TextField::placeholder() const
{
    return m_placeholder;
}

const std::string &TextField::submit() const
{
    return m_submit;
}

bool TextField::active() const
{
    return m_active;
}

void TextField::setActive(bool active)
{
    m_active = active;
}

bool TextField::contains(Vector2 origin, Vector2 point) const
{
    return point.x() >= origin.x() && point.x() <= origin.x() + m_width && point.y() >= origin.y()
        && point.y() <= origin.y() + m_height;
}

std::string TextField::display() const
{
    if (m_content.empty() && !m_active)
        return m_placeholder;
    std::string shown = m_masked ? std::string(m_content.size(), MASK_CHARACTER) : m_content;
    if (m_active)
        shown.push_back(CARET_CHARACTER);
    return shown;
}

float TextField::width() const
{
    return m_width;
}

float TextField::height() const
{
    return m_height;
}

} // namespace engine::ecs
