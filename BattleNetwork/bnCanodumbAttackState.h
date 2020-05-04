#pragma once
#include "bnAIState.h"
#include "bnCanodumb.h"

/**
 * @class CanodumbAttackState
 * @author mav
 * @date 05/05/19
 * @brief Spawns a cannon and smoke effect
 */
class CanodumbAttackState : public AIState<Canodumb>
{
public:
  CanodumbAttackState();
  ~CanodumbAttackState();

  void OnEnter(Canodumb& can) final override;
  void OnUpdate(float _elapsed, Canodumb& can) final override;
  void OnLeave(Canodumb& can) final override;
};

