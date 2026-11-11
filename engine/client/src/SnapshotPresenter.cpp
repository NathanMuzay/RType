#include "SnapshotPresenter.hpp"

#include "Protocol.hpp"

namespace engine::client {

SnapshotPresenter::SnapshotPresenter(GameView &view) : m_view(view)
{
}

void SnapshotPresenter::apply(const network::SnapshotMessage &snapshot)
{
    m_view.beginSync();
    for (const network::EntityState &entity : snapshot.entities())
        m_view.syncEntity(entity.id(), entity.kind(), static_cast<float>(entity.x()), static_cast<float>(entity.y()),
            entity.visual());
    m_view.endSync();
    m_view.setPhase(toPhase(snapshot.gameState()));
    showWorld(snapshot);
    for (std::size_t slot = 0; slot < snapshot.players().size(); ++slot) {
        const network::PlayerStatus &player = snapshot.players()[slot];
        showPlayer(slot, player);
        if (player.state() == network::Protocol::SLOT_EMPTY)
            m_view.removePlayer(slot);
        else
            m_view.setPlayer(slot, player.lives(), player.score(), player.avatarId());
    }
}

void SnapshotPresenter::showWorld(const network::SnapshotMessage &snapshot)
{
    ecs::UiStore &store = m_view.uiStore();
    const scene::WorldValueTable &table = m_view.worldTable();
    const auto &values = snapshot.values();
    for (std::size_t index = 0; index < values.size() && index < table.size(); ++index)
        store.setValue("world." + table.name(index), table.label(index, values[index]));
    store.setValue("game.phase", phaseName(snapshot.gameState()));
}

void SnapshotPresenter::showPlayer(std::size_t slot, const network::PlayerStatus &player)
{
    ecs::UiStore &store = m_view.uiStore();
    const std::string prefix = "player" + std::to_string(slot) + ".";
    store.setValue(prefix + "present", player.state() == network::Protocol::SLOT_EMPTY ? NO : YES);
    store.setValue(prefix + "alive", player.state() == network::Protocol::SLOT_ALIVE ? YES : NO);
    store.setValue(prefix + "health", ratio(player.health()));
    store.setValue(prefix + "score", std::to_string(player.score()));
    const scene::WorldValueTable &table = m_view.playerTable();
    for (std::size_t index = 0; index < player.values().size() && index < table.size(); ++index)
        store.setValue(prefix + table.name(index), table.label(index, player.values()[index]));
}

std::string SnapshotPresenter::ratio(std::uint8_t percent)
{
    return std::to_string(static_cast<float>(percent) / static_cast<float>(network::Protocol::MAX_PERCENT));
}

std::string SnapshotPresenter::phaseName(std::uint8_t gameState)
{
    if (gameState == network::Protocol::GAME_PLAYING)
        return "playing";
    if (gameState == network::Protocol::GAME_OVER)
        return "over";
    return "waiting";
}

ecs::Match::Phase SnapshotPresenter::toPhase(std::uint8_t gameState)
{
    if (gameState == network::Protocol::GAME_PLAYING)
        return ecs::Match::Phase::Playing;
    if (gameState == network::Protocol::GAME_OVER)
        return ecs::Match::Phase::Over;
    return ecs::Match::Phase::Waiting;
}

} // namespace engine::client
