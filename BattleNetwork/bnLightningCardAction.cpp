#include "bnLightningCardAction.h"
#include "bnCardAction.h"
#include "bnSpriteProxyNode.h"
#include "bnTextureResourceManager.h"
#include "bnAudioResourceManager.h"
#include "bnHitbox.h"
#include "bnField.h"

#define FRAME1 { 1, 0.05 }
#define FRAME2 { 2, 0.05 }
#define FRAME3 { 3, 0.05 }

#define FRAMES FRAME1, FRAME2, FRAME3, FRAME3, FRAME3

#define LIGHTNING_IMG "resources/spells/spell_lightning.png"
#define LIGHTNING_ANI "resources/spells/spell_lightning.animation"

LightningCardAction::LightningCardAction(Character& actor, int damage) :
  CardAction(actor, "PLAYER_SHOOTING")
{
  LightningCardAction::damage = damage;

  attachment = new SpriteProxyNode();
  attachment->setTexture(actor.getTexture());
  attachment->SetLayer(-1);

  attachmentAnim = Animation(actor.GetFirstComponent<AnimationComponent>()->GetFilePath());
  attachmentAnim.Reload();
  attachmentAnim.SetAnimation("BUSTER");

  attack = new SpriteProxyNode();
  attack->setTexture(Textures().LoadTextureFromFile(LIGHTNING_IMG));
  attack->SetLayer(-2);

  attackAnim = Animation(LIGHTNING_ANI);
  attackAnim.SetAnimation("DEFAULT");

  AddAttachment(actor, "buster", *attachment).UseAnimation(attachmentAnim);

  // add override anims
  OverrideAnimationFrames({ FRAMES });
}

void LightningCardAction::OnExecute(Character* user) {
  attachment->EnableParentShader(true);

  // On shoot frame, drop projectile
  auto onFire = [=]() -> void {
    Audio().Play(AudioType::SPREADER);

    attachment->AddNode(attack);
    attack->setPosition(attachmentAnim.GetPoint("endpoint"));

    attackAnim.Update(0, attack->getSprite());

    auto field = user->GetField();
    auto team = user->GetTeam();
    int col = user->GetTile()->GetX();
    int row = user->GetTile()->GetY();

    for (int i = 1; i < 6; i++) {
      auto hitbox = new Hitbox(team, LightningCardAction::damage);
      hitbox->HighlightTile(Battle::Tile::Highlight::solid);
      auto props = hitbox->GetHitboxProperties();
      props.aggressor = user->GetID();
      props.damage = LightningCardAction::damage;
      props.element = Element::elec;

      if (stun) {
        props.flags |= Hit::stun;
      }

      hitbox->SetHitboxProperties(props);
      field->AddEntity(*hitbox, col + i, row);
    }

    Audio().Play(AudioType::THUNDER);
    this->fired = true;
  };

  AddAnimAction(2, onFire);
}

void LightningCardAction::SetStun(bool stun)
{
  this->stun = stun;
}

LightningCardAction::~LightningCardAction()
{
}

void LightningCardAction::Update(double _elapsed)
{
  if (fired && attack) {
    attackAnim.Update(_elapsed, attack->getSprite());
  }

  CardAction::Update(_elapsed);
}

void LightningCardAction::OnAnimationEnd()
{
  if (attack) {
    attachment->RemoveNode(attack);
    delete attack;
    attack = nullptr;
  }
}

void LightningCardAction::OnEndAction()
{
  OnAnimationEnd();
}
