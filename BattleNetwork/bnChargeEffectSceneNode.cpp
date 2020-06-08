#include "bnEntity.h"
#include "bnTextureResourceManager.h"
#include "bnAudioResourceManager.h"
#include "bnShaderResourceManager.h"
#include "bnChargeEffectSceneNode.h"

ChargeEffectSceneNode::ChargeEffectSceneNode(Entity* _entity) {
  entity = _entity;
  charging = false;
  chargeCounter = 0.0f;
  setTexture(LOAD_TEXTURE(SPELL_BUSTER_CHARGE));

  animation = Animation("resources/spells/spell_buster_charge.animation");

  isCharged = isPartiallyCharged = false;
  chargeColor = sf::Color::Magenta;
}

ChargeEffectSceneNode::~ChargeEffectSceneNode() {
}

void ChargeEffectSceneNode::Update(float _elapsed) {
  if (!charging) {
    chargeCounter = 0.0f;
    setScale(0.0f, 0.0f);
    isCharged = false;
    isPartiallyCharged = false;
  } else {

    chargeCounter += _elapsed;
    if (chargeCounter >= CHARGE_COUNTER_MAX) {
      if (isCharged == false) {
        // We're switching states
        Audio().Play(AudioType::BUSTER_CHARGED);
        animation.SetAnimation("CHARGED");
        animation << Animator::Mode::Loop;
        setColor(chargeColor);
        SetShader(SHADERS.GetShader(ShaderType::ADDITIVE));

      }

      isCharged = true;      
    } else if (chargeCounter >= CHARGE_COUNTER_MIN) {
      if (isPartiallyCharged == false) {
        // Switching states
        Audio().Play(AudioType::BUSTER_CHARGING);
        animation.SetAnimation("CHARGING");
        animation << Animator::Mode::Loop;
        setColor(sf::Color::White);
        RevokeShader();

      }

      isPartiallyCharged = true;
    }

    if (isPartiallyCharged) {
      setScale(1.0f, 1.0f);
    }
  }

  animation.Update(_elapsed, getSprite());
}

void ChargeEffectSceneNode::SetCharging(bool _charging) {
  charging = _charging;
}

float ChargeEffectSceneNode::GetChargeCounter() const {
  return chargeCounter;
}

const bool ChargeEffectSceneNode::IsFullyCharged() const
{
  return isCharged;
}

void ChargeEffectSceneNode::SetFullyChargedColor(const sf::Color color)
{
  chargeColor = color;
}
