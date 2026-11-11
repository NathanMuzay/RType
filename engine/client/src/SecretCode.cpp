#include "SecretCode.hpp"

#include <cctype>
#include <utility>

namespace engine::client {

SecretCode::SecretCode(std::string code) : m_code(), m_recent()
{
    for (const char letter : code)
        m_code.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(letter))));
}

bool SecretCode::feed(const std::string &typed)
{
    if (m_code.empty())
        return false;
    bool complete = false;
    for (const char letter : typed) {
        m_recent.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(letter))));
        if (m_recent.size() > m_code.size())
            m_recent.erase(0, m_recent.size() - m_code.size());
        if (m_recent == m_code) {
            complete = true;
            m_recent.clear();
        }
    }
    return complete;
}

} // namespace engine::client
