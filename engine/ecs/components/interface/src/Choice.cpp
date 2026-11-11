#include "Choice.hpp"

#include <utility>

#include "EcsError.hpp"

namespace engine::ecs {

Choice::Choice(std::string name, std::vector<std::string> options)
    : m_name(std::move(name)), m_options(std::move(options)), m_index(0)
{
    if (m_options.empty())
        throw EcsError("Choice: " + m_name + " needs at least one option");
}

const std::string &Choice::name() const
{
    return m_name;
}

const std::string &Choice::current() const
{
    return m_options[m_index];
}

void Choice::next()
{
    m_index = (m_index + 1) % m_options.size();
}

void Choice::previous()
{
    m_index = (m_index + m_options.size() - 1) % m_options.size();
}

} // namespace engine::ecs
