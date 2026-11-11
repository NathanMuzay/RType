#include "DevConsole.hpp"

#include <algorithm>
#include <cstdlib>
#include <sstream>

#include "DebugMetrics.hpp"

namespace engine::client {

DevConsole::DevConsole(GameView &view)
    : m_view(view), m_open(false), m_lines(),
      m_cvars({{"musicVolume", std::to_string(static_cast<int>(view.settings().musicVolume()))},
          {"soundVolume", std::to_string(static_cast<int>(view.settings().soundVolume()))}, {"showMetrics", "0"}})
{
    publish();
}

void DevConsole::toggle()
{
    m_open = !m_open;
    m_view.uiStore().setValue(VISIBLE_KEY, m_open ? "yes" : "no");
    m_view.focusField(m_open ? FIELD : "");
    if (m_open)
        m_view.setFieldText(FIELD, "");
}

bool DevConsole::open() const
{
    return m_open;
}

void DevConsole::print(const std::string &line)
{
    m_lines.push_back(line);
    while (m_lines.size() > LINES)
        m_lines.pop_front();
    publish();
}

std::optional<std::string> DevConsole::submit()
{
    const std::string text = m_view.uiStore().value(std::string("field.") + FIELD);
    m_view.setFieldText(FIELD, "");
    const std::vector<std::string> words = split(text);
    if (words.empty())
        return std::nullopt;
    print("> " + text);
    if (runLocal(words))
        return std::nullopt;
    return text;
}

bool DevConsole::runLocal(const std::vector<std::string> &words)
{
    const std::string &name = words.front();
    if (name == "help") {
        print("local: help, clear, cvars, get <cvar>, set <cvar> <value>, plugins, reload");
        print("anything else is sent to the server (room owner only)");
    } else if (name == "clear") {
        m_lines.clear();
        publish();
    } else if (name == "plugins") {
        const std::vector<std::string> plugins = m_view.pluginList();
        if (plugins.empty())
            print("no plugin loaded");
        for (const std::string &line : plugins)
            print(line);
    } else if (name == "reload") {
        m_view.reloadPlugins();
        print("plugins reloaded");
    } else if (name == "cvars") {
        for (const auto &[cvar, value] : m_cvars)
            print(cvar + " = " + value);
    } else if (name == "get" && words.size() == 2) {
        const auto found = m_cvars.find(words[1]);
        print(found == m_cvars.end() ? "unknown cvar " + words[1] : words[1] + " = " + found->second);
    } else if (name == "set" && words.size() == 3) {
        set(words[1], words[2]);
    } else {
        return false;
    }
    return true;
}

void DevConsole::set(const std::string &name, const std::string &value)
{
    if (!m_cvars.contains(name)) {
        print("unknown cvar " + name);
        return;
    }
    m_cvars[name] = value;
    apply(name, value);
    print(name + " = " + value);
}

void DevConsole::apply(const std::string &name, const std::string &value)
{
    const float number = std::clamp(std::strtof(value.c_str(), nullptr), 0.f, MAX_VOLUME);
    if (name == "musicVolume")
        m_view.setMusicVolume(number);
    else if (name == "soundVolume")
        m_view.setSoundVolume(number);
    else if (name == "showMetrics")
        m_view.uiStore().setValue(DebugMetrics::VISIBLE_KEY, value == "0" ? "no" : "yes");
}

void DevConsole::publish()
{
    for (std::size_t index = 0; index < LINES; ++index)
        m_view.uiStore().setValue("console.line" + std::to_string(index), index < m_lines.size() ? m_lines[index] : "");
}

std::vector<std::string> DevConsole::split(const std::string &text)
{
    std::istringstream stream(text);
    std::vector<std::string> words;
    std::string word;
    while (stream >> word)
        words.push_back(word);
    return words;
}

} // namespace engine::client
