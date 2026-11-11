#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "Entity.hpp"
#include "IComponent.hpp"
#include "ListRowPart.hpp"

namespace engine::ecs {

class ListView : public IComponent
{
public:
    ListView(std::string source, float spacing, std::size_t maxRows, std::vector<ListRowPart> parts);

    const std::string &source() const;
    float spacing() const;
    std::size_t maxRows() const;
    const std::vector<ListRowPart> &parts() const;
    std::uint64_t version() const;
    void setVersion(std::uint64_t version);
    const std::vector<Entity> &rows() const;
    void setRows(std::vector<Entity> rows);

private:
    std::string m_source;
    float m_spacing;
    std::size_t m_maxRows;
    std::vector<ListRowPart> m_parts;
    std::uint64_t m_version;
    std::vector<Entity> m_rows;
};

} // namespace engine::ecs
