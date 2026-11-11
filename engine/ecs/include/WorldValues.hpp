#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace engine::ecs {

class WorldValues
{
public:
    static constexpr int MAX_VALUE = 65535;

    explicit WorldValues(std::size_t count);

    std::size_t size() const;
    int value(std::size_t index) const;
    void set(std::size_t index, int value);
    const std::vector<std::uint16_t> &values() const;
    void reset();

private:
    std::vector<std::uint16_t> m_values;
};

} // namespace engine::ecs
