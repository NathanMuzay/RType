#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "IComponent.hpp"

namespace engine::ecs {

class SettingSlider : public IComponent
{
public:
    static constexpr const char *STORE_PREFIX = "setting.";

    SettingSlider(std::string setting, std::vector<std::string> values, bool wrap, int direction = 0);

    std::string storeKey() const;
    std::string step(const std::string &current, int direction) const;
    int direction() const;

private:
    std::size_t indexOf(const std::string &current) const;

    std::string m_setting;
    std::vector<std::string> m_values;
    bool m_wrap;
    int m_direction;
};

} // namespace engine::ecs
