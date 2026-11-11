#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace engine {

class CommandLine
{
public:
    CommandLine(int argc, char **argv, std::string_view usage, bool acceptsHost, std::string_view defaultGame);

    const std::optional<std::string> &host() const;
    std::optional<std::uint16_t> port() const;
    const std::string &game() const;

private:
    std::optional<std::string> m_host;
    std::optional<std::uint16_t> m_port;
    std::string m_game;
};

} // namespace engine
