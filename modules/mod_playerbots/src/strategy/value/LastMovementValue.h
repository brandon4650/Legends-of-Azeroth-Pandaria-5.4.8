/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LASTMOVEMENTVALUE_H
#define _PLAYERBOT_LASTMOVEMENTVALUE_H

#include "ObjectGuid.h"
#include "WorldPosition.h"
#include "MovementActions.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class LastMovement
{
public:
    LastMovement();
    LastMovement(LastMovement& other);

    LastMovement& operator=(LastMovement const& other)
    {
        taxiNodes = other.taxiNodes;
        taxiMaster = other.taxiMaster;
        lastFollow = other.lastFollow;
        lastAreaTrigger = other.lastAreaTrigger;
        lastMoveShort = other.lastMoveShort;
        nextTeleport = other.nextTeleport;
        priority = other.priority;
        return *this;
    };

    void clear();

    void Set(Unit* follow);
    void Set(uint32 mapId, float x, float y, float z, float ori, float delayTime, MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);

    void setShort(WorldPosition point);

    std::vector<uint32> taxiNodes;
    ObjectGuid taxiMaster;
    Unit* lastFollow;
    uint32 lastAreaTrigger;
    time_t lastFlee;
    uint32 lastMoveToMapId;
    float lastMoveToX;
    float lastMoveToY;
    float lastMoveToZ;
    float lastMoveToOri;
    float lastdelayTime;
    WorldPosition lastMoveShort;
    uint32 msTime;
    MovementPriority priority;
    time_t nextTeleport;
};

class LastMovementValue : public ManualSetValue<LastMovement&>
{
public:
    LastMovementValue(PlayerbotAI* botAI) : ManualSetValue<LastMovement&>(botAI, data) {}

private:
    LastMovement data = LastMovement();
};

class StayTimeValue : public ManualSetValue<time_t>
{
public:
    StayTimeValue(PlayerbotAI* botAI) : ManualSetValue<time_t>(botAI, 0) {}
};

#endif
