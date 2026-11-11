#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include <pybind11/embed.h>

namespace engine::scripting {

class ScriptEngineImpl
{
public:
    static constexpr const char *MODULE_NAME = "engine";
    static constexpr const char *SCRIPT_EXTENSION = ".py";

    explicit ScriptEngineImpl(const std::filesystem::path &pythonHome);
    ~ScriptEngineImpl();

    pybind11::module_ load(const std::filesystem::path &directory, const std::string &file);
    static void report(const std::string &file, const std::string &hook, const std::exception &error);

private:
    static std::unique_ptr<pybind11::scoped_interpreter> startInterpreter(const std::filesystem::path &pythonHome);
    void bindApi();
    void addSearchPath(const std::filesystem::path &directory);

    std::unique_ptr<pybind11::scoped_interpreter> m_interpreter;
    std::unordered_map<std::string, pybind11::module_> m_modules;
};

} // namespace engine::scripting
