#include "ScriptSystemImpl.hpp"

#include <utility>
#include <vector>

#include "Destroyed.hpp"
#include "Script.hpp"
#include "ScriptEntity.hpp"
#include "ScriptError.hpp"

namespace py = pybind11;

namespace engine::scripting {

ScriptSystemImpl::ScriptSystemImpl(ScriptEngineImpl &engine, ScriptWorld world,
    const EventQueue<ecs::DeathEvent> &deaths, const EventQueue<ecs::CollisionEvent> &collisions,
    std::filesystem::path directory)
    : m_engine(engine), m_world(std::move(world)), m_deaths(deaths), m_collisions(collisions),
      m_directory(std::move(directory)), m_records(), m_entities()
{
}

ScriptSystemImpl::~ScriptSystemImpl()
{
    m_records.clear();
}

void ScriptSystemImpl::update(float dt)
{
    m_world.advance(dt);
    runCommands();
    notifyDeaths();
    ecs::Registry &registry = m_world.registry();
    for (std::size_t i : registry.view<ecs::Script>()) {
        const ecs::Entity entity = registry.entityAt(i);
        if (ecs::Destroyed::isActive(registry, entity))
            runEntity(entity, dt);
    }
    notifyContacts();
    forgetDeadEntities();
}

void ScriptSystemImpl::runCommands()
{
    for (const ecs::ConsoleEvent &command : m_world.takeCommands()) {
        const std::string prefix = "[P" + std::to_string(command.slot() + 1) + "] ";
        if (m_world.consoleScript().empty()) {
            m_world.reply(command.slot(), prefix + "this game has no console script");
            continue;
        }
        try {
            py::module_ module = m_engine.load(m_directory, m_world.consoleScript());
            py::object ship = py::none();
            if (const auto found = m_world.playerShip(command.slot()))
                ship = py::cast(*found);
            const py::object result =
                module.attr("run")(py::cast(&m_world, py::return_value_policy::reference), ship, command.text());
            m_world.reply(command.slot(), prefix + py::str(result).cast<std::string>());
        } catch (const std::exception &error) {
            m_world.reply(command.slot(), prefix + "command failed");
            ScriptEngineImpl::report(m_world.consoleScript(), "run", error);
        }
    }
}

void ScriptSystemImpl::notifyDeaths()
{
    for (const ecs::DeathEvent &death : m_deaths.events()) {
        const auto found = m_records.find(keyOf(death.victim()));
        if (found == m_records.end() || found->second.failed())
            continue;
        call(found->second, "on_death", py::none());
    }
}

void ScriptSystemImpl::notifyContacts()
{
    for (const ecs::CollisionEvent &collision : m_collisions.events()) {
        notifyContact(collision.first(), collision.second());
        notifyContact(collision.second(), collision.first());
    }
}

void ScriptSystemImpl::notifyContact(const ecs::Entity &entity, const ecs::Entity &other)
{
    const ecs::Registry &registry = m_world.registry();
    if (!ecs::Destroyed::isActive(registry, entity) || !registry.isAlive(other))
        return;
    const auto found = m_records.find(keyOf(entity));
    if (found == m_records.end() || found->second.failed())
        return;
    call(found->second, "on_contact", py::cast(ScriptEntity(m_world, other)));
}

void ScriptSystemImpl::runEntity(const ecs::Entity &entity, float dt)
{
    ecs::Registry &registry = m_world.registry();
    const std::string file = registry.get<ecs::Script>(entity).file();
    ScriptRecord *record = recordOf(entity, file);
    if (!record || record->failed())
        return;
    ScriptEntity &handle = record->handle().cast<ScriptEntity &>();
    handle.advance(dt);
    if (!record->spawned()) {
        record->markSpawned();
        record->setLastHealth(handle.health());
        call(*record, "on_spawn", py::none());
    }
    const int health = handle.health();
    if (health < record->lastHealth() && !record->failed())
        call(*record, "on_damage", py::int_(record->lastHealth() - health));
    record->setLastHealth(health);
    if (!record->failed() && handle.alive())
        call(*record, "on_update", py::float_(dt));
}

ScriptRecord *ScriptSystemImpl::recordOf(const ecs::Entity &entity, const std::string &file)
{
    const std::uint64_t key = keyOf(entity);
    const auto found = m_records.find(key);
    if (found != m_records.end())
        return &found->second;
    py::object module = py::none();
    try {
        module = m_engine.load(m_directory, file);
    } catch (const ScriptError &error) {
        ScriptEngineImpl::report(file, "load", error);
    }
    py::object handle = py::cast(ScriptEntity(m_world, entity));
    auto [inserted, created] = m_records.emplace(key, ScriptRecord(file, module, handle));
    m_entities.emplace(key, entity);
    if (module.is_none())
        inserted->second.markFailed();
    return &inserted->second;
}

void ScriptSystemImpl::call(ScriptRecord &record, const char *hook, const py::object &argument)
{
    if (!py::hasattr(record.module(), hook))
        return;
    try {
        py::object function = record.module().attr(hook);
        if (argument.is_none())
            function(record.handle());
        else
            function(record.handle(), argument);
    } catch (const py::error_already_set &error) {
        record.markFailed();
        ScriptEngineImpl::report(record.file(), hook, error);
    } catch (const std::exception &error) {
        record.markFailed();
        ScriptEngineImpl::report(record.file(), hook, error);
    }
}

void ScriptSystemImpl::forgetDeadEntities()
{
    const ecs::Registry &registry = m_world.registry();
    std::vector<std::uint64_t> dead;
    for (const auto &[key, entity] : m_entities) {
        if (!registry.isAlive(entity))
            dead.push_back(key);
    }
    for (std::uint64_t key : dead) {
        m_records.erase(key);
        m_entities.erase(key);
    }
}

std::uint64_t ScriptSystemImpl::keyOf(const ecs::Entity &entity)
{
    return (static_cast<std::uint64_t>(entity.generation()) << GENERATION_SHIFT)
        | static_cast<std::uint64_t>(entity.index());
}

} // namespace engine::scripting
