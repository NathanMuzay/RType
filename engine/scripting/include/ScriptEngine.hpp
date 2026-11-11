#pragma once

#include <filesystem>
#include <memory>

namespace engine::scripting {

class ScriptEngineImpl;

class ScriptEngine
{
public:
    explicit ScriptEngine(const std::filesystem::path &pythonHome);
    ~ScriptEngine();

    ScriptEngine(const ScriptEngine &) = delete;
    ScriptEngine &operator=(const ScriptEngine &) = delete;

    ScriptEngineImpl &impl();

private:
    std::unique_ptr<ScriptEngineImpl> m_impl;
};

} // namespace engine::scripting
