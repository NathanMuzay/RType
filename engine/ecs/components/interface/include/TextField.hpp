#pragma once

#include <cstddef>
#include <string>

#include "IComponent.hpp"
#include "Vector2.hpp"

namespace engine::ecs {

class TextField : public IComponent
{
public:
    static constexpr char MASK_CHARACTER = '*';
    static constexpr char CARET_CHARACTER = '_';

    TextField(std::string name, std::size_t maxLength, bool masked, std::string placeholder, float width, float height,
        std::string submit = {});

    const std::string &name() const;
    const std::string &content() const;
    void setContent(std::string content);
    void append(const std::string &text);
    void erase();
    const std::string &placeholder() const;
    const std::string &submit() const;
    bool active() const;
    void setActive(bool active);
    bool contains(Vector2 origin, Vector2 point) const;
    float width() const;
    float height() const;
    std::string display() const;

private:
    std::string m_name;
    std::string m_content;
    std::size_t m_maxLength;
    bool m_masked;
    std::string m_placeholder;
    std::string m_submit;
    float m_width;
    float m_height;
    bool m_active;
};

} // namespace engine::ecs
