#include "EngineComponentLoaders.hpp"

#include <cmath>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "Background.hpp"
#include "Bounce.hpp"
#include "Bounded.hpp"
#include "BoundText.hpp"
#include "Button.hpp"
#include "Choice.hpp"
#include "Collider.hpp"
#include "ContactDamage.hpp"
#include "DataReaders.hpp"
#include "DeathEffect.hpp"
#include "DespawnOffscreen.hpp"
#include "FadeWhen.hpp"
#include "Focusable.hpp"
#include "Foreground.hpp"
#include "Gravity.hpp"
#include "Health.hpp"
#include "Hidden.hpp"
#include "Invulnerable.hpp"
#include "KeyBind.hpp"
#include "Lifetime.hpp"
#include "ListView.hpp"
#include "Lives.hpp"
#include "MusicWhen.hpp"
#include "Parallax.hpp"
#include "ParticleEmitter.hpp"
#include "Persistent.hpp"
#include "PlayerControl.hpp"
#include "PlayerLabel.hpp"
#include "ProgressBar.hpp"
#include "Pushable.hpp"
#include "RectShape.hpp"
#include "Replicated.hpp"
#include "Respawn.hpp"
#include "Rotation.hpp"
#include "SceneError.hpp"
#include "Score.hpp"
#include "ScoreValue.hpp"
#include "Script.hpp"
#include "SettingSlider.hpp"
#include "Solid.hpp"
#include "SoundCue.hpp"
#include "Sprite.hpp"
#include "SpriteAnimation.hpp"
#include "StatusText.hpp"
#include "Text.hpp"
#include "TextField.hpp"
#include "Tilemap.hpp"
#include "TiltAnimation.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
#include "VisibleWhen.hpp"
#include "Weapon.hpp"

namespace engine::scene {

void EngineComponentLoaders::loadButton(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    ecs::Button &button = registry.emplace<ecs::Button>(
        entity, data.getString("action"), data.getFloat("width"), data.getFloat("height"));
    if (data.has("normal"))
        button.setColors(DataReaders::readColor(data, "normal"), DataReaders::readColor(data, "hover"),
            DataReaders::readColor(data, "pressed"));
    if (data.has("normalFrame"))
        button.setFrames(DataReaders::readRect(data, "normalFrame"), DataReaders::readRect(data, "hoverFrame"),
            DataReaders::readRect(data, "pressedFrame"));
    button.setSounds(readOptionalString(data, "hoverSound"), readOptionalString(data, "clickSound"));
}

void EngineComponentLoaders::loadFocusable(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Focusable>(entity, data.getInt("order"));
}

void EngineComponentLoaders::loadTextField(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    const int maxLength = data.getInt("maxLength");
    if (maxLength <= 0)
        throw SceneError("EngineComponentLoaders: " + data.path() + ".maxLength must be positive");
    registry.emplace<ecs::TextField>(entity, data.getString("name"), static_cast<std::size_t>(maxLength),
        data.getBool("masked", false), readOptionalString(data, "placeholder"), data.getFloat("width"),
        data.getFloat("height"), readOptionalString(data, "submit"));
}

void EngineComponentLoaders::loadVisibleWhen(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    std::vector<ecs::VisibleWhen::Condition> also;
    if (data.has("and")) {
        for (const DataNode &condition : data.child("and").items())
            also.emplace_back(condition.getString("key"), condition.getString("value"));
    }
    std::string unlessKey;
    std::string unlessValue;
    if (data.has("unless")) {
        unlessKey = data.child("unless").getString("key");
        unlessValue = data.child("unless").getString("value");
    }
    registry.emplace<ecs::VisibleWhen>(entity, data.getString("key"), data.getString("value"), std::move(also),
        std::move(unlessKey), std::move(unlessValue));
}

void EngineComponentLoaders::loadBoundText(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::BoundText>(entity, data.getString("format"));
}

void EngineComponentLoaders::loadListView(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    std::vector<ecs::ListRowPart> parts;
    for (const DataNode &part : data.child("parts").items())
        parts.emplace_back(part.getString("prefab"), part.getFloat("offsetX", 0.f), part.getFloat("offsetY", 0.f));
    const int maxRows = data.getInt("maxRows");
    if (maxRows <= 0)
        throw SceneError("EngineComponentLoaders: " + data.path() + ".maxRows must be positive");
    registry.emplace<ecs::ListView>(entity, data.getString("source"), data.getFloat("spacing"),
        static_cast<std::size_t>(maxRows), std::move(parts));
}

void EngineComponentLoaders::loadHidden(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &)
{
    registry.emplace<ecs::Hidden>(entity);
}

void EngineComponentLoaders::loadBackground(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &)
{
    registry.emplace<ecs::Background>(entity);
}

void EngineComponentLoaders::loadForeground(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &)
{
    registry.emplace<ecs::Foreground>(entity);
}

void EngineComponentLoaders::loadProgressBar(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::ProgressBar>(entity, data.getString("key"), data.getFloat("width"), data.getFloat("max", 1.f),
        data.getBool("vertical", false));
}

void EngineComponentLoaders::loadFadeWhen(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::FadeWhen>(
        entity, data.getString("key"), data.getString("value"), data.getFloat("seconds", 0.f));
}

void EngineComponentLoaders::loadChoice(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    std::vector<std::string> options;
    for (const DataNode &option : data.child("options").items())
        options.push_back(option.asString());
    registry.emplace<ecs::Choice>(entity, data.getString("name"), std::move(options));
}

void EngineComponentLoaders::loadKeyBind(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    const std::string device = data.has("device") ? data.getString("device") : ecs::KeyBind::KEYBOARD;
    if (device != ecs::KeyBind::KEYBOARD && device != ecs::KeyBind::GAMEPAD)
        throw SceneError("EngineComponentLoaders: " + data.path() + ".device must be keyboard or gamepad");
    const std::string prompt = data.has("prompt") ? data.getString("prompt") : ecs::KeyBind::DEFAULT_PROMPT;
    registry.emplace<ecs::KeyBind>(entity, data.getString("action"), device, prompt);
}

void EngineComponentLoaders::loadSettingSlider(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    if (data.has("options")) {
        std::vector<std::string> options;
        for (const DataNode &option : data.child("options").items())
            options.push_back(option.asString());
        registry.emplace<ecs::SettingSlider>(
            entity, data.getString("setting"), std::move(options), true, data.getInt("direction", 0));
        return;
    }
    registry.emplace<ecs::SettingSlider>(
        entity, data.getString("setting"), sliderRange(data), false, data.getInt("direction", 0));
}

std::vector<std::string> EngineComponentLoaders::sliderRange(const DataNode &data)
{
    const float minimum = data.getFloat("min");
    const float maximum = data.getFloat("max");
    const float step = data.getFloat("step");
    if (step <= 0.f || maximum < minimum)
        throw SceneError("EngineComponentLoaders: " + data.path() + " needs min <= max and a positive step");
    const bool whole = std::floor(step) == step && std::floor(minimum) == minimum;
    std::vector<std::string> values;
    for (int index = 0; minimum + static_cast<float>(index) * step <= maximum + step / 2.f; ++index) {
        const float value = minimum + static_cast<float>(index) * step;
        if (whole) {
            values.push_back(std::to_string(static_cast<int>(std::lround(value))));
        } else {
            const long tenths = std::lround(value * 10.f);
            values.push_back(std::to_string(tenths / 10) + "." + std::to_string(tenths % 10));
        }
    }
    return values;
}

} // namespace engine::scene
