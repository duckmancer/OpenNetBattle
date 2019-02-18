#pragma once
#include "bnCharacter.h"
#include "bnComponent.h"
#include "bnField.h"

class DefenseAura;

class Aura : virtual public Character, virtual public Component
{
public:
  enum class Type : int {
    AURA_100,
    AURA_200,
    AURA_1000,
    BARRIER_100,
    BARRIER_200,
    BARRIER_500
  };

private:
  Animation animation;
  sf::Sprite aura;
  Type type;
  DefenseAura* defense;

public:
  Aura(Type type, Character* owner);
  ~Aura();

  virtual void Inject(BattleScene&);
  virtual void Update(float _elapsed);
  virtual bool Move(Direction _direction) { return false; }
  const Type GetAuraType();
  virtual const bool Hit(int _damage, Hit::Properties props = Hit::DefaultProperties);


};

