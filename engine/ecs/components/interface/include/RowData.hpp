#pragma once

#include "IComponent.hpp"
#include "UiStore.hpp"

namespace engine::ecs {

class RowData : public IComponent
{
public:
    explicit RowData(UiStore::Row values);

    const UiStore::Row &values() const;

private:
    UiStore::Row m_values;
};

} // namespace engine::ecs
