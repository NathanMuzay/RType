#pragma once

#include <cstddef>
#include <optional>

#include "Entity.hpp"
#include "IComponent.hpp"

namespace engine::ecs {

class Player : public IComponent
{
public:
    explicit Player(std::size_t slot);

    std::size_t slot() const;
    std::optional<Entity> avatar() const;
    void setAvatar(Entity avatar);
    void clearAvatar();

private:
    std::size_t m_slot;
    std::optional<Entity> m_avatar;
};

} // namespace engine::ecs
