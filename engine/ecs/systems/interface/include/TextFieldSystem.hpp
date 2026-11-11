#pragma once

#include <string>

#include "ISystem.hpp"
#include "Registry.hpp"
#include "UiInput.hpp"
#include "UiStore.hpp"

namespace engine::ecs {

class TextFieldSystem : public ISystem
{
public:
    static constexpr const char *STORE_PREFIX = "field.";

    TextFieldSystem(Registry &registry, const UiInput &input, UiStore &store);

    void update(float dt) override;

private:
    Registry &m_registry;
    const UiInput &m_input;
    UiStore &m_store;
};

} // namespace engine::ecs
