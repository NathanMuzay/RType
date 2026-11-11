#include "ScriptEngine.hpp"

#include "ScriptEngineImpl.hpp"

namespace engine::scripting {

ScriptEngine::ScriptEngine(const std::filesystem::path &pythonHome)
    : m_impl(std::make_unique<ScriptEngineImpl>(pythonHome))
{
}

ScriptEngine::~ScriptEngine() = default;

ScriptEngineImpl &ScriptEngine::impl()
{
    return *m_impl;
}

} // namespace engine::scripting
