#include "OnlineClient.hpp"

#include <cstddef>
#include <utility>
#include <vector>

#include "Action.hpp"
#include "EngineError.hpp"
#include "PortOption.hpp"
#include "Protocol.hpp"
#include "TextFormat.hpp"
#include "TextRules.hpp"
#include "UdpTransport.hpp"

namespace engine::client {

void OnlineClient::applySnapshot(const network::SnapshotMessage &snapshot)
{
    if (m_view.currentScene() == ROOM_SCENE)
        enterScene(GAME_SCENE);
    if (m_view.currentScene() != GAME_SCENE)
        return;
    m_presenter.apply(snapshot);
}

void OnlineClient::showPlayerLeft(const network::PlayerLeftMessage &left, Time now)
{
    const std::string key = left.reason() == network::Protocol::LEFT_TIMEOUT ? "playerLost" : "playerLeft";
    const std::string text = replaceToken(m_view.message(key), "{slot}", std::to_string(left.playerId() + 1));
    m_view.uiStore().setValue(NOTICE_KEY, text);
    m_console.print(text);
    m_noticeUntil = now + NOTICE_DURATION;
}

void OnlineClient::expireNotice(Time now)
{
    if (!m_noticeUntil || now < *m_noticeUntil)
        return;
    m_view.uiStore().setValue(NOTICE_KEY, {});
    m_noticeUntil.reset();
}

} // namespace engine::client
