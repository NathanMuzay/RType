#include "CommandLine.hpp"

#include "CoreError.hpp"
#include "PortOption.hpp"

namespace engine {

CommandLine::CommandLine(int argc, char **argv, std::string_view usage, bool acceptsHost, std::string_view defaultGame)
    : m_host(), m_port(), m_game(defaultGame)
{
    for (int i = 1; i < argc; ++i) {
        const std::string_view option(argv[i]);
        const bool known = option == "-p" || option == "-g" || (acceptsHost && option == "-h");
        if (!known || i + 1 >= argc)
            throw CoreError(std::string(usage));
        const std::string value = argv[++i];
        if (option == "-p")
            m_port = PortOption::parse(value);
        else if (option == "-g")
            m_game = value;
        else
            m_host = value;
    }
    if (m_game.empty() || m_game.find_first_of("/\\.") != std::string::npos)
        throw CoreError("CommandLine: invalid game name " + m_game);
}

const std::optional<std::string> &CommandLine::host() const
{
    return m_host;
}

std::optional<std::uint16_t> CommandLine::port() const
{
    return m_port;
}

const std::string &CommandLine::game() const
{
    return m_game;
}

} // namespace engine
