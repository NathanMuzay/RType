#include "SnapshotDiff.hpp"

#include <algorithm>
#include <unordered_set>

namespace engine::network {

const EntityState *SnapshotDiff::find(const SnapshotMessage &snapshot, std::uint16_t id)
{
    for (const EntityState &entity : snapshot.entities()) {
        if (entity.id() == id)
            return &entity;
    }
    return nullptr;
}

SnapshotDeltaMessage SnapshotDiff::build(const SnapshotMessage &base, const SnapshotMessage &current)
{
    SnapshotDeltaMessage::WorldChanges world;
    for (std::size_t i = 0; i < current.values().size(); ++i) {
        if (i < base.values().size() && base.values()[i] == current.values()[i])
            continue;
        world.emplace_back(static_cast<std::uint8_t>(i), current.values()[i]);
    }
    SnapshotDeltaMessage::PlayerChanges players;
    for (std::size_t i = 0; i < current.players().size(); ++i) {
        if (i < base.players().size() && base.players()[i] == current.players()[i])
            continue;
        players.emplace_back(static_cast<std::uint8_t>(i), current.players()[i]);
    }
    std::vector<EntityChange> entities;
    for (const EntityState &entity : current.entities()) {
        const EntityState *old = find(base, entity.id());
        if (!old) {
            entities.push_back(EntityChange::created(entity));
            continue;
        }
        if (const auto change = EntityChange::between(*old, entity))
            entities.push_back(*change);
    }
    std::vector<std::uint16_t> removed;
    for (const EntityState &entity : base.entities()) {
        if (!find(current, entity.id()))
            removed.push_back(entity.id());
    }
    return SnapshotDeltaMessage(current.tick(), base.tick(), current.gameState(), std::move(world), std::move(players),
        std::move(entities), std::move(removed));
}

SnapshotMessage SnapshotDiff::apply(const SnapshotMessage &base, const SnapshotDeltaMessage &delta)
{
    SnapshotMessage::Values values = base.values();
    for (const auto &entry : delta.world()) {
        if (entry.first < values.size())
            values[entry.first] = entry.second;
    }
    SnapshotMessage::Players players = base.players();
    for (const auto &entry : delta.players()) {
        if (entry.first < players.size())
            players[entry.first] = entry.second;
    }
    const std::unordered_set<std::uint16_t> removed(delta.removed().begin(), delta.removed().end());
    std::unordered_set<std::uint16_t> replaced;
    for (const EntityChange &change : delta.entities())
        replaced.insert(change.id());
    std::vector<EntityState> entities;
    for (const EntityState &entity : base.entities()) {
        if (removed.count(entity.id()) == 0 && replaced.count(entity.id()) == 0)
            entities.push_back(entity);
    }
    for (const EntityChange &change : delta.entities()) {
        if (const auto resolved = change.resolve(find(base, change.id())))
            entities.push_back(*resolved);
    }
    return SnapshotMessage(delta.tick(), delta.gameState(), std::move(values), std::move(players), std::move(entities));
}

} // namespace engine::network
