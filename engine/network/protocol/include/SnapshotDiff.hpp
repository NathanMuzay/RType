#pragma once

#include "SnapshotDeltaMessage.hpp"
#include "SnapshotMessage.hpp"

namespace engine::network {

class SnapshotDiff
{
public:
    static SnapshotDeltaMessage build(const SnapshotMessage &base, const SnapshotMessage &current);
    static SnapshotMessage apply(const SnapshotMessage &base, const SnapshotDeltaMessage &delta);

private:
    static const EntityState *find(const SnapshotMessage &snapshot, std::uint16_t id);
};

} // namespace engine::network
