#include "ListView.hpp"

#include <utility>

namespace engine::ecs {

ListView::ListView(std::string source, float spacing, std::size_t maxRows, std::vector<ListRowPart> parts)
    : m_source(std::move(source)), m_spacing(spacing), m_maxRows(maxRows), m_parts(std::move(parts)), m_version(0),
      m_rows()
{
}

const std::string &ListView::source() const
{
    return m_source;
}

float ListView::spacing() const
{
    return m_spacing;
}

std::size_t ListView::maxRows() const
{
    return m_maxRows;
}

const std::vector<ListRowPart> &ListView::parts() const
{
    return m_parts;
}

std::uint64_t ListView::version() const
{
    return m_version;
}

void ListView::setVersion(std::uint64_t version)
{
    m_version = version;
}

const std::vector<Entity> &ListView::rows() const
{
    return m_rows;
}

void ListView::setRows(std::vector<Entity> rows)
{
    m_rows = std::move(rows);
}

} // namespace engine::ecs
