#include "ScriptEngineImpl.hpp"

#include <iostream>

#include <pybind11/stl.h>

#include "ScriptEntity.hpp"
#include "ScriptError.hpp"
#include "ScriptWorld.hpp"

namespace py = pybind11;

namespace engine::scripting {

ScriptEngineImpl::ScriptEngineImpl(const std::filesystem::path &pythonHome)
    : m_interpreter(startInterpreter(pythonHome)), m_modules()
{
    bindApi();
}

ScriptEngineImpl::~ScriptEngineImpl()
{
    m_modules.clear();
}

std::unique_ptr<py::scoped_interpreter> ScriptEngineImpl::startInterpreter(const std::filesystem::path &pythonHome)
{
    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    config.install_signal_handlers = 0;
    if (!pythonHome.empty() && std::filesystem::exists(pythonHome)) {
        const PyStatus status = PyConfig_SetBytesString(&config, &config.home, pythonHome.string().c_str());
        if (PyStatus_Exception(status)) {
            PyConfig_Clear(&config);
            throw ScriptError("ScriptEngine: invalid Python home " + pythonHome.string());
        }
    }
    try {
        auto interpreter = std::make_unique<py::scoped_interpreter>(&config, 0, nullptr, false);
        PyConfig_Clear(&config);
        return interpreter;
    } catch (const std::exception &error) {
        throw ScriptError(std::string("ScriptEngine: the Python interpreter could not start: ") + error.what());
    }
}

pybind11::module_ ScriptEngineImpl::load(const std::filesystem::path &directory, const std::string &file)
{
    const std::filesystem::path path = directory / file;
    const std::string key = path.lexically_normal().string();
    const auto found = m_modules.find(key);
    if (found != m_modules.end())
        return found->second;
    if (!std::filesystem::exists(path))
        throw ScriptError("ScriptEngine: script not found " + key);
    addSearchPath(directory);
    std::string name = path.stem().string();
    try {
        py::module_ module = py::module_::import(name.c_str());
        m_modules.emplace(key, module);
        return module;
    } catch (const py::error_already_set &error) {
        throw ScriptError("ScriptEngine: cannot load " + key + ": " + error.what());
    }
}

void ScriptEngineImpl::report(const std::string &file, const std::string &hook, const std::exception &error)
{
    std::cerr << "[script] " << file << " " << hook << " failed, script disabled for this entity:\n"
              << error.what() << std::endl;
}

void ScriptEngineImpl::addSearchPath(const std::filesystem::path &directory)
{
    py::list path = py::module_::import("sys").attr("path");
    const std::string text = std::filesystem::absolute(directory).lexically_normal().string();
    for (const py::handle entry : path) {
        if (py::str(entry).cast<std::string>() == text)
            return;
    }
    path.insert(0, text);
}

} // namespace engine::scripting
