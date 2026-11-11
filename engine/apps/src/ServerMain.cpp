#include <exception>
#include <filesystem>
#include <iostream>
#include <string_view>

#include "AssetPath.hpp"
#include "CommandLine.hpp"
#include "DataNode.hpp"
#include "EngineError.hpp"
#include "GameServer.hpp"
#include "NetworkSettings.hpp"
#include "ScriptEngine.hpp"
#include "ServerRunner.hpp"
#include "SimulationFactory.hpp"

int main(int argc, char **argv)
{
    constexpr std::string_view usage = "Usage: " ENGINE_PROGRAM_NAME " [-p port] [-g game]";
    constexpr std::string_view game = ENGINE_DEFAULT_GAME;

    if (argc == 2 && std::string_view(argv[1]) == "--help") {
        std::cout << usage << std::endl;
        return 0;
    }
    try {
        const engine::CommandLine options(argc, argv, usage, false, game);
        const std::filesystem::path data = engine::AssetPath::findData(argv[0]);
        const auto settings = engine::network::NetworkSettings::fromGame(
            engine::scene::DataNode::fromFile(data / (options.game() + "_game.json")));
        engine::scripting::ScriptEngine scripts(std::filesystem::path(argv[0]).parent_path() / "python");
        engine::simulation::SimulationFactory factory(data, options.game(), settings, scripts);
        engine::network::GameServer server(options.port().value_or(settings.defaultPort()), settings, factory);
        engine::network::ServerRunner::run(server, settings.tickRate());
    } catch (const engine::EngineError &error) {
        std::cerr << ENGINE_PROGRAM_NAME ": " << error.what() << std::endl;
        return 84;
    } catch (const std::exception &error) {
        std::cerr << ENGINE_PROGRAM_NAME ": unexpected error: " << error.what() << std::endl;
        return 84;
    }
    return 0;
}
