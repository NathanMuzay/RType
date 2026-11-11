#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class ContactDamage : public IComponent
{
public:
    ContactDamage(int amount, bool destroyOnHit);

    int amount() const;
    bool destroyOnHit() const;

private:
    int m_amount;
    bool m_destroyOnHit;
};

} // namespace engine::ecs
