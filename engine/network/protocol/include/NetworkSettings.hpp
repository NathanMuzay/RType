#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "DataNode.hpp"

namespace engine::network {

class NetworkSettings
{
public:
    static constexpr int MAX_PORT = 65535;
    static constexpr int MAX_PLAYER_LIMIT = 255;
    static constexpr int MAX_ENTITY_LIMIT = 255;
    static constexpr int MAX_COUNT = 1000000;
    static constexpr int MAX_CLIENT_LIMIT = 255;
    static constexpr int MAX_ROOM_LIMIT = 255;
    static constexpr int MAX_TEXT_LIMIT = 32;
    static constexpr int MAX_CHAT_LIMIT = 128;

    static NetworkSettings fromGame(const scene::DataNode &game);

    std::uint16_t defaultPort() const;
    std::size_t maxPlayers() const;
    std::size_t maxEntities() const;
    int tickRate() const;
    std::uint32_t snapshotEveryTicks() const;
    std::chrono::milliseconds inputInterval() const;
    std::chrono::milliseconds inputReleaseDelay() const;
    std::chrono::milliseconds heartbeatInterval() const;
    std::chrono::milliseconds connectRetryInterval() const;
    unsigned connectAttempts() const;
    std::chrono::milliseconds sessionTimeout() const;
    std::size_t maxClients() const;
    std::size_t maxRooms() const;
    std::size_t roomNameMaxLength() const;
    std::size_t roomCodeMaxLength() const;
    std::size_t chatMaxLength() const;
    std::size_t playerNameMaxLength() const;
    std::chrono::milliseconds chatInterval() const;
    unsigned maxJoinAttempts() const;
    std::size_t worldValueCount() const;
    std::size_t playerValueCount() const;
    std::size_t ruleCount() const;
    std::size_t roomCapacity(const std::vector<std::uint8_t> &rules) const;
    std::size_t eventSoundCount() const;
    std::optional<std::size_t> consoleRule() const;
    bool isFreeCommand(const std::string &text) const;

private:
    static std::optional<std::size_t> findRule(const scene::DataNode &game);
    static std::vector<std::string> readFreeCommands(const scene::DataNode &game);
    std::vector<std::vector<std::size_t>> readRuleCapacities(const scene::DataNode &game) const;
    NetworkSettings(const scene::DataNode &game, const scene::DataNode &network);

    void checkPacketFits(const std::string &source) const;
    static int readBounded(const scene::DataNode &node, const std::string &key, int maximum);
    static std::chrono::milliseconds readDuration(const scene::DataNode &node, const std::string &key);

    std::uint16_t m_defaultPort;
    std::size_t m_maxPlayers;
    std::size_t m_maxEntities;
    int m_tickRate;
    std::uint32_t m_snapshotEveryTicks;
    std::chrono::milliseconds m_inputInterval;
    std::chrono::milliseconds m_inputReleaseDelay;
    std::chrono::milliseconds m_heartbeatInterval;
    std::chrono::milliseconds m_connectRetryInterval;
    unsigned m_connectAttempts;
    std::chrono::milliseconds m_sessionTimeout;
    std::size_t m_maxClients;
    std::size_t m_maxRooms;
    std::size_t m_roomNameMaxLength;
    std::size_t m_roomCodeMaxLength;
    std::size_t m_chatMaxLength;
    std::size_t m_playerNameMaxLength;
    std::chrono::milliseconds m_chatInterval;
    unsigned m_maxJoinAttempts;
    std::size_t m_worldValueCount;
    std::size_t m_playerValueCount;
    std::size_t m_ruleCount;
    std::size_t m_eventSoundCount;
    std::optional<std::size_t> m_consoleRule;
    std::vector<std::string> m_freeCommands;
    std::vector<std::vector<std::size_t>> m_ruleCapacities;
};

} // namespace engine::network
