#include "ContactDamage.hpp"

namespace engine::ecs {

ContactDamage::ContactDamage(int amount, bool destroyOnHit) : m_amount(amount), m_destroyOnHit(destroyOnHit)
{
}

int ContactDamage::amount() const
{
    return m_amount;
}

bool ContactDamage::destroyOnHit() const
{
    return m_destroyOnHit;
}

} // namespace engine::ecs
