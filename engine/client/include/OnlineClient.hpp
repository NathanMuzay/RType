#pragma once

#include <chrono>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "ClientConnection.hpp"
#include "DebugMetrics.hpp"
#include "DevConsole.hpp"
#include "GameView.hpp"
#include "Match.hpp"
#include "NetworkSettings.hpp"
#include "Packet.hpp"
#include "RoomListMessage.hpp"
#include "RoomUpdateMessage.hpp"
#include "SecretCode.hpp"
#include "SnapshotMessage.hpp"
#include "SnapshotPresenter.hpp"

namespace engine::client {

class OnlineClient
{
public:
    static constexpr std::chrono::seconds ROOM_LIST_INTERVAL{2};
    static constexpr std::chrono::seconds ROOM_SILENCE_LIMIT{4};
    static constexpr std::chrono::seconds NOTICE_DURATION{4};
    static constexpr const char *NOTICE_KEY = "game.notice";
    static constexpr std::chrono::milliseconds BOMB_HOLD{300};
    static constexpr const char *MENU_SCENE = "menu";
    static constexpr const char *LOBBY_SCENE = "lobby";
    static constexpr const char *ROOM_SCENE = "room";
    static constexpr const char *GAME_SCENE = "game";
    static constexpr const char *SETTINGS_SCENE = "settings";
    static constexpr const char *CAPTURING_KEY = "settings.capturing";
    static constexpr const char *YES = "yes";
    static constexpr const char *NO = "no";
    static constexpr const char *RULE_PREFIX = "rule.";
    static constexpr const char *PAUSE_KEY = "pause.visible";
    static constexpr const char *COMMAND_PREFIX = "command:";
    static constexpr std::size_t CHAT_HISTORY = 5;

    OnlineClient(const std::filesystem::path &assetsDirectory, const std::filesystem::path &dataDirectory,
        const std::string &game, const network::NetworkSettings &settings, std::optional<std::string> address);

    void run();

private:
    using Time = std::chrono::steady_clock::time_point;

    void updateNetwork(Time now);
    void watchSecretCode(Time now);
    void handleActions();
    void handleConsole();
    void setPaused(bool paused);
    void openSettings();
    void restartGame();
    void closeSettings();
    bool paused();
    void handleAction(const std::string &action);
    void handleEvent(const network::Packet &event, Time now);
    void handleRejection(std::uint8_t reason);
    void showPlayerLeft(const network::PlayerLeftMessage &left, Time now);
    void expireNotice(Time now);
    void createRoom();
    std::vector<std::uint8_t> ruleValues();
    void joinOrAsk(std::uint16_t roomId);
    void openPopup(std::uint16_t roomId);
    void closePopup();
    bool popupOpen();
    void connect();
    void disconnect(const std::string &errorKey);
    void enterScene(const std::string &scene);
    void showRoomList(const network::RoomListMessage &list);
    void showMembers(const network::RoomUpdateMessage &update);
    std::string playerName(const network::RoomUpdateMessage &update, std::size_t slot) const;
    void sendChat();
    void showChat(const network::ChatBroadcastMessage &message);
    void clearChat();
    void applySnapshot(const network::SnapshotMessage &snapshot);
    std::uint8_t buttons() const;
    std::string field(const std::string &name);
    static bool startsWith(const std::string &text, const std::string &prefix);

    GameView m_view;
    SnapshotPresenter m_presenter;
    network::NetworkSettings m_settings;
    std::unique_ptr<network::ClientConnection> m_network;
    std::string m_address;
    std::optional<std::uint8_t> m_slot;
    std::unordered_set<std::uint16_t> m_lockedRooms;
    std::uint16_t m_pendingRoom;
    Time m_lastRoomList;
    Time m_lastRoomUpdate;
    std::optional<Time> m_noticeUntil;
    std::string m_settingsReturn;
    DebugMetrics m_metrics;
    DevConsole m_console;
    SecretCode m_secretCode;
    Time m_bombUntil;
    std::deque<ecs::UiStore::Row> m_chat;
    std::vector<std::string> m_names;
};

} // namespace engine::client
