#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class CreateRoomMessage
{
public:
    static constexpr MessageType TYPE = MessageType::CreateRoom;

    CreateRoomMessage(std::uint32_t token, std::string name, std::string code, std::vector<std::uint8_t> rules);

    std::uint32_t token() const;
    const std::string &name() const;
    const std::string &code() const;
    const std::vector<std::uint8_t> &rules() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static CreateRoomMessage read(net::ByteReader &reader);

private:
    std::uint32_t m_token;
    std::string m_name;
    std::string m_code;
    std::vector<std::uint8_t> m_rules;
};

} // namespace engine::network
