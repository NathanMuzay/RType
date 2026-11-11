#pragma once

#include <cstdint>
#include <optional>

#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"

namespace engine::network {

class MessageValidator
{
public:
    explicit MessageValidator(const NetworkSettings &settings);

    template <class Message>
    bool operator()(const Message &message) const;

private:
    const NetworkSettings &m_settings;
};

class MessageWriter
{
public:
    explicit MessageWriter(net::ByteWriter &writer);

    template <class Message>
    void operator()(const Message &message) const;

private:
    net::ByteWriter &m_writer;
};

class TokenReader
{
public:
    template <class Message>
    std::optional<std::uint32_t> operator()(const Message &message) const;
};

} // namespace engine::network

#include "PacketVisitors.tpp"
