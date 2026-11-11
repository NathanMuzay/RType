#include "RowData.hpp"

#include <utility>

namespace engine::ecs {

RowData::RowData(UiStore::Row values) : m_values(std::move(values))
{
}

const UiStore::Row &RowData::values() const
{
    return m_values;
}

} // namespace engine::ecs
