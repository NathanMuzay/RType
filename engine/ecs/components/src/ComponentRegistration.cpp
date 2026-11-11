#include "ComponentRegistration.hpp"

#include "Anchor.hpp"
#include "Attachment.hpp"
#include "Background.hpp"
#include "Bounce.hpp"
#include "Bounded.hpp"
#include "BoundText.hpp"
#include "Button.hpp"
#include "Choice.hpp"
#include "Collider.hpp"
#include "ContactDamage.hpp"
#include "DeathEffect.hpp"
#include "DespawnOffscreen.hpp"
#include "Destroyed.hpp"
#include "FadeWhen.hpp"
#include "Focusable.hpp"
#include "Foreground.hpp"
#include "Gravity.hpp"
#include "Health.hpp"
#include "Hidden.hpp"
#include "HitFlash.hpp"
#include "Hovered.hpp"
#include "Invulnerable.hpp"
#include "KeyBind.hpp"
#include "Lifetime.hpp"
#include "ListView.hpp"
#include "Lives.hpp"
#include "Match.hpp"
#include "MusicWhen.hpp"
#include "Owner.hpp"
#include "Parallax.hpp"
#include "ParticleEmitter.hpp"
#include "Persistent.hpp"
#include "Player.hpp"
#include "PlayerControl.hpp"
#include "PlayerIndex.hpp"
#include "PlayerLabel.hpp"
#include "PlayerValues.hpp"
#include "ProgressBar.hpp"
#include "Pushable.hpp"
#include "RectShape.hpp"
#include "Replicated.hpp"
#include "Respawn.hpp"
#include "Rotation.hpp"
#include "RowData.hpp"
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

namespace engine::ecs {

void registerEngineComponents(Registry &registry)
{
    registry.registerComponent<Transform>();
    registry.registerComponent<Velocity>();
    registry.registerComponent<Collider>();
    registry.registerComponent<Bounded>();
    registry.registerComponent<Health>();
    registry.registerComponent<ContactDamage>();
    registry.registerComponent<Invulnerable>();
    registry.registerComponent<DespawnOffscreen>();
    registry.registerComponent<PlayerControl>();
    registry.registerComponent<Owner>();
    registry.registerComponent<ScoreValue>();
    registry.registerComponent<Destroyed>();
    registry.registerComponent<Sprite>();
    registry.registerComponent<RectShape>();
    registry.registerComponent<Parallax>();
    registry.registerComponent<TiltAnimation>();
    registry.registerComponent<Weapon>();
    registry.registerComponent<Replicated>();
    registry.registerComponent<SpriteAnimation>();
    registry.registerComponent<SoundCue>();
    registry.registerComponent<Text>();
    registry.registerComponent<Anchor>();
    registry.registerComponent<Player>();
    registry.registerComponent<Lives>();
    registry.registerComponent<Score>();
    registry.registerComponent<Respawn>();
    registry.registerComponent<Match>();
    registry.registerComponent<Persistent>();
    registry.registerComponent<DeathEffect>();
    registry.registerComponent<PlayerIndex>();
    registry.registerComponent<PlayerLabel>();
    registry.registerComponent<StatusText>();
    registry.registerComponent<Button>();
    registry.registerComponent<Hovered>();
    registry.registerComponent<Hidden>();
    registry.registerComponent<Focusable>();
    registry.registerComponent<TextField>();
    registry.registerComponent<VisibleWhen>();
    registry.registerComponent<FadeWhen>();
    registry.registerComponent<Background>();
    registry.registerComponent<Foreground>();
    registry.registerComponent<ProgressBar>();
    registry.registerComponent<Script>();
    registry.registerComponent<Attachment>();
    registry.registerComponent<HitFlash>();
    registry.registerComponent<Rotation>();
    registry.registerComponent<PlayerValues>();
    registry.registerComponent<Solid>();
    registry.registerComponent<Pushable>();
    registry.registerComponent<Tilemap>();
    registry.registerComponent<ParticleEmitter>();
    registry.registerComponent<Lifetime>();
    registry.registerComponent<MusicWhen>();
    registry.registerComponent<Choice>();
    registry.registerComponent<KeyBind>();
    registry.registerComponent<SettingSlider>();
    registry.registerComponent<Gravity>();
    registry.registerComponent<Bounce>();
    registry.registerComponent<BoundText>();
    registry.registerComponent<RowData>();
    registry.registerComponent<ListView>();
}

} // namespace engine::ecs
