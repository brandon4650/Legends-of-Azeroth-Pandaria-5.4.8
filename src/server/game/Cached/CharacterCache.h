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

#ifndef CharacterCache_h__
#define CharacterCache_h__

#include "Define.h"
#include "ObjectGuid.h"
#include "Optional.h"
#include <string>

struct CharacterCacheEntry
{
    ObjectGuid Guid;
    std::string Name;
    uint32 AccountId;
    uint8 Class;
    uint8 Race;
    uint8 Sex;
    uint8 Level;
    uint8 MailCount;
    ObjectGuid::LowType GuildId;
    //std::array<uint32, MAX_ARENA_SLOT> ArenaTeamId;
    ObjectGuid GroupGuid;
};

class TC_GAME_API CharacterCache
{
    public:
        CharacterCache() noexcept = default;
        ~CharacterCache() noexcept = default;
        static CharacterCache* instance();

        void LoadCharacterCacheStorage();
        void RefreshCacheEntry(uint32 lowGuid);

        void AddCharacterCacheEntry(ObjectGuid const& guid, uint32 accountId, std::string const& name, uint8 gender, uint8 race, uint8 playerClass, uint8 level);
        void DeleteCharacterCacheEntry(ObjectGuid const& guid, std::string const& name);

        void UpdateCharacterData(ObjectGuid const& guid, std::string const& name, Optional<uint8> gender = {}, Optional<uint8> race = {});
        void UpdateCharacterLevel(ObjectGuid const& guid, uint8 level);
        void UpdateCharacterAccountId(ObjectGuid const& guid, uint32 accountId);
        void UpdateCharacterGuildId(ObjectGuid const& guid, ObjectGuid::LowType guildId);
        void UpdateCharacterArenaTeamId(ObjectGuid const& guid, uint8 slot, uint32 arenaTeamId);

        void UpdateCharacterMailCount(ObjectGuid const& guid, int8 count, bool update = false);
        void DecreaseCharacterMailCount(ObjectGuid const& guid) { UpdateCharacterMailCount(guid, -1); };
        void IncreaseCharacterMailCount(ObjectGuid const& guid) { UpdateCharacterMailCount(guid, 1); };

        [[nodiscard]] bool HasCharacterCacheEntry(ObjectGuid const& guid) const;
        [[nodiscard]] CharacterCacheEntry const* GetCharacterCacheByGuid(ObjectGuid const& guid) const;
        [[nodiscard]] CharacterCacheEntry const* GetCharacterCacheByName(std::string const& name) const;

        void UpdateCharacterGroup(ObjectGuid const& guid, ObjectGuid groupGUID);
        void ClearCharacterGroup(ObjectGuid const& guid) { UpdateCharacterGroup(guid, ObjectGuid::Empty); };

        [[nodiscard]] ObjectGuid GetCharacterGuidByName(std::string const& name) const;
        bool GetCharacterNameByGuid(ObjectGuid guid, std::string& name) const;
        //[[nodiscard]] uint32 GetCharacterTeamByGuid(ObjectGuid guid) const;
        [[nodiscard]] uint32 GetCharacterAccountIdByGuid(ObjectGuid guid) const;
        [[nodiscard]] uint32 GetCharacterAccountIdByName(std::string const& name) const;
        [[nodiscard]] uint8 GetCharacterLevelByGuid(ObjectGuid guid) const;
        [[nodiscard]] ObjectGuid::LowType GetCharacterGuildIdByGuid(ObjectGuid guid) const;
        //[[nodiscard]] uint32 GetCharacterArenaTeamIdByGuid(ObjectGuid guid, uint8 type) const;
        [[nodiscard]] ObjectGuid GetCharacterGroupGuidByGuid(ObjectGuid guid) const;
};

#define sCharacterCache CharacterCache::instance()

#endif // CharacterCache_h__
