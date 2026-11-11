#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "GameView.hpp"
#include "Match.hpp"
#include "PlayerStatus.hpp"
#include "SnapshotMessage.hpp"

namespace engine::client {

class SnapshotPresenter
{
public:
    static constexpr const char *YES = "yes";
    static constexpr const char *NO = "no";

    explicit SnapshotPresenter(GameView &view);

    void apply(const network::SnapshotMessage &snapshot);

private:
    void showWorld(const network::SnapshotMessage &snapshot);
    void showPlayer(std::size_t slot, const network::PlayerStatus &player);
    static std::string ratio(std::uint8_t percent);
    static std::string phaseName(std::uint8_t gameState);
    static ecs::Match::Phase toPhase(std::uint8_t gameState);

    GameView &m_view;
};

} // namespace engine::client
