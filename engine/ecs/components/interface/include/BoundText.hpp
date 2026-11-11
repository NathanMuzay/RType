#pragma once

#include <string>

#include "IComponent.hpp"

namespace engine::ecs {

class BoundText : public IComponent
{
public:
    explicit BoundText(std::string format);

    const std::string &format() const;

private:
    std::string m_format;
};

} // namespace engine::ecs
