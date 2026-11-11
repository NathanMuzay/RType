#include "GameRoom.hpp"

#include <utility>

#include "NetError.hpp"

namespace engine::network {

GameRoom::GameRoom(std::uint16_t id, std::string name, std::string code, std::size_t capacity,
    std::unique_ptr<IGameSession> session, std::vector<std::uint8_t> rules)
    : m_id(id), m_name(std::move(name)), m_code(std::move(code)), m_playing(false), m_session(std::move(session)),
      m_seats(capacity), m_commands(), m_rules(std::move(rules))
{
    if (!m_session)
        throw net::NetError("GameRoom: a room needs a game session");
}

std::uint16_t GameRoom::id() const
{
    return m_id;
}

const std::string &GameRoom::name() const
{
    return m_name;
}

bool GameRoom::locked() const
{
    return !m_code.empty();
}

bool GameRoom::acceptsCode(const std::string &code) const
{
    return m_code.empty() || m_code == code;
}

bool GameRoom::playing() const
{
    return m_playing;
}

void GameRoom::setPlaying(bool playing)
{
    m_playing = playing;
}

IGameSession &GameRoom::session()
{
    return *m_session;
}

void GameRoom::resetSession(std::unique_ptr<IGameSession> session)
{
    m_session = std::move(session);
}

const std::vector<std::uint8_t> &GameRoom::rules() const
{
    return m_rules;
}

std::size_t GameRoom::capacity() const
{
    return m_seats.size();
}

std::size_t GameRoom::playerCount() const
{
    std::size_t count = 0;
    for (const auto &seat : m_seats)
        count += seat.has_value();
    return count;
}

std::optional<std::size_t> GameRoom::freeSlot() const
{
    for (std::size_t slot = 0; slot < m_seats.size(); ++slot) {
        if (!m_seats[slot])
            return slot;
    }
    return std::nullopt;
}

void GameRoom::seat(std::size_t slot, std::size_t client)
{
    m_seats.at(slot) = client;
}

void GameRoom::unseat(std::size_t slot)
{
    m_seats.at(slot).reset();
}

std::optional<std::size_t> GameRoom::clientAt(std::size_t slot) const
{
    return slot < m_seats.size() ? m_seats[slot] : std::nullopt;
}

std::optional<std::size_t> GameRoom::slotOf(std::size_t client) const
{
    for (std::size_t slot = 0; slot < m_seats.size(); ++slot) {
        if (m_seats[slot] == client)
            return slot;
    }
    return std::nullopt;
}

std::vector<bool> GameRoom::occupiedSlots() const
{
    std::vector<bool> occupied;
    for (const auto &seat : m_seats)
        occupied.push_back(seat.has_value());
    return occupied;
}

std::vector<std::size_t> GameRoom::clients() const
{
    std::vector<std::size_t> clients;
    for (const auto &seat : m_seats) {
        if (seat)
            clients.push_back(*seat);
    }
    return clients;
}

std::size_t GameRoom::owner() const
{
    return OWNER_SLOT;
}

void GameRoom::queueCommand(std::size_t slot, std::string text)
{
    m_commands.emplace_back(slot, std::move(text));
}

std::vector<std::pair<std::size_t, std::string>> GameRoom::takeCommands()
{
    return std::exchange(m_commands, {});
}

std::uint8_t GameRoom::rule(std::size_t index) const
{
    return index < m_rules.size() ? m_rules[index] : 0;
}

} // namespace engine::network
