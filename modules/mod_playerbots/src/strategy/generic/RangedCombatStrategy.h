#ifndef _PLAYERBOT_RANGEDCOMBATSTRATEGY_H
#define _PLAYERBOT_RANGEDCOMBATSTRATEGY_H

#include "CombatStrategy.h"

class PlayerbotAI;

class RangedCombatStrategy : public CombatStrategy
{
public:
    RangedCombatStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_RANGED; }
    std::string const getName() override { return "ranged"; }
};

#endif
