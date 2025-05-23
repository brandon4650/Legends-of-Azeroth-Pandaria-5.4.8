/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "CharacterCache.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Player.h"
#include "Timer.h"
#include "World.h"
#include <unordered_map>

namespace
{
    std::unordered_map<ObjectGuid, CharacterCacheEntry> _characterCacheStore;
    std::unordered_map<std::string, CharacterCacheEntry*> _characterCacheByNameStore;
}

CharacterCache* CharacterCache::instance()
{
    static CharacterCache instance;
    return &instance;
}

/**
* @brief Loads several pieces of information on server startup with the GUID
* There is no further database query necessary.
* These are a number of methods that work into the calling function.
*
* @param guid Requires a guid to call
* @return Name, Gender, Race, Class and Level of player character
* Example Usage:
* @code
*    CharacterCacheEntry const* characterInfo = sCharacterCache->GetCharacterCacheByGuid(GUID);
*    if (!characterInfo)
*        return;
*
*    std::string playerName = characterInfo->Name;
*    uint8 playerGender = characterInfo->Sex;
*    uint8 playerRace = characterInfo->Race;
*    uint8 playerClass = characterInfo->Class;
*    uint8 playerLevel = characterInfo->Level;
* @endcode
**/

void CharacterCache::LoadCharacterCacheStorage()
{
    _characterCacheStore.clear();
    uint32 oldMSTime = getMSTime();

    QueryResult result = CharacterDatabase.Query("SELECT guid, name, account, race, gender, class, level FROM characters");
    if (!result)
    {
        TC_LOG_INFO("server.loading", "No character name data loaded, empty query!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        AddCharacterCacheEntry(ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt32()) /*guid*/, fields[2].GetUInt32() /*account*/, fields[1].GetString() /*name*/,
            fields[4].GetUInt8() /*gender*/, fields[3].GetUInt8() /*race*/, fields[5].GetUInt8() /*class*/, fields[6].GetUInt8() /*level*/);
    } while (result->NextRow());

    QueryResult mailCountResult = CharacterDatabase.Query("SELECT receiver, COUNT(receiver) FROM mail GROUP BY receiver");
    if (mailCountResult)
    {
        do
        {
            Field* fields = mailCountResult->Fetch();
            UpdateCharacterMailCount(ObjectGuid(HighGuid::Player, fields[0].GetUInt32()), static_cast<int8>(fields[1].GetUInt64()), true);
        } while (mailCountResult->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded Character Infos For %u Characters in %u ms", _characterCacheStore.size(), GetMSTimeDiffToNow(oldMSTime));
    TC_LOG_INFO("server.loading", " ");
}

void CharacterCache::RefreshCacheEntry(uint32 lowGuid)
{
    QueryResult result = CharacterDatabase.PQuery("SELECT guid, name, account, race, gender, class, level FROM characters WHERE guid = %u", lowGuid);
    if (!result)
    {
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        DeleteCharacterCacheEntry(ObjectGuid::Create<HighGuid::Player>(lowGuid), fields[1].GetString());
        AddCharacterCacheEntry(ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt32()) /*guid*/, fields[2].GetUInt32() /*account*/, fields[1].GetString() /*name*/, fields[4].GetUInt8() /*gender*/, fields[3].GetUInt8() /*race*/,
            fields[5].GetUInt8() /*class*/, fields[6].GetUInt8() /*level*/);
    } while (result->NextRow());

    QueryResult mailCountResult = CharacterDatabase.PQuery("SELECT receiver, COUNT(receiver) FROM mail WHERE receiver = %u GROUP BY receiver", lowGuid);
    if (mailCountResult)
    {
        do
        {
            Field* fields = mailCountResult->Fetch();
            UpdateCharacterMailCount(ObjectGuid(HighGuid::Player, fields[0].GetUInt32()), static_cast<int8>(fields[1].GetInt64()), true);
        } while (mailCountResult->NextRow());
    }
}

/*
Modifying functions
*/
void CharacterCache::AddCharacterCacheEntry(ObjectGuid const& guid, uint32 accountId, std::string const& name, uint8 gender, uint8 race, uint8 playerClass, uint8 level)
{
    CharacterCacheEntry& data = _characterCacheStore[guid];
    data.Guid = guid;
    data.Name = name;
    data.AccountId = accountId;
    data.Race = race;
    data.Sex = gender;
    data.Class = playerClass;
    data.Level = level;
    data.GuildId = 0;                           // Will be set in guild loading or guild setting
    //for (uint8 i = 0; i < MAX_ARENA_SLOT; ++i)
    //{
    //    data.ArenaTeamId[i] = 0; // Will be set in arena teams loading
    //}

    // Fill Name to Guid Store
    _characterCacheByNameStore[name] = &data;
}

void CharacterCache::DeleteCharacterCacheEntry(ObjectGuid const& guid, std::string const& name)
{
    _characterCacheStore.erase(guid);
    _characterCacheByNameStore.erase(name);
}

void CharacterCache::UpdateCharacterData(ObjectGuid const& guid, std::string const& name, Optional<uint8> gender /*= {}*/, Optional<uint8> race /*= {}*/)
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
        return;

    std::string oldName = itr->second.Name;
    itr->second.Name = name;

    if (gender)
    {
        itr->second.Sex = *gender;
    }

    if (race)
    {
        itr->second.Race = *race;
    }

    //WorldPackets::Misc::InvalidatePlayer packet(guid);
    //sWorld->SendGlobalMessage(packet.Write());

    // Correct name -> pointer storage
    _characterCacheByNameStore.erase(oldName);
    _characterCacheByNameStore[name] = &itr->second;
}

void CharacterCache::UpdateCharacterLevel(ObjectGuid const& guid, uint8 level)
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
    {
        return;
    }

    itr->second.Level = level;
}

void CharacterCache::UpdateCharacterAccountId(ObjectGuid const& guid, uint32 accountId)
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
    {
        return;
    }

    itr->second.AccountId = accountId;
}

void CharacterCache::UpdateCharacterGuildId(ObjectGuid const& guid, ObjectGuid::LowType guildId)
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
    {
        return;
    }

    itr->second.GuildId = guildId;
}

/*void CharacterCache::UpdateCharacterArenaTeamId(ObjectGuid const& guid, uint8 slot, uint32 arenaTeamId)
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
    {
        return;
    }

    itr->second.ArenaTeamId[slot] = arenaTeamId;
}*/

void CharacterCache::UpdateCharacterMailCount(ObjectGuid const& guid, int8 count, bool update)
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
    {
        return;
    }

    if (update)
    {
        itr->second.MailCount = count;
        return;
    }

    // Let's be safe and prevent overflow
    if (!itr->second.MailCount && count < 0)
    {
        return;
    }

    itr->second.MailCount += count;
}

void CharacterCache::UpdateCharacterGroup(ObjectGuid const& guid, ObjectGuid groupGUID)
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
    {
        return;
    }

    itr->second.GroupGuid = groupGUID;
}

/*
Getters
*/
bool CharacterCache::HasCharacterCacheEntry(ObjectGuid const& guid) const
{
    return _characterCacheStore.find(guid) != _characterCacheStore.end();
}

CharacterCacheEntry const* CharacterCache::GetCharacterCacheByGuid(ObjectGuid const& guid) const
{
    auto itr = _characterCacheStore.find(guid);
    if (itr != _characterCacheStore.end())
    {
        return &itr->second;
    }

    return nullptr;
}

CharacterCacheEntry const* CharacterCache::GetCharacterCacheByName(std::string const& name) const
{
    auto itr = _characterCacheByNameStore.find(name);
    if (itr != _characterCacheByNameStore.end())
    {
        return itr->second;
    }

    return nullptr;
}

ObjectGuid CharacterCache::GetCharacterGuidByName(std::string const& name) const
{
    auto itr = _characterCacheByNameStore.find(name);
    if (itr != _characterCacheByNameStore.end())
    {
        return itr->second->Guid;
    }

    return ObjectGuid::Empty;
}

bool CharacterCache::GetCharacterNameByGuid(ObjectGuid guid, std::string& name) const
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
    {
        return false;
    }

    name = itr->second.Name;
    return true;
}

/*uint32 CharacterCache::GetCharacterTeamByGuid(ObjectGuid guid) const
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
    {
        return 0;
    }

    return Player::TeamIdForRace(itr->second.Race);
}*/

uint32 CharacterCache::GetCharacterAccountIdByGuid(ObjectGuid guid) const
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
    {
        return 0;
    }

    return itr->second.AccountId;
}

uint32 CharacterCache::GetCharacterAccountIdByName(std::string const& name) const
{
    auto itr = _characterCacheByNameStore.find(name);
    if (itr != _characterCacheByNameStore.end())
    {
        return itr->second->AccountId;
    }

    return 0;
}

uint8 CharacterCache::GetCharacterLevelByGuid(ObjectGuid guid) const
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
    {
        return 0;
    }

    return itr->second.Level;
}

ObjectGuid::LowType CharacterCache::GetCharacterGuildIdByGuid(ObjectGuid guid) const
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
    {
        return 0;
    }

    return itr->second.GuildId;
}

/*uint32 CharacterCache::GetCharacterArenaTeamIdByGuid(ObjectGuid guid, uint8 type) const
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
    {
        return 0;
    }

    return itr->second.ArenaTeamId[type];
}*/

ObjectGuid CharacterCache::GetCharacterGroupGuidByGuid(ObjectGuid guid) const
{
    auto itr = _characterCacheStore.find(guid);
    if (itr == _characterCacheStore.end())
    {
        return ObjectGuid::Empty;
    }

    return itr->second.GroupGuid;
}
