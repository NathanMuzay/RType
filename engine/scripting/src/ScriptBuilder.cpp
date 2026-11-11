#include "ScriptBuilder.hpp"

#include <functional>
#include <utility>

#include "ScriptSystem.hpp"
#include "ScriptWorld.hpp"

namespace engine::scripting {

ScriptBuilder::ScriptBuilder(ScriptEngine &engine, std::filesystem::path directory)
    : m_engine(engine), m_directory(std::move(directory))
{
}

void ScriptBuilder::registerInto(scene::SystemFactory &factory)
{
    factory.add(SYSTEM_NAME, std::bind_front(&ScriptBuilder::build, this));
}

std::unique_ptr<ecs::ISystem> ScriptBuilder::build(scene::SystemContext &context, const scene::DataNode &)
{
    ScriptWorld world(context);
    return std::make_unique<ScriptSystem>(
        m_engine, std::move(world), context.deaths(), context.collisions(), m_directory);
}

} // namespace engine::scripting
