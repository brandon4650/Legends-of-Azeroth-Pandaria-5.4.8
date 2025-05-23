#ifndef _PLAYERBOT_EstimatedLifetimeValue_H
#define _PLAYERBOT_EstimatedLifetimeValue_H

#include "NamedObjectContext.h"
#include "PossibleTargetsValue.h"
#include "TargetValue.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

// [target health] / [expected group single target dps] = [expected lifetime]
class EstimatedLifetimeValue : public FloatCalculatedValue, public Qualified
{
public:
    EstimatedLifetimeValue(PlayerbotAI* botAI) : FloatCalculatedValue(botAI, "estimated lifetime") {}

public:
    float Calculate() override;
};

class EstimatedGroupDpsValue : public FloatCalculatedValue
{
public:
    EstimatedGroupDpsValue(PlayerbotAI* botAI) : FloatCalculatedValue(botAI, "estimated group dps", 20 * 1000) {}

public:
    float Calculate() override;

protected:
    float GetBasicDps(uint32 level);
    float GetBasicGs(uint32 level);
};

#endif
