/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WORLDPACKETHANDLERSTRATEGY_H
#define _PLAYERBOT_WORLDPACKETHANDLERSTRATEGY_H

#include "PassTroughStrategy.h"

class PlayerbotAI;

class WorldPacketHandlerStrategy : public PassTroughStrategy
{
public:
    WorldPacketHandlerStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "default"; }
};

#endif
