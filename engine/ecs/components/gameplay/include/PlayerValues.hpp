#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "IComponent.hpp"

namespace engine::ecs {

class PlayerValues : public IComponent
{
public:
    static constexpr int MAX_VALUE = 65535;

    explicit PlayerValues(std::size_t count);

    const std::vector<std::uint16_t> &values() const;
    void set(std::size_t index, int value);
    void reset();

private:
    std::vector<std::uint16_t> m_values;
};

} // namespace engine::ecs
