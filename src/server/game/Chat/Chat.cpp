/*
* This file is part of the Pandaria 5.4.8 Project. See THANKS file for Copyright information
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

#include "Common.h"
#include "ObjectMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"

#include "AccountMgr.h"
#include "CellImpl.h"
#include "Chat.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Language.h"
#include "Log.h"
#include "Opcodes.h"
#include "Player.h"
#include "UpdateMask.h"
#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "ChatLink.h"

bool ChatHandler::load_command_table = true;

std::vector<ChatCommand> const& ChatHandler::getCommandTable()
{
    static std::vector<ChatCommand> commandTableCache;

    if (LoadCommandTable())
    {
        SetLoadCommandTable(false);

        std::vector<ChatCommand> cmds = sScriptMgr->GetChatCommands();
        commandTableCache.swap(cmds);

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_COMMANDS);
        PreparedQueryResult result = WorldDatabase.Query(stmt);
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                std::string name = fields[0].GetString();

                SetDataForCommandInTable(commandTableCache, name.c_str(), fields[1].GetUInt8(), fields[2].GetString(), name);
            }
            while (result->NextRow());
        }
    }

    return commandTableCache;
}

std::string ChatHandler::PGetParseString(int32 entry, ...) const
{
    const char *format = GetTrinityString(entry);
    char str[1024];
    va_list ap;
    va_start(ap, entry);
    vsnprintf(str, 1024, format, ap);
    va_end(ap);
    return std::string(str);
}

const char *ChatHandler::GetTrinityString(int32 entry) const
{
    return m_session->GetTrinityString(entry);
}

bool ChatHandler::isAvailable(ChatCommand const& cmd) const
{
    return m_session->GetSecurity() >= AccountTypes(cmd.SecurityLevel);
}

bool ChatHandler::HasLowerSecurity(Player* target, ObjectGuid guid, bool strong)
{
    WorldSession* target_session = NULL;
    uint32 target_account = 0;

    if (target)
        target_session = target->GetSession();
    else if (guid)
        target_account = sObjectMgr->GetPlayerAccountIdByGUID(guid);

    if (!target_session && !target_account)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return true;
    }

    return HasLowerSecurityAccount(target_session, target_account, strong);
}

bool ChatHandler::HasLowerSecurityAccount(WorldSession* target, uint32 target_account, bool strong)
{
    uint32 target_sec;

    // allow everything from console and RA console
    if (!m_session)
        return false;

    // ignore only for non-players for non strong checks (when allow apply command at least to same sec level)
    if (GetSession()->GetSecurity() > SEC_PLAYER && !strong && !sWorld->getBoolConfig(CONFIG_GM_LOWER_SECURITY))
        return false;

    if (target)
        target_sec = target->GetSecurity();
    else if (target_account)
        target_sec = AccountMgr::GetSecurity(target_account, realm.Id.Realm);
    else
        return true;                                        // caller must report error for (target == NULL && target_account == 0)

    AccountTypes target_ac_sec = AccountTypes(target_sec);
    if (m_session->GetSecurity() < target_ac_sec || (strong && m_session->GetSecurity() <= target_ac_sec))
    {
        SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
        SetSentErrorMessage(true);
        return true;
    }

    return false;
}

bool ChatHandler::hasStringAbbr(const char* name, const char* part)
{
    // non "" command
    if (*name)
    {
        // "" part from non-"" command
        if (!*part)
            return false;

        while (true)
        {
            if (!*part)
                return true;
            else if (!*name)
                return false;
            else if (tolower(*name) != tolower(*part))
                return false;
            ++name; ++part;
        }
    }
    // allow with any for ""

    return true;
}

void ChatHandler::SendSysMessage(const char *str)
{
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = strdup(str);
    char* pos = buf;

    while (char* line = LineFromMessage(pos))
    {
        BuildChatPacket(data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, NULL, NULL, line);
        m_session->SendPacket(&data);
    }

    free(buf);
}

void ChatHandler::SendGlobalSysMessage(const char *str)
{
    // Chat output
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = strdup(str);
    char* pos = buf;

    while (char* line = LineFromMessage(pos))
    {
        BuildChatPacket(data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, NULL, NULL, line);
        sWorld->SendGlobalMessage(&data);
    }

    free(buf);
}

void ChatHandler::SendGlobalGMSysMessage(const char *str)
{
    // Chat output
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = strdup(str);
    char* pos = buf;

    while (char* line = LineFromMessage(pos))
    {
        BuildChatPacket(data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, NULL, NULL, line);
        sWorld->SendGlobalGMMessage(&data);
    }
    free(buf);
}

void ChatHandler::SendSysMessage(int32 entry)
{
    SendSysMessage(GetTrinityString(entry));
}

void ChatHandler::PSendSysMessage(int32 entry, ...)
{
    const char *format = GetTrinityString(entry);
    va_list ap;
    char str [2048];
    va_start(ap, entry);
    vsnprintf(str, 2048, format, ap);
    va_end(ap);
    SendSysMessage(str);
}

void ChatHandler::PSendSysMessage(const char *format, ...)
{
    va_list ap;
    char str [2048];
    va_start(ap, format);
    vsnprintf(str, 2048, format, ap);
    va_end(ap);
    SendSysMessage(str);
}

bool ChatHandler::SendError(int32 entry, ...)
{
    char const* format = GetTrinityString(entry);
    va_list ap;
    char str[4096];
    va_start(ap, entry);
    auto n = vsnprintf(str, 4096, format, ap);
    va_end(ap);
    if (n == -1)
        SendSysMessage("ChatHandler::SendError - Buffer overflow");
    else
        SendSysMessage(str);
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::SendError(char const* format, ...)
{
    va_list ap;
    char str[4096];
    va_start(ap, format);
    auto n = vsnprintf(str, 4096, format, ap);
    va_end(ap);
    if (n == -1)
        SendSysMessage("ChatHandler::SendError - Buffer overflow");
    else
        SendSysMessage(str);
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::ExecuteCommandInTable(std::vector<ChatCommand> const& table, const char* text, std::string const& fullcmd)
{
    char const* oldtext = text;
    std::string cmd = "";

    while (*text != ' ' && *text != '\0')
    {
        cmd += *text;
        ++text;
    }

    while (*text == ' ') ++text;

    for (uint32 i = 0; i < table.size(); ++i)
    {
        if (!hasStringAbbr(table[i].Name, cmd.c_str()))
            continue;

        bool match = false;
        if (strlen(table[i].Name) > cmd.length())
        {
            for (uint32 j = 0; j < table.size(); ++j)
            {
                if (!hasStringAbbr(table[j].Name, cmd.c_str()))
                    continue;

                if (strcmp(table[j].Name, cmd.c_str()) == 0)
                {
                    match = true;
                    break;
                }
            }
        }
        if (match)
            continue;

        // select subcommand from child commands list
        if (!table[i].ChildCommands.empty())
        {
            if (!ExecuteCommandInTable(table[i].ChildCommands, text, fullcmd))
            {
                if (text[0] != '\0')
                    SendSysMessage(LANG_NO_SUBCMD);
                else
                    SendSysMessage(LANG_CMD_SYNTAX);

                ShowHelpForCommand(table[i].ChildCommands, text);
            }

            return true;
        }

        // must be available and have handler
        if (!table[i].Handler || !isAvailable(table[i]))
            continue;

        SetSentErrorMessage(false);
        // table[i].Name == "" is special case: send original command to handler
        if ((table[i].Handler)(this, table[i].Name[0] != '\0' ? text : oldtext))
        {
            if (!m_session) // ignore console
                return true;

            Player* player = m_session->GetPlayer();
            if (!AccountMgr::IsPlayerAccount(m_session->GetSecurity()))
            {
                ObjectGuid guid = player->GetTarget();
                uint32 areaId = player->GetAreaId();
                std::string areaName = "Unknown";
                std::string zoneName = "Unknown";
                if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaId))
                {
                    areaName = area->area_name[sObjectMgr->GetDBCLocaleIndex()];
                    if (AreaTableEntry const* zone = sAreaTableStore.LookupEntry(area->ParentAreaID))
                        zoneName = zone->area_name[sObjectMgr->GetDBCLocaleIndex()];
                }

                sLog->outCommand(m_session->GetAccountId(), "Command: %s [Player: %s (Guid: %u) (Account: %u) X: %f Y: %f Z: %f Map: %u (%s) Area: %u (%s) Zone: %s Selected %s: %s (GUID: %u)]",
                    fullcmd.c_str(), player->GetName().c_str(), player->GetGUID().GetCounter(),
                    m_session->GetAccountId(), player->GetPositionX(), player->GetPositionY(),
                    player->GetPositionZ(), player->GetMapId(),
                    player->GetMap() ? player->GetMap()->GetMapName() : "Unknown",
                    areaId, areaName.c_str(), zoneName.c_str(), guid.GetTypeName(),
                    (player->GetSelectedUnit()) ? player->GetSelectedUnit()->GetName().c_str() : "",
                    guid.GetCounter());
            }
        }
        // some commands have custom error messages. Don't send the default one in these cases.
        else if (!HasSentErrorMessage())
        {
            if (!table[i].Help.empty())
                SendSysMessage(table[i].Help.c_str());
            else
                SendSysMessage(LANG_CMD_SYNTAX);
        }

        return true;
    }

    return false;
}

bool ChatHandler::SetDataForCommandInTable(std::vector<ChatCommand>& table, char const* text, uint32 security, std::string const& help, std::string const& fullcommand)
{
    std::string cmd = "";

    while (*text != ' ' && *text != '\0')
    {
        cmd += *text;
        ++text;
    }

    while (*text == ' ') ++text;

    for (uint32 i = 0; i < table.size(); ++i)
    {
        // for data fill use full explicit command names
        if (table[i].Name != cmd)
            continue;

        // select subcommand from child commands list (including "")
        if (!table[i].ChildCommands.empty())
        {
            if (SetDataForCommandInTable(table[i].ChildCommands, text, security, help, fullcommand))
                return true;
            else if (*text)
                return false;

            // fail with "" subcommands, then use normal level up command instead
        }
        // expected subcommand by full name DB content
        else if (*text)
        {
            TC_LOG_ERROR("sql.sql", "Table `command` have unexpected subcommand '%s' in command '%s', skip.", text, fullcommand.c_str());
            return false;
        }

        if (table[i].SecurityLevel != security)
            TC_LOG_INFO("misc", "Table `command` overwrite for command '%s' default permission (%u) by %u", fullcommand.c_str(), table[i].SecurityLevel, security);

        table[i].SecurityLevel = security;
        table[i].Help          = help;
        return true;
    }

    // in case "" command let process by caller
    if (!cmd.empty())
    {
        if (&table == &getCommandTable())
            TC_LOG_ERROR("sql.sql", "Table `command` have not existed command '%s', skip.", cmd.c_str());
        else
            TC_LOG_ERROR("sql.sql", "Table `command` have not existed subcommand '%s' in command '%s', skip.", cmd.c_str(), fullcommand.c_str());
    }

    return false;
}

bool ChatHandler::ParseCommands(char const* text)
{
    ASSERT(text);
    ASSERT(*text);

    std::string fullcmd = text;

    /// chat case (.command or !command format)
    if (m_session)
    {
        if (text[0] != '!' && text[0] != '.')
            return false;
    }

    /// ignore single . and ! in line
    if (strlen(text) < 2)
        return false;
    // original `text` can't be used. It content destroyed in command code processing.

    /// ignore messages staring from many dots.
    if ((text[0] == '.' && text[1] == '.') || (text[0] == '!' && text[1] == '!'))
        return false;

    /// skip first . or ! (in console allowed use command with . and ! and without its)
    if (text[0] == '!' || text[0] == '.')
        ++text;

    if (!ExecuteCommandInTable(getCommandTable(), text, fullcmd))
    {
        if (m_session && m_session->GetSecurity() == SEC_PLAYER)
            return false;

        SendSysMessage(LANG_NO_CMD);
    }
    return true;
}

bool ChatHandler::isValidChatMessage(char const* message)
{
/*
Valid examples:
|cffa335ee|Hitem:812:0:0:0:0:0:0:0:70|h[Glowing Brightwood Staff]|h|r
|cff808080|Hquest:2278:47|h[The Platinum Discs]|h|r
|cffffd000|Htrade:4037:1:150:1:6AAAAAAAAAAAAAAAAAAAAAAOAADAAAAAAAAAAAAAAAAIAAAAAAAAA|h[Engineering]|h|r
|cff4e96f7|Htalent:2232:-1|h[Taste for Blood]|h|r
|cff71d5ff|Hspell:21563|h[Command]|h|r
|cffffd000|Henchant:3919|h[Engineering: Rough Dynamite]|h|r
|cffffff00|Hachievement:546:0000000000000001:0:0:0:-1:0:0:0:0|h[Safe Deposit]|h|r
|cff66bbff|Hglyph:21:762|h[Glyph of Bladestorm]|h|r

| will be escaped to ||
*/

    if (strlen(message) > 255)
        return false;

    // more simple checks
    if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_SEVERITY) < 3)
    {
        const char validSequence[6] = "cHhhr";
        const char* validSequenceIterator = validSequence;
        const std::string validCommands = "cHhr|";

        while (*message)
        {
            // find next pipe command
            message = strchr(message, '|');

            if (!message)
                return true;

            ++message;
            char commandChar = *message;
            if (validCommands.find(commandChar) == std::string::npos)
                return false;

            ++message;
            // validate sequence
            if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_SEVERITY) == 2)
            {
                if (commandChar == *validSequenceIterator)
                {
                    if (validSequenceIterator == validSequence + 4)
                        validSequenceIterator = validSequence;
                    else
                        ++validSequenceIterator;
                }
                else
                    return false;
            }
        }
        return true;
    }

    return LinkExtractor(message).IsValidMessage();
}

bool ChatHandler::ShowHelpForSubCommands(std::vector<ChatCommand> const& table, char const* cmd, char const* subcmd)
{
    std::string list;
    for (uint32 i = 0; i < table.size(); ++i)
    {
        // must be available (ignore handler existence for show command with possible available subcommands)
        if (!isAvailable(table[i]))
            continue;

        // for empty subcmd show all available
        if (*subcmd && !hasStringAbbr(table[i].Name, subcmd))
            continue;

        if (m_session)
            list += "\n    ";
        else
            list += "\n\r    ";

        list += table[i].Name;

        if (!table[i].ChildCommands.empty())
            list += " ...";
    }

    if (list.empty())
        return false;

    if (&table == &getCommandTable())
    {
        SendSysMessage(LANG_AVIABLE_CMD);
        PSendSysMessage("%s", list.c_str());
    }
    else
        PSendSysMessage(LANG_SUBCMDS_LIST, cmd, list.c_str());

    return true;
}

bool ChatHandler::ShowHelpForCommand(std::vector<ChatCommand> const& table, const char* cmd)
{
    if (*cmd)
    {
        for (uint32 i = 0; i < table.size(); ++i)
        {
            // must be available (ignore handler existence for show command with possible available subcommands)
            if (!isAvailable(table[i]))
                continue;

            if (!hasStringAbbr(table[i].Name, cmd))
                continue;

            // have subcommand
            char const* subcmd = (*cmd) ? strtok(NULL, " ") : "";

            if (!table[i].ChildCommands.empty() && subcmd && *subcmd)
            {
                if (ShowHelpForCommand(table[i].ChildCommands, subcmd))
                    return true;
            }

            if (!table[i].Help.empty())
                SendSysMessage(table[i].Help.c_str());

            if (!table[i].ChildCommands.empty())
                if (ShowHelpForSubCommands(table[i].ChildCommands, table[i].Name, subcmd ? subcmd : ""))
                    return true;

            return !table[i].Help.empty();
        }
    }
    else
    {
        for (uint32 i = 0; i < table.size(); ++i)
        {
            // must be available (ignore handler existence for show command with possible available subcommands)
            if (!isAvailable(table[i]))
                continue;

            if (strlen(table[i].Name))
                continue;

            if (!table[i].Help.empty())
                SendSysMessage(table[i].Help.c_str());

            if (!table[i].ChildCommands.empty())
                if (ShowHelpForSubCommands(table[i].ChildCommands, "", ""))
                    return true;

            return !table[i].Help.empty();
        }
    }

    return ShowHelpForSubCommands(table, "", cmd);
}

ChatHandler::ChatHandler(Player* player)
    : ChatHandler(player->GetSession())
{
}

size_t ChatHandler::BuildChatPacket(WorldPacket& data, ChatMsg chatType, Language language, ObjectGuid senderGUID, ObjectGuid receiverGUID, std::string const& message, uint8 chatTag,
                                  std::string const& senderName /*= ""*/, std::string const& receiverName /*= ""*/,
                                  uint32 achievementId /*= 0*/, bool gmMessage /*= false*/, std::string const& channelName /*= ""*/,
                                  std::string const& addonPrefix /*= ""*/)
{
    size_t receiverGUIDPos = 0;
    bool hasAchievementId = (chatType == CHAT_MSG_ACHIEVEMENT || chatType == CHAT_MSG_GUILD_ACHIEVEMENT) && achievementId;
    bool hasLanguage = (language > Language::LANG_UNIVERSAL || language == Language::LANG_ADDON);
    bool hasSenderName = false;
    bool hasReceiverName = false;
    bool hasChannelName = false;
    bool hasGroupGUID = false;
    bool hasGuildGUID = false;
    bool hasPrefix = false;

    switch (chatType)
    {
        case CHAT_MSG_PARTY:
        case CHAT_MSG_PARTY_LEADER:
        case CHAT_MSG_RAID:
        case CHAT_MSG_RAID_LEADER:
        case CHAT_MSG_RAID_WARNING:
            hasGroupGUID = true;
            break;
        case CHAT_MSG_GUILD:
        case CHAT_MSG_OFFICER:
        case CHAT_MSG_GUILD_ACHIEVEMENT:
            hasGuildGUID = true;
            break;
        case CHAT_MSG_MONSTER_WHISPER:
        case CHAT_MSG_RAID_BOSS_WHISPER:
            if (receiverGUID && !receiverGUID.IsPlayer() && !receiverGUID.IsPet())
                hasReceiverName = receiverName.length();
        case CHAT_MSG_MONSTER_SAY:
        case CHAT_MSG_MONSTER_PARTY:
        case CHAT_MSG_MONSTER_YELL:
        case CHAT_MSG_MONSTER_EMOTE:
        case CHAT_MSG_RAID_BOSS_EMOTE:
            hasSenderName = senderName.length();
            break;
        case CHAT_MSG_WHISPER_FOREIGN:
            hasSenderName = senderName.length();
            break;
        case CHAT_MSG_BG_SYSTEM_NEUTRAL:
        case CHAT_MSG_BG_SYSTEM_ALLIANCE:
        case CHAT_MSG_BG_SYSTEM_HORDE:
            if (receiverGUID && !receiverGUID.IsPlayer())
                hasReceiverName = receiverName.length();
            break;

        case CHAT_MSG_CHANNEL:
            hasChannelName = channelName.length();
            hasSenderName = senderName.length();
            break;
        default:
            if (gmMessage)
                hasSenderName = senderName.length();
            break;
    }

    if (language == LANG_ADDON)
        hasPrefix = addonPrefix.length();

    Player* sender = ObjectAccessor::FindPlayer(senderGUID);

    ObjectGuid guildGUID = hasGuildGUID && sender && sender->GetGuildId() ? ObjectGuid(HighGuid::Guild, sender->GetGuildId()) : ObjectGuid::Empty;
    ObjectGuid groupGUID = hasGroupGUID && sender && sender->GetGroup() ? sender->GetGroup()->GetGUID() : ObjectGuid::Empty;

    uint32 realmId1 = realm.Id.Realm;
    // uint32 realmId2 = realmID;

    data.Initialize(SMSG_MESSAGECHAT);
    data.WriteBit(!hasSenderName);
    data.WriteBit(0); // HideInChatLog - only bubble shows

    if (hasSenderName)
        data.WriteBits(senderName.length(), 11);

    data.WriteBit(0); // Fake Bit
    data.WriteBit(!hasChannelName);
    data.WriteBit(0); // Unk
    data.WriteBit(1); // SendFakeTime - float later
    data.WriteBit(!chatTag); // ChatFlags
    data.WriteBit(!realmId1); // RealmID ?

    data.WriteBit(groupGUID[0]);
    data.WriteBit(groupGUID[1]);
    data.WriteBit(groupGUID[5]);
    data.WriteBit(groupGUID[4]);
    data.WriteBit(groupGUID[3]);
    data.WriteBit(groupGUID[2]);
    data.WriteBit(groupGUID[6]);
    data.WriteBit(groupGUID[7]);

     if (chatTag)
        data.WriteBits(chatTag, 9);

    data.WriteBit(0); // Fake Bit

    receiverGUIDPos = data.bitwpos();

    data.WriteBit(receiverGUID[7]);
    data.WriteBit(receiverGUID[6]);
    data.WriteBit(receiverGUID[1]);
    data.WriteBit(receiverGUID[4]);
    data.WriteBit(receiverGUID[0]);
    data.WriteBit(receiverGUID[2]);
    data.WriteBit(receiverGUID[3]);
    data.WriteBit(receiverGUID[5]);

    data.WriteBit(0); // Fake Bit
    data.WriteBit(!hasLanguage);
    data.WriteBit(!hasPrefix);

    data.WriteBit(senderGUID[0]);
    data.WriteBit(senderGUID[3]);
    data.WriteBit(senderGUID[7]);
    data.WriteBit(senderGUID[2]);
    data.WriteBit(senderGUID[1]);
    data.WriteBit(senderGUID[5]);
    data.WriteBit(senderGUID[4]);
    data.WriteBit(senderGUID[6]);

    data.WriteBit(!hasAchievementId);
    data.WriteBit(!message.length());

    if (hasChannelName)
        data.WriteBits(channelName.length(), 7);

    if (message.length())
        data.WriteBits(message.length(), 12);

    data.WriteBit(!hasReceiverName);

    if (hasPrefix)
        data.WriteBits(addonPrefix.length(), 5);

    data.WriteBit(1); // RealmID ?

    if (hasReceiverName)
        data.WriteBits(receiverName.length(), 11);

    data.WriteBit(0); // Fake Bit

    data.WriteBit(guildGUID[2]);
    data.WriteBit(guildGUID[5]);
    data.WriteBit(guildGUID[7]);
    data.WriteBit(guildGUID[4]);
    data.WriteBit(guildGUID[0]);
    data.WriteBit(guildGUID[1]);
    data.WriteBit(guildGUID[3]);
    data.WriteBit(guildGUID[6]);

    data.FlushBits();

    data.WriteByteSeq(guildGUID[4]);
    data.WriteByteSeq(guildGUID[5]);
    data.WriteByteSeq(guildGUID[7]);
    data.WriteByteSeq(guildGUID[3]);
    data.WriteByteSeq(guildGUID[2]);
    data.WriteByteSeq(guildGUID[6]);
    data.WriteByteSeq(guildGUID[0]);
    data.WriteByteSeq(guildGUID[1]);

    if (hasChannelName)
        data.WriteString(channelName);

    if (hasPrefix)
        data.WriteString(addonPrefix);

    // if (hasFakeTime)
    //     data << float(fakeTime);

    data.WriteByteSeq(senderGUID[4]);
    data.WriteByteSeq(senderGUID[7]);
    data.WriteByteSeq(senderGUID[1]);
    data.WriteByteSeq(senderGUID[5]);
    data.WriteByteSeq(senderGUID[0]);
    data.WriteByteSeq(senderGUID[6]);
    data.WriteByteSeq(senderGUID[2]);
    data.WriteByteSeq(senderGUID[3]);

    data << uint8(chatType);

    if (hasAchievementId)
        data << uint32(achievementId);

    data.WriteByteSeq(groupGUID[1]);
    data.WriteByteSeq(groupGUID[3]);
    data.WriteByteSeq(groupGUID[4]);
    data.WriteByteSeq(groupGUID[6]);
    data.WriteByteSeq(groupGUID[0]);
    data.WriteByteSeq(groupGUID[2]);
    data.WriteByteSeq(groupGUID[5]);
    data.WriteByteSeq(groupGUID[7]);

    // receiverGUIDPos = data.bitwpos(); 

    data.WriteByteSeq(receiverGUID[2]);
    data.WriteByteSeq(receiverGUID[5]);
    data.WriteByteSeq(receiverGUID[3]);
    data.WriteByteSeq(receiverGUID[6]);
    data.WriteByteSeq(receiverGUID[7]);
    data.WriteByteSeq(receiverGUID[4]);
    data.WriteByteSeq(receiverGUID[1]);
    data.WriteByteSeq(receiverGUID[0]);

    if (hasLanguage)
        data << uint8(language);

    if (message.length())
        data.WriteString(message);

    if (hasReceiverName)
        data.WriteString(receiverName);

    if (hasSenderName)
        data.WriteString(senderName);

    return receiverGUIDPos;
}

size_t ChatHandler::BuildChatPacket(WorldPacket& data, ChatMsg chatType, Language language, WorldObject const* sender, WorldObject const* receiver, std::string const& message,
                                  uint32 achievementId /*= 0*/, std::string const& channelName /*= ""*/, LocaleConstant locale /*= DEFAULT_LOCALE*/, std::string const& addonPrefix /*= ""*/)
{
    ObjectGuid senderGUID = ObjectGuid::Empty;
    std::string senderName = "";
    uint8 chatTag = 0;
    bool gmMessage = false;
    ObjectGuid receiverGUID = ObjectGuid::Empty;
    std::string receiverName = "";
    if (sender)
    {
        senderGUID = sender->GetGUID();
        senderName = sender->GetNameForLocaleIdx(locale);
        if (Player const* playerSender = sender->ToPlayer())
        {
            chatTag = playerSender->GetChatTag();
            gmMessage = chatTag & CHAT_TAG_GM;
        }
    }

    if (receiver)
    {
        receiverGUID = receiver->GetGUID();
        receiverName = receiver->GetNameForLocaleIdx(locale);
    }

    return BuildChatPacket(data, chatType, language, senderGUID, receiverGUID, message, chatTag, senderName, receiverName, achievementId, gmMessage, channelName, addonPrefix);
}


Player* ChatHandler::getSelectedPlayer()
{
    if (!m_session)
        return NULL;

    ObjectGuid selected = m_session->GetPlayer()->GetTarget();
    if (!selected)
        return m_session->GetPlayer();

    return ObjectAccessor::FindPlayer(selected);
}

Unit* ChatHandler::getSelectedUnit()
{
    if (!m_session)
        return NULL;

    if (Unit* selected = m_session->GetPlayer()->GetSelectedUnit())
        return selected;

    return m_session->GetPlayer();
}

WorldObject* ChatHandler::getSelectedObject()
{
    if (!m_session)
        return NULL;

    ObjectGuid guid = m_session->GetPlayer()->GetTarget();

    if (guid == 0)
        return GetNearbyGameObject();

    return ObjectAccessor::GetUnit(*m_session->GetPlayer(), guid);
}

Creature* ChatHandler::getSelectedCreature()
{
    if (!m_session)
        return NULL;

    return ObjectAccessor::GetCreatureOrPetOrVehicle(*m_session->GetPlayer(), m_session->GetPlayer()->GetTarget());
}

char* ChatHandler::extractKeyFromLink(char* text, char const* linkType, char** something1)
{
    // skip empty
    if (!text)
        return NULL;

    // skip spaces
    while (*text == ' '||*text == '\t'||*text == '\b')
        ++text;

    if (!*text)
        return NULL;

    // return non link case
    if (text[0] != '|')
        return strtok(text, " ");

    // [name] Shift-click form |color|linkType:key|h[name]|h|r
    // or
    // [name] Shift-click form |color|linkType:key:something1:...:somethingN|h[name]|h|r

    char* check = strtok(text, "|");                        // skip color
    if (!check)
        return NULL;                                        // end of data

    char* cLinkType = strtok(NULL, ":");                    // linktype
    if (!cLinkType)
        return NULL;                                        // end of data

    if (strcmp(cLinkType, linkType) != 0)
    {
        strtok(NULL, " ");                                  // skip link tail (to allow continue strtok(NULL, s) use after retturn from function
        SendSysMessage(LANG_WRONG_LINK_TYPE);
        return NULL;
    }

    char* cKeys = strtok(NULL, "|");                        // extract keys and values
    char* cKeysTail = strtok(NULL, "");

    char* cKey = strtok(cKeys, ":|");                       // extract key
    if (something1)
        *something1 = strtok(NULL, ":|");                   // extract something

    strtok(cKeysTail, "]");                                 // restart scan tail and skip name with possible spaces
    strtok(NULL, " ");                                      // skip link tail (to allow continue strtok(NULL, s) use after return from function
    return cKey;
}

char* ChatHandler::extractKeyFromLink(char* text, char const* const* linkTypes, int* found_idx, char** something1)
{
    // skip empty
    if (!text)
        return NULL;

    // skip spaces
    while (*text == ' '||*text == '\t'||*text == '\b')
        ++text;

    if (!*text)
        return NULL;

    // return non link case
    if (text[0] != '|')
        return strtok(text, " ");

    // [name] Shift-click form |color|linkType:key|h[name]|h|r
    // or
    // [name] Shift-click form |color|linkType:key:something1:...:somethingN|h[name]|h|r
    // or
    // [name] Shift-click form |linkType:key|h[name]|h|r

    char* tail;

    if (text[1] == 'c')
    {
        char* check = strtok(text, "|");                    // skip color
        if (!check)
            return NULL;                                    // end of data

        tail = strtok(NULL, "");                            // tail
    }
    else
        tail = text+1;                                      // skip first |

    char* cLinkType = strtok(tail, ":");                    // linktype
    if (!cLinkType)
        return NULL;                                        // end of data

    for (int i = 0; linkTypes[i]; ++i)
    {
        if (strcmp(cLinkType, linkTypes[i]) == 0)
        {
            char* cKeys = strtok(NULL, "|");                // extract keys and values
            char* cKeysTail = strtok(NULL, "");

            char* cKey = strtok(cKeys, ":|");               // extract key
            if (something1)
                *something1 = strtok(NULL, ":|");           // extract something

            strtok(cKeysTail, "]");                         // restart scan tail and skip name with possible spaces
            strtok(NULL, " ");                              // skip link tail (to allow continue strtok(NULL, s) use after return from function
            if (found_idx)
                *found_idx = i;
            return cKey;
        }
    }

    strtok(NULL, " ");                                      // skip link tail (to allow continue strtok(NULL, s) use after return from function
    SendSysMessage(LANG_WRONG_LINK_TYPE);
    return NULL;
}

GameObject* ChatHandler::GetNearbyGameObject()
{
    if (!m_session)
        return NULL;

    Player* pl = m_session->GetPlayer();
    GameObject* obj = NULL;
    Trinity::NearestGameObjectCheck check(*pl);
    Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectCheck> searcher(pl, obj, check);
    pl->VisitNearbyGridObject(SIZE_OF_GRIDS, searcher);
    return obj;
}

GameObject* ChatHandler::GetObjectGlobalyWithGuidOrNearWithDbGuid(uint32 lowguid, uint32 entry)
{
    if (!m_session)
        return NULL;

    Player* pl = m_session->GetPlayer();

    GameObject* obj = pl->GetMap()->GetGameObject(ObjectGuid(HighGuid::GameObject, entry, lowguid));

    if (!obj && sObjectMgr->GetGOData(lowguid))                   // guid is DB guid of object
    {
        // search near player then
        CellCoord p(Trinity::ComputeCellCoord(pl->GetPositionX(), pl->GetPositionY()));
        Cell cell(p);

        Trinity::GameObjectWithDbGUIDCheck go_check(*pl, lowguid);
        Trinity::GameObjectSearcher<Trinity::GameObjectWithDbGUIDCheck> checker(pl, obj, go_check);

        TypeContainerVisitor<Trinity::GameObjectSearcher<Trinity::GameObjectWithDbGUIDCheck>, GridTypeMapContainer > object_checker(checker);
        cell.Visit(p, object_checker, *pl->GetMap(), *pl, pl->GetGridActivationRange());
    }

    return obj;
}

enum SpellLinkType
{
    SPELL_LINK_SPELL   = 0,
    SPELL_LINK_TALENT  = 1,
    SPELL_LINK_ENCHANT = 2,
    SPELL_LINK_TRADE   = 3,
    SPELL_LINK_GLYPH   = 4
};

static char const* const spellKeys[] =
{
    "Hspell",                                               // normal spell
    "Htalent",                                              // talent spell
    "Henchant",                                             // enchanting recipe spell
    "Htrade",                                               // profession/skill spell
    "Hglyph",                                               // glyph
    0
};

uint32 ChatHandler::extractSpellIdFromLink(char* text)
{
    // number or [name] Shift-click form |color|Henchant:recipe_spell_id|h[prof_name: recipe_name]|h|r
    // number or [name] Shift-click form |color|Hglyph:glyph_slot_id:glyph_prop_id|h[%s]|h|r
    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    // number or [name] Shift-click form |color|Htalent:talent_id, rank|h[name]|h|r
    // number or [name] Shift-click form |color|Htrade:spell_id, skill_id, max_value, cur_value|h[name]|h|r
    int type = 0;
    char* param1_str = NULL;
    char* idS = extractKeyFromLink(text, spellKeys, &type, &param1_str);
    if (!idS)
        return 0;

    uint32 id = (uint32)atol(idS);

    switch (type)
    {
        case SPELL_LINK_SPELL:
            return id;
        case SPELL_LINK_TALENT:
        {
            // talent
            TalentEntry const* talentEntry = sTalentStore.LookupEntry(id);
            if (!talentEntry)
                return 0;

            return talentEntry->SpellId;
        }
        case SPELL_LINK_ENCHANT:
        case SPELL_LINK_TRADE:
            return id;
        case SPELL_LINK_GLYPH:
        {
            uint32 glyph_prop_id = param1_str ? (uint32)atol(param1_str) : 0;

            GlyphPropertiesEntry const* glyphPropEntry = sGlyphPropertiesStore.LookupEntry(glyph_prop_id);
            if (!glyphPropEntry)
                return 0;

            return glyphPropEntry->SpellId;
        }
    }

    // unknown type?
    return 0;
}

GameTele const* ChatHandler::extractGameTeleFromLink(char* text)
{
    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    char* cId = extractKeyFromLink(text, "Htele");
    if (!cId)
        return NULL;

    // id case (explicit or from shift link)
    if (cId[0] >= '0' || cId[0] >= '9')
        if (uint32 id = atoi(cId))
            return sObjectMgr->GetGameTele(id);

    return sObjectMgr->GetGameTele(cId);
}

enum GuidLinkType
{
    SPELL_LINK_PLAYER     = 0,                              // must be first for selection in not link case
    SPELL_LINK_CREATURE   = 1,
    SPELL_LINK_GAMEOBJECT = 2
};

static char const* const guidKeys[] =
{
    "Hplayer",
    "Hcreature",
    "Hgameobject",
    0
};

ObjectGuid ChatHandler::extractGuidFromLink(char* text)
{
    int type = 0;

    // |color|Hcreature:creature_guid|h[name]|h|r
    // |color|Hgameobject:go_guid|h[name]|h|r
    // |color|Hplayer:name|h[name]|h|r
    char* idS = extractKeyFromLink(text, guidKeys, &type);
    if (!idS)
        return ObjectGuid::Empty;

    switch (type)
    {
        case SPELL_LINK_PLAYER:
        {
            std::string name = idS;
            if (!normalizePlayerName(name))
                return ObjectGuid::Empty;

            if (Player* player = ObjectAccessor::FindPlayerByName(name))
                return player->GetGUID();

            return sObjectMgr->GetPlayerGUIDByName(name);
        }
        case SPELL_LINK_CREATURE:
        {
            ObjectGuid::LowType lowguid = (uint32)atol(idS);

            if (CreatureData const* data = sObjectMgr->GetCreatureData(lowguid))
                return ObjectGuid(HighGuid::Unit, data->id, lowguid);
            else
                return ObjectGuid::Empty;
        }
        case SPELL_LINK_GAMEOBJECT:
        {
            ObjectGuid::LowType lowguid = (uint32)atol(idS);

            if (GameObjectData const* data = sObjectMgr->GetGOData(lowguid))
                return ObjectGuid(HighGuid::GameObject, data->id, lowguid);
            else
                return ObjectGuid::Empty;
        }
    }

    // unknown type?
    return ObjectGuid::Empty;
}

std::string ChatHandler::extractPlayerNameFromLink(char* text)
{
    // |color|Hplayer:name|h[name]|h|r
    char* name_str = extractKeyFromLink(text, "Hplayer");
    if (!name_str)
        return "";

    std::string name = name_str;
    if (!normalizePlayerName(name))
        return "";

    return name;
}

bool ChatHandler::extractPlayerTarget(char* args, Player** player, ObjectGuid* player_guid /*=NULL*/, std::string* player_name /*= NULL*/)
{
    if (args && *args)
    {
        std::string name = extractPlayerNameFromLink(args);
        if (name.empty())
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        Player* pl = ObjectAccessor::FindPlayerByName(name);

        // if allowed player pointer
        if (player)
            *player = pl;

        // if need guid value from DB (in name case for check player existence)
        ObjectGuid guid = !pl && (player_guid || player_name) ? sObjectMgr->GetPlayerGUIDByName(name) : ObjectGuid::Empty;

        // if allowed player guid (if no then only online players allowed)
        if (player_guid)
            *player_guid = pl ? pl->GetGUID() : guid;

        if (player_name)
            *player_name = pl || guid ? name : "";
    }
    else
    {
        Player* pl = getSelectedPlayer();
        // if allowed player pointer
        if (player)
            *player = pl;
        // if allowed player guid (if no then only online players allowed)
        if (player_guid)
            *player_guid = pl ? pl->GetGUID() : ObjectGuid::Empty;

        if (player_name)
            *player_name = pl ? pl->GetName() : "";
    }

    // some from req. data must be provided (note: name is empty if player not exist)
    if ((!player || !*player) && (!player_guid || !*player_guid) && (!player_name || player_name->empty()))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

void ChatHandler::extractOptFirstArg(char* args, char** arg1, char** arg2)
{
    char* p1 = strtok(args, " ");
    char* p2 = strtok(NULL, " ");

    if (!p2)
    {
        p2 = p1;
        p1 = NULL;
    }

    if (arg1)
        *arg1 = p1;

    if (arg2)
        *arg2 = p2;
}

void ChatHandler::extractOptFirstArg(char* args, char** arg1, char** arg2, char** arg3)
{
    char* p1 = strtok(args, " ");
    char* p2 = strtok(NULL, " ");
    char* p3 = strtok(NULL, "");

    if (!p3)
    {
        p3 = p2;
        p2 = p1;
        p1 = NULL;
    }

    if (arg1)
        *arg1 = p1;

    if (arg2)
        *arg2 = p2;

    if (arg3)
        *arg3 = p3;
}

char* ChatHandler::extractQuotedArg(char* args)
{
    if (!*args)
        return NULL;

    if (*args == '"')
        return strtok(args+1, "\"");
    else
    {
        char* space = strtok(args, "\"");
        if (!space)
            return NULL;
        return strtok(NULL, "\"");
    }
}

bool ChatHandler::needReportToTarget(Player* chr) const
{
    Player* pl = m_session->GetPlayer();
    return pl != chr && pl->IsVisibleGloballyFor(chr);
}

LocaleConstant ChatHandler::GetSessionDbcLocale() const
{
    return m_session->GetSessionDbcLocale();
}

int ChatHandler::GetSessionDbLocaleIndex() const
{
    return m_session->GetSessionDbLocaleIndex();
}

std::string ChatHandler::GetNameLink(Player* chr) const
{
    return playerLink(chr->GetName());
}

// CommandHolder ChatHandler::CreateCommandHolder(TaskBase* task)
// {
//     GetSession()->NewAction(task);
//     return CommandHolder{ new ChatCommandHolder{ new ChatHandler{*this} } };
// }

// CommandHolder CliHandler::CreateCommandHolder(TaskBase* task)
// {
//     m_holder->Delay(new CliHandler{ *this });
//     return m_hptr;
// }

void CliCommandHolder::FinishCommand(bool success)
{
    if (m_commandFinished)
        m_commandFinished(m_callbackArg, success);
}

const char *CliHandler::GetTrinityString(int32 entry) const
{
    return sObjectMgr->GetTrinityStringForDBCLocale(entry);
}

bool CliHandler::isAvailable(ChatCommand const& cmd) const
{
    // skip non-console commands in console case
    return cmd.AllowConsole;
}

void CliHandler::SendSysMessage(const char *str)
{
    m_print(m_callbackArg, str);
    m_print(m_callbackArg, "\r\n");
}

std::string CliHandler::GetNameLink() const
{
    return GetTrinityString(LANG_CONSOLE_COMMAND);
}

bool CliHandler::needReportToTarget(Player* /*chr*/) const
{
    return true;
}

bool ChatHandler::GetPlayerGroupAndGUIDByName(const char* cname, Player* &player, Group* &group, ObjectGuid &guid, bool offline)
{
    player  = NULL;
    guid = ObjectGuid::Empty;

    if (cname)
    {
        std::string name = cname;
        if (!name.empty())
        {
            if (!normalizePlayerName(name))
            {
                PSendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }

            player = ObjectAccessor::FindPlayerByName(name);
            if (offline)
                guid = sObjectMgr->GetPlayerGUIDByName(name.c_str());
        }
    }

    if (player)
    {
        group = player->GetGroup();
        if (!guid || !offline)
            guid = player->GetGUID();
    }
    else
    {
        if (getSelectedPlayer())
            player = getSelectedPlayer();
        else
            player = m_session->GetPlayer();

        if (!guid || !offline)
            guid  = player->GetGUID();
        group = player->GetGroup();
    }

    return true;
}

void ChatHandler::SecureLevelAnnounce(AccountTypes sec, const char* args)
{
    if (!*args)
        return;

    char buff[2048];
    sprintf(buff, GetTrinityString(LANG_SYSTEMMESSAGE), args);

    WorldPacket data(SMSG_SERVER_MESSAGE, 100);
    data << uint32(SERVER_MSG_STRING);
    data << buff;

    sWorld->SendGlobalMessage(&data, sec);
}

bool ChatHandler::ValidatePipeSequence(std::string const& text)
{
    for (auto itr = text.begin(), end = text.end(); itr != end; ++itr)
        if (*itr == '|' && (++itr == end || *itr != '|'))
            return false;
    return true;
}

LocaleConstant CliHandler::GetSessionDbcLocale() const
{
    return sWorld->GetDefaultDbcLocale();
}

int CliHandler::GetSessionDbLocaleIndex() const
{
    return sObjectMgr->GetDBCLocaleIndex();
}
