#include "SimulationFactory.hpp"

#include <utility>

#include "Simulation.hpp"

namespace engine::simulation {

SimulationFactory::SimulationFactory(std::filesystem::path dataDirectory, std::string game,
    network::NetworkSettings settings, scripting::ScriptEngine &scripts)
    : m_dataDirectory(std::move(dataDirectory)), m_game(std::move(game)), m_settings(std::move(settings)),
      m_scripts(scripts)
{
}

std::unique_ptr<network::IGameSession> SimulationFactory::createSession(const std::vector<std::uint8_t> &rules)
{
    return std::make_unique<Simulation>(m_dataDirectory, m_game, m_settings, m_scripts, rules);
}

} // namespace engine::simulation
