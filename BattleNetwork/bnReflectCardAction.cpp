#include "bnReflectCardAction.h"
#include "bnCardAction.h"
#include "bnSpriteProxyNode.h"
#include "bnTextureResourceManager.h"
#include "bnAudioResourceManager.h"
#include "bnReflectShield.h"

#define FRAME1 { 1, 1.3f }

#define FRAMES FRAME1


ReflectCardAction::ReflectCardAction(Character& owner, int damage) : CardAction(owner, "PLAYER_IDLE") {
  ReflectCardAction::damage = damage;

  // add override anims
  OverrideAnimationFrames({ FRAMES });
}

ReflectCardAction::~ReflectCardAction()
{
}

void ReflectCardAction::OnExecute() {
  auto& owner = GetUser();

  // Create a new reflect shield component. This handles the logic for shields.
  ReflectShield* reflect = new ReflectShield(&owner, damage);

  // Add the component to the player
  owner.RegisterComponent(reflect);

  // Play the appear sound
  Audio().Play(AudioType::APPEAR);

  // Add shield artifact on the same layer as player
  Battle::Tile* tile = owner.GetTile();

  if (tile) {
    auto status = owner.GetField()->AddEntity(*reflect, tile->GetX(), tile->GetY());
    if (status != Field::AddEntityStatus::deleted) {
      Entity::RemoveCallback& cleanup = reflect->CreateRemoveCallback();
      cleanup.Slot([this]() {
        this->EndAction();
      });
    }
    else {
      this->EndAction(); // quit early
    }
  }
}

void ReflectCardAction::OnEndAction()
{
}
