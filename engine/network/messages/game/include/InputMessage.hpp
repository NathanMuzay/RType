#pragma once

#include <cstdint>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class InputMessage
{
public:
    static constexpr MessageType TYPE = MessageType::Input;

    InputMessage(std::uint32_t token, std::uint8_t buttons);

    std::uint32_t token() const;
    std::uint8_t buttons() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(engine::net::ByteWriter &writer) const;
    static InputMessage read(engine::net::ByteReader &reader);

private:
    std::uint32_t m_token;
    std::uint8_t m_buttons;
};

} // namespace engine::network
