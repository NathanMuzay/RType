#include "PacketVisitors.hpp"

namespace engine::network {

MessageValidator::MessageValidator(const NetworkSettings &settings) : m_settings(settings)
{
}

MessageWriter::MessageWriter(net::ByteWriter &writer) : m_writer(writer)
{
}

} // namespace engine::network
