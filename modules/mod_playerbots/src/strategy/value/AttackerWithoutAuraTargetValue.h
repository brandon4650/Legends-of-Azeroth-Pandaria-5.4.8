#ifndef _PLAYERBOT_ATTACKERWITHOUTAURATARGETVALUE_H
#define _PLAYERBOT_ATTACKERWITHOUTAURATARGETVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class AttackerWithoutAuraTargetValue : public UnitCalculatedValue, public Qualified
{
public:
    AttackerWithoutAuraTargetValue(PlayerbotAI* botAI, std::string range = "spell")
        : UnitCalculatedValue(botAI, "attacker without aura"), range(range)
    {
    }

protected:
    Unit* Calculate() override;
    std::string range;
};

class MeleeAttackerWithoutAuraTargetValue : public AttackerWithoutAuraTargetValue
{
public:
    MeleeAttackerWithoutAuraTargetValue(PlayerbotAI* botAI, bool checkArc = true) : AttackerWithoutAuraTargetValue(botAI, "melee"), checkArc(checkArc) {}
    Unit* Calculate() override;
    bool checkArc;
};

#endif
