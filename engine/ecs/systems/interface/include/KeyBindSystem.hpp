#pragma once

#include <optional>
#include <string>

#include "ISystem.hpp"
#include "KeyBind.hpp"
#include "Registry.hpp"
#include "UiInput.hpp"
#include "UiStore.hpp"

namespace engine::ecs {

class KeyBindSystem : public ISystem
{
public:
    static constexpr const char *CAPTURING_KEY = "settings.capturing";
    static constexpr const char *YES = "yes";
    static constexpr const char *NO = "no";
    static constexpr const char *RAW_PREFIX = "#";

    KeyBindSystem(Registry &registry, const UiInput &input, UiStore &store);

    void update(float dt) override;

private:
    void capture(const Entity &entity, KeyBind &bind);
    void finish(KeyBind &bind, const std::string &value);
    std::optional<std::string> captured(const KeyBind &bind) const;
    bool activated() const;

    Registry &m_registry;
    const UiInput &m_input;
    UiStore &m_store;
    bool m_releasing;
};

} // namespace engine::ecs
