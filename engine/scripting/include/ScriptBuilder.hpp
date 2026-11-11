#pragma once

#include <filesystem>
#include <memory>

#include "DataNode.hpp"
#include "ISystem.hpp"
#include "ScriptEngine.hpp"
#include "SystemContext.hpp"
#include "SystemFactory.hpp"

namespace engine::scripting {

class ScriptBuilder
{
public:
    static constexpr const char *SYSTEM_NAME = "Script";

    ScriptBuilder(ScriptEngine &engine, std::filesystem::path directory);

    void registerInto(scene::SystemFactory &factory);
    std::unique_ptr<ecs::ISystem> build(scene::SystemContext &context, const scene::DataNode &entry);

private:
    ScriptEngine &m_engine;
    std::filesystem::path m_directory;
};

} // namespace engine::scripting
