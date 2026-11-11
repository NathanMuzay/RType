#pragma once

#include <string>

#include "ISystem.hpp"
#include "Registry.hpp"
#include "StatusText.hpp"

namespace engine::ecs {

class StatusTextSystem : public ISystem
{
public:
    explicit StatusTextSystem(Registry &registry);

    void update(float dt) override;

private:
    std::string render(const StatusText &status) const;

    Registry &m_registry;
};

} // namespace engine::ecs
