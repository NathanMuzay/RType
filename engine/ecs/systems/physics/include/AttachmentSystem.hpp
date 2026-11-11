#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class AttachmentSystem : public ISystem
{
public:
    explicit AttachmentSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

} // namespace engine::ecs
