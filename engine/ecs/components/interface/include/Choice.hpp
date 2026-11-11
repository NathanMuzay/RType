#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "IComponent.hpp"

namespace engine::ecs {

class Choice : public IComponent
{
public:
    static constexpr const char *STORE_PREFIX = "field.";

    Choice(std::string name, std::vector<std::string> options);

    const std::string &name() const;
    const std::string &current() const;
    void next();
    void previous();

private:
    std::string m_name;
    std::vector<std::string> m_options;
    std::size_t m_index;
};

} // namespace engine::ecs
