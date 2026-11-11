#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "IGameSession.hpp"

namespace engine::network {

class ISessionFactory
{
public:
    virtual ~ISessionFactory() = default;

    virtual std::unique_ptr<IGameSession> createSession(const std::vector<std::uint8_t> &rules) = 0;
};

} // namespace engine::network
