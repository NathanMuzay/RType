#include "ScriptRecord.hpp"

#include <utility>

namespace engine::scripting {

ScriptRecord::ScriptRecord(std::string file, pybind11::object module, pybind11::object handle)
    : m_file(std::move(file)), m_module(std::move(module)), m_handle(std::move(handle)), m_spawned(false),
      m_failed(false), m_lastHealth(0)
{
}

const std::string &ScriptRecord::file() const
{
    return m_file;
}

const pybind11::object &ScriptRecord::module() const
{
    return m_module;
}

const pybind11::object &ScriptRecord::handle() const
{
    return m_handle;
}

bool ScriptRecord::spawned() const
{
    return m_spawned;
}

void ScriptRecord::markSpawned()
{
    m_spawned = true;
}

bool ScriptRecord::failed() const
{
    return m_failed;
}

void ScriptRecord::markFailed()
{
    m_failed = true;
}

int ScriptRecord::lastHealth() const
{
    return m_lastHealth;
}

void ScriptRecord::setLastHealth(int health)
{
    m_lastHealth = health;
}

} // namespace engine::scripting
