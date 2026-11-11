#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "DataWriter.hpp"

namespace engine::scene {

class DataWriter::Impl
{
public:
    static constexpr int INDENT = 2;

    nlohmann::json &slot(const std::string &section, const std::string &key);
    const nlohmann::json &root() const;

private:
    nlohmann::json m_root = nlohmann::json::object();
};

} // namespace engine::scene
