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

OnlineClient::OnlineClient(const std::filesystem::path &assetsDirectory, const std::filesystem::path &dataDirectory,
    const std::string &game, const network::NetworkSettings &settings, std::optional<std::string> address)
    : m_view(assetsDirectory, dataDirectory, game), m_presenter(m_view), m_settings(settings), m_network(),
      m_address(address.value_or(m_view.settings().serverAddress())), m_slot(), m_lockedRooms(), m_pendingRoom(0),
      m_lastRoomList(), m_lastRoomUpdate(), m_noticeUntil(), m_settingsReturn(), m_metrics(), m_console(m_view),
      m_secretCode(m_view.secretCode()), m_bombUntil()
{
    enterScene(m_view.currentScene());
}

void OnlineClient::run()
{
    while (m_view.isOpen()) {
        const Time now = std::chrono::steady_clock::now();
        m_view.processEvents();
        if (const std::string action = m_view.anyKeyAction(); !action.empty())
            handleAction(action);
        watchSecretCode(now);
        updateNetwork(now);
        expireNotice(now);
        m_view.frame();
        if (m_view.takeLoadingFinished())
            enterScene(m_view.currentScene());
        if (m_view.debugPressed())
            DebugMetrics::toggle(m_view.uiStore());
        handleConsole();
        m_metrics.frame(m_view.uiStore(), m_view.entityCount(), m_network ? m_network->bytesReceived() : 0,
            m_network ? m_network->bytesSent() : 0, m_network ? m_network->pingMs() : 0);
        handleActions();
    }
    m_view.close();
    if (m_network)
        m_network->disconnect();
}

void OnlineClient::watchSecretCode(Time now)
{
    if (m_view.currentScene() == GAME_SCENE && m_secretCode.feed(m_view.typedText()))
        m_bombUntil = now + BOMB_HOLD;
}

void OnlineClient::updateNetwork(Time now)
{
    if (!m_network)
        return;
    try {
        m_network->update(buttons());
        for (const network::Packet &event : m_network->takeEvents())
            handleEvent(event, now);
        if (auto snapshot = m_network->takeSnapshot())
            applySnapshot(*snapshot);
    } catch (const EngineError &) {
        disconnect("connectionFailed");
        return;
    }
    const std::string &scene = m_view.currentScene();
    m_view.uiStore().setValue("lobby.status", m_view.message(m_network->connected() ? "online" : "connecting"));
    if (scene == LOBBY_SCENE && m_network->connected() && now - m_lastRoomList >= ROOM_LIST_INTERVAL) {
        m_network->listRooms();
        m_lastRoomList = now;
    }
    if ((scene == ROOM_SCENE || scene == GAME_SCENE) && now - m_lastRoomUpdate >= ROOM_SILENCE_LIMIT) {
        enterScene(LOBBY_SCENE);
        m_view.uiStore().setValue("lobby.error", m_view.message("roomLost"));
    }
}

void OnlineClient::handleActions()
{
    std::vector<std::string> actions;
    const std::vector<ecs::UiActionEvent> events = m_view.takeUiActions();
    if (m_console.open())
        return;
    for (const ecs::UiActionEvent &event : events)
        actions.push_back(event.action());
    if (m_view.uiStore().value(CAPTURING_KEY) == YES)
        return;
    if (m_view.escapePressed() && popupOpen())
        actions.push_back("close_popup");
    else if (m_view.escapePressed() && !m_view.escapeAction().empty())
        actions.push_back(m_view.escapeAction());
    for (const std::string &action : actions)
        handleAction(action);
}

void OnlineClient::handleAction(const std::string &action)
{
    if (action == "quit")
        m_view.closeWindow();
    else if (action == "restart_game")
        restartGame();
    else if (action == "pause")
        setPaused(!paused());
    else if (action == "resume")
        setPaused(false);
    else if (action == "settings")
        openSettings();
    else if (action == "back")
        closeSettings();
    else if (action == "settings_reset")
        m_view.resetSettings();
    else if (action == "connect")
        connect();
    else if (action == "disconnect")
        disconnect({});
    else if (startsWith(action, "scene:"))
        enterScene(action.substr(std::string("scene:").size()));
    else if (!m_network)
        return;
    else if (startsWith(action, COMMAND_PREFIX))
        m_network->sendCommand(action.substr(std::string(COMMAND_PREFIX).size()));
    else if (action == "refresh_rooms")
        m_network->listRooms();
    else if (action == "create_room")
        createRoom();
    else if (startsWith(action, "join:"))
        joinOrAsk(static_cast<std::uint16_t>(std::stoul(action.substr(std::string("join:").size()))));
    else if (action == "join_code")
        m_network->joinRoom(m_pendingRoom, field("joinCode"));
    else if (action == "close_popup")
        closePopup();
    else if (action == "leave_room") {
        m_network->leaveRoom();
        m_slot.reset();
        clearChat();
        enterScene(LOBBY_SCENE);
    } else if (action == "start_game")
        m_network->startGame();
    else if (startsWith(action, "kick:"))
        m_network->kickPlayer(static_cast<std::uint8_t>(std::stoul(action.substr(std::string("kick:").size()))));
    else if (action == "delete_room")
        m_network->deleteRoom();
    else if (action == "send_chat")
        sendChat();
}

void OnlineClient::handleEvent(const network::Packet &event, Time now)
{
    ecs::UiStore &store = m_view.uiStore();
    if (event.holds<network::RoomListMessage>()) {
        showRoomList(event.as<network::RoomListMessage>());
    } else if (event.holds<network::RoomJoinedMessage>()) {
        const network::RoomJoinedMessage &joined = event.as<network::RoomJoinedMessage>();
        m_slot = joined.slot();
        m_lastRoomUpdate = now;
        store.setValue("room.name", joined.name());
        store.setValue("lobby.error", {});
        if (m_view.currentScene() != ROOM_SCENE) {
            if (m_view.currentScene() != GAME_SCENE)
                clearChat();
            enterScene(ROOM_SCENE);
        }
    } else if (event.holds<network::ChatBroadcastMessage>()) {
        showChat(event.as<network::ChatBroadcastMessage>());
    } else if (event.holds<network::RoomRejectedMessage>()) {
        handleRejection(event.as<network::RoomRejectedMessage>().reason());
    } else if (event.holds<network::RoomUpdateMessage>()) {
        m_lastRoomUpdate = now;
        const network::RoomUpdateMessage &update = event.as<network::RoomUpdateMessage>();
        m_names.assign(update.names().begin(), update.names().end());
        showMembers(update);
    } else if (event.holds<network::GameStartedMessage>()) {
        m_lastRoomUpdate = now;
        enterScene(GAME_SCENE);
    } else if (event.holds<network::KickedMessage>() || event.holds<network::RoomClosedMessage>()) {
        m_slot.reset();
        enterScene(LOBBY_SCENE);
        store.setValue("lobby.error", m_view.message(event.holds<network::KickedMessage>() ? "kicked" : "roomClosed"));
    } else if (event.holds<network::ConsoleTextMessage>()) {
        m_console.print(event.as<network::ConsoleTextMessage>().text());
    } else if (event.holds<network::PlaySoundMessage>() && m_view.currentScene() == GAME_SCENE) {
        m_view.playEventSound(event.as<network::PlaySoundMessage>().sound());
    } else if (event.holds<network::PlayerLeftMessage>() && m_view.currentScene() == GAME_SCENE) {
        showPlayerLeft(event.as<network::PlayerLeftMessage>(), now);
    } else if (event.holds<network::EntityDestroyedMessage>() && m_view.currentScene() == GAME_SCENE) {
        m_view.destroyEntity(event.as<network::EntityDestroyedMessage>().entityId());
    }
}

void OnlineClient::connect()
{
    if (const std::string typed = field("serverAddress"); !typed.empty())
        m_address = typed;
    const std::size_t colon = m_address.rfind(':');
    try {
        const std::string host = colon == std::string::npos ? m_address : m_address.substr(0, colon);
        const std::uint16_t port =
            colon == std::string::npos ? m_settings.defaultPort() : PortOption::parse(m_address.substr(colon + 1));
        m_network = std::make_unique<network::ClientConnection>(
            net::UdpTransport::resolve(host, port), m_settings, field("playerName"));
    } catch (const EngineError &) {
        m_view.uiStore().setValue("menu.error", m_view.message("invalidAddress"));
        return;
    }
    m_lastRoomList = Time();
    enterScene(LOBBY_SCENE);
}

void OnlineClient::restartGame()
{
    if (m_network)
        m_network->disconnect();
    m_network.reset();
    m_slot.reset();
    m_view.replayIntro();
}

void OnlineClient::disconnect(const std::string &errorKey)
{
    if (m_network)
        m_network->disconnect();
    m_network.reset();
    m_slot.reset();
    enterScene(MENU_SCENE);
    m_view.uiStore().setValue("menu.error", errorKey.empty() ? std::string() : m_view.message(errorKey));
}

void OnlineClient::enterScene(const std::string &scene)
{
    if (scene != m_view.currentScene())
        m_view.switchScene(scene);
    if (m_console.open() && scene != ROOM_SCENE && scene != GAME_SCENE)
        m_console.toggle();
    ecs::UiStore &store = m_view.uiStore();
    store.setValue("menu.error", {});
    store.setValue("lobby.error", {});
    store.setValue("room.error", {});
    store.setValue(PAUSE_KEY, NO);
    store.setValue("lobby.popup", NO);
    store.setValue("lobby.popupError", {});
    if (scene == MENU_SCENE)
        m_view.setFieldText("serverAddress", m_address);
}

std::uint8_t OnlineClient::buttons() const
{
    using input::Action;

    if (m_view.currentScene() != GAME_SCENE || m_console.open() || m_view.uiStore().value(PAUSE_KEY) == YES)
        return 0;
    std::uint8_t bits = 0;
    if (m_view.isPressed(Action::Up))
        bits |= network::Protocol::BUTTON_UP;
    if (m_view.isPressed(Action::Down))
        bits |= network::Protocol::BUTTON_DOWN;
    if (m_view.isPressed(Action::Left))
        bits |= network::Protocol::BUTTON_LEFT;
    if (m_view.isPressed(Action::Right))
        bits |= network::Protocol::BUTTON_RIGHT;
    if (m_view.isPressed(Action::Fire))
        bits |= network::Protocol::BUTTON_FIRE;
    if (m_view.isPressed(Action::Special))
        bits |= network::Protocol::BUTTON_ACTION;
    if (std::chrono::steady_clock::now() < m_bombUntil)
        bits |= network::Protocol::BUTTON_BOMB;
    return bits;
}

std::string OnlineClient::field(const std::string &name)
{
    return m_view.uiStore().value("field." + name);
}

bool OnlineClient::startsWith(const std::string &text, const std::string &prefix)
{
    return text.rfind(prefix, 0) == 0;
}

void OnlineClient::handleConsole()
{
    const bool consoleScene = m_view.currentScene() == ROOM_SCENE || m_view.currentScene() == GAME_SCENE;
    if (m_view.consolePressed() && (consoleScene || m_console.open()))
        m_console.toggle();
    if (!m_console.open() || !m_view.enterPressed())
        return;
    const auto command = m_console.submit();
    if (!command)
        return;
    if (m_network && m_network->connected())
        m_network->sendCommand(*command);
    else
        m_console.print("not connected to a server");
}

void OnlineClient::openSettings()
{
    m_settingsReturn = m_view.currentScene();
    enterScene(SETTINGS_SCENE);
}

void OnlineClient::closeSettings()
{
    if (m_view.currentScene() != SETTINGS_SCENE)
        return;
    enterScene(m_settingsReturn.empty() ? MENU_SCENE : m_settingsReturn);
}

void OnlineClient::setPaused(bool paused)
{
    m_view.uiStore().setValue(PAUSE_KEY, paused ? YES : NO);
    if (paused)
        m_view.focusAction("resume");
}

bool OnlineClient::paused()
{
    return m_view.uiStore().value(PAUSE_KEY) == YES;
}

} // namespace engine::client
