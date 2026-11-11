#pragma once

#include <string>

namespace engine::client {

class SecretCode
{
public:
    explicit SecretCode(std::string code);

    bool feed(const std::string &typed);

private:
    std::string m_code;
    std::string m_recent;
};

} // namespace engine::client
