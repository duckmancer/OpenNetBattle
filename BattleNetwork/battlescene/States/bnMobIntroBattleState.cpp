#include "bnMobIntroBattleState.h"

#include "../bnBattleSceneBase.h"
#include "../../bnPlayer.h"
#include "../../bnMob.h"
#include "../../bnAgent.h"

void MobIntroBattleState::onUpdate(double elapsed)
{
  if (!GetScene().IsSceneInFocus()) return;

  Field& field = *GetScene().GetField();

  if (mob->NextMobReady()) {
    Mob::MobData* data = mob->GetNextMob();

    Agent* cast = dynamic_cast<Agent*>(data->mob);

    // Some entities have AI and need targets
    // TODO: support multiple targets
    Player* player = tracked[0];
    if (cast) {
      cast->SetTarget(player);
    }

    Character* enemy = data->mob;

    enemy->ToggleTimeFreeze(false);
    GetScene().GetField()->AddEntity(*enemy, data->tileX, data->tileY);

    // Listen for events
    GetScene().CounterHitListener::Subscribe(*enemy);
    GetScene().HitListener::Subscribe(*enemy);
  }

  GetScene().GetField()->Update((float)elapsed);
}

void MobIntroBattleState::onEnd(const BattleSceneState*)
{
  mob->DefaultState();

  for (auto* player : tracked) {
    player->ChangeState<PlayerControlledState>();
  }

  // Tell everything to begin battle
  GetScene().BroadcastBattleStart();
}

void MobIntroBattleState::onStart(const BattleSceneState*)
{
}

void MobIntroBattleState::onDraw(sf::RenderTexture&)
{
  ENGINE.Draw(GetScene().GetCardSelectWidget());
}

const bool MobIntroBattleState::IsOver() {
  return mob->IsSpawningDone();
}

MobIntroBattleState::MobIntroBattleState(Mob* mob, std::vector<Player*> tracked) 
  : mob(mob), tracked(tracked)
{
}
