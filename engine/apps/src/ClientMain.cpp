#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

#include "AssetPath.hpp"
#include "CommandLine.hpp"
#include "DataNode.hpp"
#include "EngineError.hpp"
#include "NetworkSettings.hpp"
#include "OnlineClient.hpp"

int main(int argc, char **argv)
{
    constexpr std::string_view usage = "Usage: " ENGINE_PROGRAM_NAME " [-h host] [-p port] [-g game]";
    constexpr std::string_view game = ENGINE_DEFAULT_GAME;

    if (argc == 2 && std::string_view(argv[1]) == "--help") {
        std::cout << usage << std::endl;
        return 0;
    }
    try {
        const engine::CommandLine options(argc, argv, usage, true, game);
        const std::filesystem::path data = engine::AssetPath::findData(argv[0]);
        const auto settings = engine::network::NetworkSettings::fromGame(
            engine::scene::DataNode::fromFile(data / (options.game() + "_game.json")));
        std::optional<std::string> address = options.host();
        if (address && options.port())
            *address += ":" + std::to_string(*options.port());
        engine::client::OnlineClient client(
            engine::AssetPath::findAssets(argv[0]), data, options.game(), settings, address);
        client.run();
    } catch (const engine::EngineError &error) {
        std::cerr << ENGINE_PROGRAM_NAME ": " << error.what() << std::endl;
        return 84;
    } catch (const std::exception &error) {
        std::cerr << ENGINE_PROGRAM_NAME ": unexpected error: " << error.what() << std::endl;
        return 84;
    }
    return 0;
}
