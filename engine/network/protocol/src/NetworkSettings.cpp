#include "NetworkSettings.hpp"

#include <algorithm>
#include <utility>

#include "NetError.hpp"
#include "Protocol.hpp"
#include "RoomInfo.hpp"
#include "SnapshotMessage.hpp"

namespace engine::network {

NetworkSettings NetworkSettings::fromGame(const scene::DataNode &game)
{
    const NetworkSettings settings(game, game.child("network"));
    settings.checkPacketFits(game.path());
    return settings;
}

NetworkSettings::NetworkSettings(const scene::DataNode &game, const scene::DataNode &network)
    : m_defaultPort(static_cast<std::uint16_t>(readBounded(network, "defaultPort", MAX_PORT))),
      m_maxPlayers(static_cast<std::size_t>(readBounded(network, "maxPlayers", MAX_PLAYER_LIMIT))),
      m_maxEntities(static_cast<std::size_t>(readBounded(network, "maxEntities", MAX_ENTITY_LIMIT))),
      m_tickRate(readBounded(game, "tickRate", MAX_COUNT)),
      m_snapshotEveryTicks(static_cast<std::uint32_t>(readBounded(network, "snapshotEveryTicks", MAX_COUNT))),
      m_inputInterval(readDuration(network, "inputIntervalMs")),
      m_inputReleaseDelay(readDuration(network, "inputReleaseDelayMs")),
      m_heartbeatInterval(readDuration(network, "heartbeatIntervalMs")),
      m_connectRetryInterval(readDuration(network, "connectRetryIntervalMs")),
      m_connectAttempts(static_cast<unsigned>(readBounded(network, "connectAttempts", MAX_COUNT))),
      m_sessionTimeout(readDuration(network, "sessionTimeoutMs")),
      m_maxClients(static_cast<std::size_t>(readBounded(network, "maxClients", MAX_CLIENT_LIMIT))),
      m_maxRooms(static_cast<std::size_t>(readBounded(network, "maxRooms", MAX_ROOM_LIMIT))),
      m_roomNameMaxLength(static_cast<std::size_t>(readBounded(network, "roomNameMaxLength", MAX_TEXT_LIMIT))),
      m_roomCodeMaxLength(static_cast<std::size_t>(readBounded(network, "roomCodeMaxLength", MAX_TEXT_LIMIT))),
      m_chatMaxLength(static_cast<std::size_t>(readBounded(network, "chatMaxLength", MAX_CHAT_LIMIT))),
      m_playerNameMaxLength(static_cast<std::size_t>(readBounded(network, "playerNameMaxLength", MAX_TEXT_LIMIT))),
      m_chatInterval(readDuration(network, "chatIntervalMs")),
      m_maxJoinAttempts(static_cast<unsigned>(readBounded(network, "maxJoinAttempts", MAX_COUNT))),
      m_worldValueCount(game.has("worldValues") ? game.child("worldValues").items().size() : 0),
      m_playerValueCount(game.has("playerValues") ? game.child("playerValues").items().size() : 0),
      m_ruleCount(game.has("rules") ? game.child("rules").items().size() : 0),
      m_eventSoundCount(game.has("eventSounds") ? game.child("eventSounds").items().size() : 0),
      m_consoleRule(findRule(game)), m_freeCommands(readFreeCommands(game)), m_ruleCapacities(readRuleCapacities(game))
{
}

std::size_t NetworkSettings::maxClients() const
{
    return m_maxClients;
}

std::size_t NetworkSettings::maxRooms() const
{
    return m_maxRooms;
}

std::size_t NetworkSettings::roomNameMaxLength() const
{
    return m_roomNameMaxLength;
}

std::size_t NetworkSettings::roomCodeMaxLength() const
{
    return m_roomCodeMaxLength;
}

std::size_t NetworkSettings::chatMaxLength() const
{
    return m_chatMaxLength;
}

std::size_t NetworkSettings::playerNameMaxLength() const
{
    return m_playerNameMaxLength;
}

std::chrono::milliseconds NetworkSettings::chatInterval() const
{
    return m_chatInterval;
}

std::size_t NetworkSettings::playerValueCount() const
{
    return m_playerValueCount;
}

std::size_t NetworkSettings::ruleCount() const
{
    return m_ruleCount;
}

std::optional<std::size_t> NetworkSettings::consoleRule() const
{
    return m_consoleRule;
}

std::optional<std::size_t> NetworkSettings::findRule(const scene::DataNode &game)
{
    if (!game.has("consoleRule") || !game.has("rules"))
        return std::nullopt;
    const std::string name = game.getString("consoleRule");
    const auto rules = game.child("rules").items();
    for (std::size_t index = 0; index < rules.size(); ++index) {
        if (rules[index].getString("name") == name)
            return index;
    }
    return std::nullopt;
}

std::size_t NetworkSettings::roomCapacity(const std::vector<std::uint8_t> &rules) const
{
    std::size_t capacity = m_maxPlayers;
    for (std::size_t rule = 0; rule < rules.size() && rule < m_ruleCapacities.size(); ++rule) {
        const std::vector<std::size_t> &seats = m_ruleCapacities[rule];
        if (rules[rule] < seats.size())
            capacity = std::min(capacity, seats[rules[rule]]);
    }
    return capacity;
}

std::vector<std::vector<std::size_t>> NetworkSettings::readRuleCapacities(const scene::DataNode &game) const
{
    std::vector<std::vector<std::size_t>> capacities;
    if (!game.has("rules"))
        return capacities;
    for (const scene::DataNode &rule : game.child("rules").items()) {
        std::vector<std::size_t> seats;
        if (rule.has("maxPlayers")) {
            for (const scene::DataNode &value : rule.child("maxPlayers").items()) {
                const int count = value.asInt();
                if (count <= 0 || static_cast<std::size_t>(count) > m_maxPlayers)
                    throw net::NetError("NetworkSettings: " + rule.path() + ".maxPlayers must be between 1 and "
                        + std::to_string(m_maxPlayers));
                seats.push_back(static_cast<std::size_t>(count));
            }
        }
        capacities.push_back(std::move(seats));
    }
    return capacities;
}

bool NetworkSettings::isFreeCommand(const std::string &text) const
{
    const std::string name = text.substr(0, text.find(' '));
    for (const std::string &command : m_freeCommands) {
        if (command == name)
            return true;
    }
    return false;
}

std::vector<std::string> NetworkSettings::readFreeCommands(const scene::DataNode &game)
{
    std::vector<std::string> commands;
    if (game.has("freeCommands")) {
        for (const scene::DataNode &command : game.child("freeCommands").items())
            commands.push_back(command.asString());
    }
    return commands;
}

std::size_t NetworkSettings::eventSoundCount() const
{
    return m_eventSoundCount;
}

std::size_t NetworkSettings::worldValueCount() const
{
    return m_worldValueCount;
}

unsigned NetworkSettings::maxJoinAttempts() const
{
    return m_maxJoinAttempts;
}

std::uint16_t NetworkSettings::defaultPort() const
{
    return m_defaultPort;
}

std::size_t NetworkSettings::maxPlayers() const
{
    return m_maxPlayers;
}

std::size_t NetworkSettings::maxEntities() const
{
    return m_maxEntities;
}

int NetworkSettings::tickRate() const
{
    return m_tickRate;
}

std::uint32_t NetworkSettings::snapshotEveryTicks() const
{
    return m_snapshotEveryTicks;
}

std::chrono::milliseconds NetworkSettings::inputInterval() const
{
    return m_inputInterval;
}

std::chrono::milliseconds NetworkSettings::inputReleaseDelay() const
{
    return m_inputReleaseDelay;
}

std::chrono::milliseconds NetworkSettings::heartbeatInterval() const
{
    return m_heartbeatInterval;
}

std::chrono::milliseconds NetworkSettings::connectRetryInterval() const
{
    return m_connectRetryInterval;
}

unsigned NetworkSettings::connectAttempts() const
{
    return m_connectAttempts;
}

std::chrono::milliseconds NetworkSettings::sessionTimeout() const
{
    return m_sessionTimeout;
}

void NetworkSettings::checkPacketFits(const std::string &source) const
{
    if (Protocol::HEADER_SIZE
            + SnapshotMessage::sizeFor(m_maxPlayers, m_maxEntities, m_worldValueCount, m_playerValueCount)
        > Protocol::MAX_PACKET_BYTES)
        throw net::NetError("NetworkSettings: " + source + ": maxPlayers and maxEntities do not fit in one packet");
    const std::size_t roomList = 1 + m_maxRooms * RoomInfo::sizeFor(m_roomNameMaxLength);
    if (Protocol::HEADER_SIZE + roomList > Protocol::MAX_PACKET_BYTES)
        throw net::NetError("NetworkSettings: " + source + ": maxRooms and roomNameMaxLength do not fit in one packet");
}

int NetworkSettings::readBounded(const scene::DataNode &node, const std::string &key, int maximum)
{
    const int value = node.getInt(key);
    if (value <= 0 || value > maximum)
        throw net::NetError(
            "NetworkSettings: " + node.path() + "." + key + " must be between 1 and " + std::to_string(maximum));
    return value;
}

std::chrono::milliseconds NetworkSettings::readDuration(const scene::DataNode &node, const std::string &key)
{
    return std::chrono::milliseconds(readBounded(node, key, MAX_COUNT));
}

} // namespace engine::network
