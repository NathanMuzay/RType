#include "OnlineClient.hpp"

#include <cstddef>
#include <utility>
#include <vector>

#include "Action.hpp"
#include "EngineError.hpp"
#include "PortOption.hpp"
#include "Protocol.hpp"
#include "TextRules.hpp"
#include "UdpTransport.hpp"

namespace engine::client {

void OnlineClient::handleRejection(std::uint8_t reason)
{
    static constexpr const char *KEYS[] = {"", "roomNotFound", "roomFull", "wrongCode", "tooManyRooms",
        "invalidRoomName", "tooManyAttempts", "notAllowed"};
    const std::string key = reason < std::size(KEYS) ? KEYS[reason] : "notAllowed";
    std::string target = m_view.currentScene() == ROOM_SCENE ? "room.error" : "lobby.error";
    if (popupOpen())
        target = "lobby.popupError";
    m_view.uiStore().setValue(target, m_view.message(key));
}

void OnlineClient::createRoom()
{
    const std::string name = field("roomName");
    const std::string code = field("roomCode");
    if (!network::TextRules::isValid(name, 1, m_settings.roomNameMaxLength())
        || !network::TextRules::isValid(code, 0, m_settings.roomCodeMaxLength())) {
        m_view.uiStore().setValue("lobby.error", m_view.message("invalidRoomName"));
        return;
    }
    m_network->createRoom(name, code, ruleValues());
}

void OnlineClient::joinOrAsk(std::uint16_t roomId)
{
    if (m_lockedRooms.contains(roomId))
        openPopup(roomId);
    else
        m_network->joinRoom(roomId, {});
}

void OnlineClient::openPopup(std::uint16_t roomId)
{
    m_pendingRoom = roomId;
    m_view.setFieldText("joinCode", {});
    m_view.uiStore().setValue("lobby.popupError", {});
    m_view.uiStore().setValue("lobby.popup", YES);
}

void OnlineClient::closePopup()
{
    m_view.uiStore().setValue("lobby.popup", NO);
    m_view.uiStore().setValue("lobby.popupError", {});
}

bool OnlineClient::popupOpen()
{
    return m_view.currentScene() == LOBBY_SCENE && m_view.uiStore().value("lobby.popup") == YES;
}

void OnlineClient::showRoomList(const network::RoomListMessage &list)
{
    std::vector<ecs::UiStore::Row> rows;
    m_lockedRooms.clear();
    for (const network::RoomInfo &room : list.rooms()) {
        if (room.locked())
            m_lockedRooms.insert(room.id());
        rows.push_back({{"id", std::to_string(room.id())}, {"name", room.name()},
            {"players", std::to_string(room.players()) + "/" + std::to_string(room.capacity())},
            {"lock", room.locked() ? m_view.message("locked") : std::string()},
            {"status", m_view.message(room.playing() ? "playing" : "waiting")}});
    }
    m_view.uiStore().setValue("lobby.empty", rows.empty() ? m_view.message("noRooms") : std::string());
    m_view.uiStore().setList("rooms", std::move(rows));
}

void OnlineClient::showMembers(const network::RoomUpdateMessage &update)
{
    const bool owner = m_slot && *m_slot == update.ownerSlot();
    std::vector<ecs::UiStore::Row> rows;
    for (std::size_t slot = 0; slot < update.occupied().size(); ++slot) {
        const bool occupied = update.occupied()[slot];
        std::string tag;
        if (occupied && slot == update.ownerSlot())
            tag = m_view.message("ownerTag");
        if (occupied && m_slot && slot == *m_slot)
            tag += m_view.message("youTag");
        const bool canKick = owner && occupied && slot != *m_slot;
        rows.push_back({{"slot", std::to_string(slot)}, {"number", std::to_string(slot + 1)},
            {"name", occupied ? playerName(update, slot) : m_view.message("emptySlot")}, {"tag", tag},
            {"canKick", canKick ? YES : NO}});
    }
    m_view.uiStore().setValue("room.isOwner", owner ? YES : NO);
    m_view.uiStore().setList("members", std::move(rows));
}

std::vector<std::uint8_t> OnlineClient::ruleValues()
{
    const scene::WorldValueTable &rules = m_view.ruleTable();
    std::vector<std::uint8_t> values;
    for (std::size_t index = 0; index < rules.size(); ++index) {
        const auto value = rules.valueOf(index, field(RULE_PREFIX + rules.name(index)));
        values.push_back(static_cast<std::uint8_t>(value.value_or(0)));
    }
    return values;
}

void OnlineClient::sendChat()
{
    const std::string text = field("chatInput");
    if (text.empty())
        return;
    m_network->sendChat(text);
    m_view.setFieldText("chatInput", {});
}

void OnlineClient::showChat(const network::ChatBroadcastMessage &message)
{
    const bool mine = m_slot && *m_slot == message.slot();
    const std::string author = message.slot() < m_names.size() && !m_names[message.slot()].empty()
        ? m_names[message.slot()]
        : "P" + std::to_string(message.slot() + 1);
    m_chat.push_back({{"author", author}, {"text", message.text()}, {"mine", mine ? YES : NO}});
    if (m_chat.size() > CHAT_HISTORY)
        m_chat.pop_front();
    m_view.uiStore().setList("chat", std::vector<ecs::UiStore::Row>(m_chat.begin(), m_chat.end()));
}

void OnlineClient::clearChat()
{
    m_chat.clear();
    m_view.uiStore().setList("chat", {});
    m_view.setFieldText("chatInput", {});
}

std::string OnlineClient::playerName(const network::RoomUpdateMessage &update, std::size_t slot) const
{
    if (slot < update.names().size() && !update.names()[slot].empty())
        return update.names()[slot];
    return "P" + std::to_string(slot + 1);
}

} // namespace engine::client
