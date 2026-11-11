#pragma once

#include <string>

#include <pybind11/embed.h>

namespace engine::scripting {

class ScriptRecord
{
public:
    ScriptRecord(std::string file, pybind11::object module, pybind11::object handle);

    const std::string &file() const;
    const pybind11::object &module() const;
    const pybind11::object &handle() const;
    bool spawned() const;
    void markSpawned();
    bool failed() const;
    void markFailed();
    int lastHealth() const;
    void setLastHealth(int health);

private:
    std::string m_file;
    pybind11::object m_module;
    pybind11::object m_handle;
    bool m_spawned;
    bool m_failed;
    int m_lastHealth;
};

} // namespace engine::scripting
