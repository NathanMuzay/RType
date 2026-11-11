#pragma once

#include <cstddef>
#include <deque>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "GameView.hpp"

namespace engine::client {

class DevConsole
{
public:
    static constexpr std::size_t LINES = 8;
    static constexpr const char *FIELD = "console";
    static constexpr const char *VISIBLE_KEY = "console.visible";
    static constexpr float MAX_VOLUME = 100.f;

    explicit DevConsole(GameView &view);

    void toggle();
    bool open() const;
    void print(const std::string &line);
    std::optional<std::string> submit();

private:
    bool runLocal(const std::vector<std::string> &words);
    void set(const std::string &name, const std::string &value);
    void apply(const std::string &name, const std::string &value);
    void publish();
    static std::vector<std::string> split(const std::string &text);

    GameView &m_view;
    bool m_open;
    std::deque<std::string> m_lines;
    std::map<std::string, std::string> m_cvars;
};

} // namespace engine::client
