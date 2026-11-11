#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "IGameSession.hpp"
#include "ISessionFactory.hpp"
#include "NetworkSettings.hpp"
#include "ScriptEngine.hpp"

namespace engine::simulation {

class SimulationFactory : public network::ISessionFactory
{
public:
    SimulationFactory(std::filesystem::path dataDirectory, std::string game, network::NetworkSettings settings,
        scripting::ScriptEngine &scripts);

    std::unique_ptr<network::IGameSession> createSession(const std::vector<std::uint8_t> &rules) override;

private:
    std::filesystem::path m_dataDirectory;
    std::string m_game;
    network::NetworkSettings m_settings;
    scripting::ScriptEngine &m_scripts;
};

} // namespace engine::simulation
