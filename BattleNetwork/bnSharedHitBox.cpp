#include <random>
#include <time.h>

#include "bnSharedHitbox.h"
#include "bnTile.h"
#include "bnField.h"
#include "bnTextureResourceManager.h"
#include "bnAudioResourceManager.h"

SharedHitbox::SharedHitbox(Spell* owner, float duration) : owner(owner), Spell(owner->GetField(), owner->GetTeam()) {
  cooldown = duration;
  SetHitboxProperties(owner->GetHitboxProperties());
  keepAlive = (duration == 0.0f);

  Entity::RemoveCallback& deleteHandler = owner->CreateRemoveCallback();
  deleteHandler.Slot([this]() {
      this->owner = nullptr;
  });
}

SharedHitbox::~SharedHitbox() {
}

void SharedHitbox::OnUpdate(float _elapsed) {
  cooldown -= _elapsed;
  
  tile->AffectEntities(this);
  
  if (this->owner) {
      if (this->owner->IsDeleted()) {
          this->Delete();
      }
      else if (!keepAlive && cooldown <= 0.f ) {
          this->Delete();
      }
  }
  else if (this->keepAlive) {
      // If we are set to KeepAlive but the owner isn't set, delete
      this->Delete();
  }
}

bool SharedHitbox::Move(Direction _direction) {
  return false;
}

void SharedHitbox::Attack(Character* _entity) {
  if(_entity->GetID() == owner->GetID()) return;
  
  if(owner) {
    owner->Attack(_entity);
  }

  // Remove after first registered hit
  this->Delete();
}

void SharedHitbox::OnDelete()
{
  Remove();
}

const float SharedHitbox::GetHeight() const {
    if(Character* c = dynamic_cast<Character*>(owner)) { return c->GetHeight(); }
    else { return 0; }
}
