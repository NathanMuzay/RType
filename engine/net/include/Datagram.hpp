#pragma once

#include <cstdint>
#include <vector>

#include "Endpoint.hpp"

namespace engine::net {

class Datagram
{
public:
    Datagram(Endpoint sender, std::vector<std::uint8_t> bytes);

    const Endpoint &sender() const;
    const std::vector<std::uint8_t> &bytes() const;

private:
    Endpoint m_sender;
    std::vector<std::uint8_t> m_bytes;
};

} // namespace engine::net
