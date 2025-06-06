/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SHAREDVALUECONTEXT_H
#define _PLAYERBOT_SHAREDVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "Playerbots.h"

class PlayerbotAI;
class SharedValueContext : public NamedObjectContext<UntypedValue>
{
public:
    SharedValueContext() : NamedObjectContext(true)
    {

    }

private:

    // Global acess functions
public:
    static SharedValueContext* instance()
    {
        static SharedValueContext instance;
        return &instance;
    }

    template <class T>
    Value<T>* getGlobalValue(std::string const name)
    {
        NamedObjectContextList<UntypedValue> valueContexts;
        valueContexts.Add(this);
        PlayerbotAI* botAI = new PlayerbotAI();
        UntypedValue* value = valueContexts.GetContextObject(name, botAI);
        delete botAI;
        return dynamic_cast<Value<T>*>(value);
    }

    template <class T>
    Value<T>* getGlobalValue(std::string const name, std::string const param)
    {
        return getGlobalValue<T>((std::string(name) + "::" + param));
    }

    template <class T>
    Value<T>* getGlobalValue(std::string const name, uint32 param)
    {
        std::ostringstream out;
        out << param;
        return getGlobalValue<T>(name, out.str());
    }
};

#define sSharedValueContext SharedValueContext::instance()

#endif
