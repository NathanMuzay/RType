#include "Player.hpp"

namespace engine::ecs {

Player::Player(std::size_t slot) : m_slot(slot)
{
}

std::size_t Player::slot() const
{
    return m_slot;
}

std::optional<Entity> Player::avatar() const
{
    return m_avatar;
}

void Player::setAvatar(Entity avatar)
{
    m_avatar = avatar;
}

void Player::clearAvatar()
{
    m_avatar.reset();
}

} // namespace engine::ecs
