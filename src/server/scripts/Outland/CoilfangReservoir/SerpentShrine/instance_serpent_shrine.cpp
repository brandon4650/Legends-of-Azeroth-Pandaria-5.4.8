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

/* ScriptData
SDName: Instance_Serpent_Shrine
SD%Complete: 100
SDComment: Instance Data Scripts and functions to acquire mobs and set encounter status for use in various Serpent Shrine Scripts
SDCategory: Coilfang Resevoir, Serpent Shrine Cavern
EndScriptData */

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "serpent_shrine.h"
#include "Player.h"
#include "TemporarySummon.h"

#define MAX_ENCOUNTER 6

enum Misc
{
    // Spells
    SPELL_SCALDINGWATER             = 37284,

    // Creatures
    NPC_COILFANG_FRENZY             = 21508,
    NPC_COILFANG_PRIESTESS          = 21220,
    NPC_COILFANG_SHATTERER          = 21301,

    // Misc
    MIN_KILLS                       = 30
};


//NOTE: there are 6 platforms
//there should be 3 shatterers and 2 priestess on all platforms, total of 30 elites, else it won't work!
//delete all other elites not on platforms! these mobs should only be on those platforms nowhere else.

/* Serpentshrine cavern encounters:
0 - Hydross The Unstable event
1 - Leotheras The Blind Event
2 - The Lurker Below Event
3 - Fathom-Lord Karathress Event
4 - Morogrim Tidewalker Event
5 - Lady Vashj Event
*/

class go_bridge_console : public GameObjectScript
{
    public:
        go_bridge_console() : GameObjectScript("go_bridge_console") { }

        bool OnGossipHello(Player* /*player*/, GameObject* go) override
        {
            InstanceScript* instance = go->GetInstanceScript();

            if (!instance)
                return false;

            if (instance)
                instance->SetData(DATA_CONTROL_CONSOLE, DONE);

            instance->HandleGameObject(go->GetGUID(), true);

            return true;
        }
};

class instance_serpent_shrine : public InstanceMapScript
{
    public:
        instance_serpent_shrine() : InstanceMapScript("instance_serpent_shrine", 548) { }

        struct instance_serpentshrine_cavern_InstanceMapScript : public InstanceScript
        {
            instance_serpentshrine_cavern_InstanceMapScript(Map* map) : InstanceScript(map)
            {
            }

            void Initialize() override
            {
                memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

                LurkerBelow = ObjectGuid::Empty;
                Sharkkis = ObjectGuid::Empty;
                Tidalvess = ObjectGuid::Empty;
                Caribdis = ObjectGuid::Empty;
                LadyVashj = ObjectGuid::Empty;
                Karathress = ObjectGuid::Empty;
                KarathressEvent_Starter = ObjectGuid::Empty;
                LeotherasTheBlind = ObjectGuid::Empty;
                LeotherasEventStarter = ObjectGuid::Empty;

                ControlConsole = ObjectGuid::Empty;
                BridgePart[0] = ObjectGuid::Empty;
                BridgePart[1] = ObjectGuid::Empty;
                BridgePart[2] = ObjectGuid::Empty;
                StrangePool = 0;
                Water = WATERSTATE_FRENZY;

                ShieldGeneratorDeactivated[0] = false;
                ShieldGeneratorDeactivated[1] = false;
                ShieldGeneratorDeactivated[2] = false;
                ShieldGeneratorDeactivated[3] = false;
                FishingTimer = 1000;
                WaterCheckTimer = 500;
                FrenzySpawnTimer = 2000;
                DoSpawnFrenzy = false;
                TrashCount = 0;

            }

            bool IsEncounterInProgress() const override
            {
                for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                    if (m_auiEncounter[i] == IN_PROGRESS)
                        return true;

                return false;
            }

            void Update(uint32 diff) override
            {
                //Water checks
                if (WaterCheckTimer <= diff)
                {
                    if (TrashCount >= MIN_KILLS)
                        Water = WATERSTATE_SCALDING;
                    else
                        Water = WATERSTATE_FRENZY;

                    Map::PlayerList const &PlayerList = instance->GetPlayers();
                    if (PlayerList.isEmpty())
                        return;
                    for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                    {
                        if (Player* player = i->GetSource())
                        {
                            if (player->IsAlive() && /*i->GetSource()->GetPositionZ() <= -21.434931f*/player->IsInWater())
                            {
                                if (Water == WATERSTATE_SCALDING)
                                {

                                    if (!player->HasAura(SPELL_SCALDINGWATER))
                                    {
                                        player->CastSpell(player, SPELL_SCALDINGWATER, true);
                                    }
                                } else if (Water == WATERSTATE_FRENZY)
                                {
                                    //spawn frenzy
                                    if (DoSpawnFrenzy)
                                    {
                                        if (Creature* frenzy = player->SummonCreature(NPC_COILFANG_FRENZY, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 2000))
                                        {
                                            frenzy->Attack(player, false);
                                            frenzy->SetSwim(true);
                                            frenzy->SetDisableGravity(true);
                                        }
                                        DoSpawnFrenzy = false;
                                    }
                                }
                            }
                            if (!player->IsInWater())
                                player->RemoveAurasDueToSpell(SPELL_SCALDINGWATER);
                        }

                    }
                    WaterCheckTimer = 500;//remove stress from core
                }
                else
                    WaterCheckTimer -= diff;

                if (FrenzySpawnTimer <= diff)
                {
                    DoSpawnFrenzy = true;
                    FrenzySpawnTimer = 2000;
                }
                else
                    FrenzySpawnTimer -= diff;
            }

            void OnGameObjectCreate(GameObject* go) override
            {
                switch (go->GetEntry())
                {
                    case 184568:
                        ControlConsole = go->GetGUID();
                        go->setActive(true);
                        break;
                    case 184203:
                        BridgePart[0] = go->GetGUID();
                        go->setActive(true);
                        break;
                    case 184204:
                        BridgePart[1] = go->GetGUID();
                        go->setActive(true);
                        break;
                    case 184205:
                        BridgePart[2] = go->GetGUID();
                        go->setActive(true);
                        break;
                    default:
                        break;
                }
            }

            void OnCreatureCreate(Creature* creature) override
            {
                switch (creature->GetEntry())
                {
                    case 21212:
                        LadyVashj = creature->GetGUID();
                        break;
                    case 21214:
                        Karathress = creature->GetGUID();
                        break;
                    case 21966:
                        Sharkkis = creature->GetGUID();
                        break;
                    case 21217:
                        LurkerBelow = creature->GetGUID();
                        break;
                    case 21965:
                        Tidalvess = creature->GetGUID();
                        break;
                    case 21964:
                        Caribdis = creature->GetGUID();
                        break;
                    case 21215:
                        LeotherasTheBlind = creature->GetGUID();
                        break;
                    default:
                        break;
                }
            }

            void SetGuidData(uint32 type, ObjectGuid data) override
            {
                if (type == DATA_KARATHRESSEVENT_STARTER)
                    KarathressEvent_Starter = data;
                if (type == DATA_LEOTHERAS_EVENT_STARTER)
                    LeotherasEventStarter = data;
            }

            ObjectGuid GetGuidData(uint32 identifier) const override
            {
                switch (identifier)
                {
                    case DATA_THELURKERBELOW:
                        return LurkerBelow;
                    case DATA_SHARKKIS:
                        return Sharkkis;
                    case DATA_TIDALVESS:
                        return Tidalvess;
                    case DATA_CARIBDIS:
                        return Caribdis;
                    case DATA_LADYVASHJ:
                        return LadyVashj;
                    case DATA_KARATHRESS:
                        return Karathress;
                    case DATA_KARATHRESSEVENT_STARTER:
                        return KarathressEvent_Starter;
                    case DATA_LEOTHERAS:
                        return LeotherasTheBlind;
                    case DATA_LEOTHERAS_EVENT_STARTER:
                        return LeotherasEventStarter;
                    default:
                        break;
                }
                return ObjectGuid::Empty;
            }

            void SetData(uint32 type, uint32 data) override
            {
                switch (type)
                {
                    case DATA_STRANGE_POOL:
                        StrangePool = data;
                        break;
                    case DATA_CONTROL_CONSOLE:
                        if (data == DONE)
                        {
                            HandleGameObject(BridgePart[0], true);
                            HandleGameObject(BridgePart[1], true);
                            HandleGameObject(BridgePart[2], true);
                        }
                        ControlConsole = ObjectGuid(uint64(data));
                        break;
                    case DATA_TRASH:
                        if (data == 1 && TrashCount < MIN_KILLS)
                            ++TrashCount;//+1 died
                        SaveToDB();
                        break;
                    case DATA_WATER:
                        Water = data;
                        break;
                    case DATA_HYDROSSTHEUNSTABLEEVENT:
                        m_auiEncounter[0] = data;
                        break;
                    case DATA_LEOTHERASTHEBLINDEVENT:
                        m_auiEncounter[1] = data;
                        break;
                    case DATA_THELURKERBELOWEVENT:
                        m_auiEncounter[2] = data;
                        break;
                    case DATA_KARATHRESSEVENT:
                        m_auiEncounter[3] = data;
                        break;
                    case DATA_MOROGRIMTIDEWALKEREVENT:
                        m_auiEncounter[4] = data;
                        break;
                        //Lady Vashj
                    case DATA_LADYVASHJEVENT:
                        if (data == NOT_STARTED)
                        {
                            ShieldGeneratorDeactivated[0] = false;
                            ShieldGeneratorDeactivated[1] = false;
                            ShieldGeneratorDeactivated[2] = false;
                            ShieldGeneratorDeactivated[3] = false;
                        }
                        m_auiEncounter[5] = data;
                        break;
                    case DATA_SHIELDGENERATOR1:
                        ShieldGeneratorDeactivated[0] = data != 0;
                        break;
                    case DATA_SHIELDGENERATOR2:
                        ShieldGeneratorDeactivated[1] = data != 0;
                        break;
                    case DATA_SHIELDGENERATOR3:
                        ShieldGeneratorDeactivated[2] = data != 0;
                        break;
                    case DATA_SHIELDGENERATOR4:
                        ShieldGeneratorDeactivated[3] = data != 0;
                        break;
                    default:
                        break;
                }

                if (data == DONE)
                    SaveToDB();
            }

            uint32 GetData(uint32 type) const override
            {
                switch (type)
                {
                    case DATA_HYDROSSTHEUNSTABLEEVENT:
                        return m_auiEncounter[0];
                    case DATA_LEOTHERASTHEBLINDEVENT:
                        return m_auiEncounter[1];
                    case DATA_THELURKERBELOWEVENT:
                        return m_auiEncounter[2];
                    case DATA_KARATHRESSEVENT:
                        return m_auiEncounter[3];
                    case DATA_MOROGRIMTIDEWALKEREVENT:
                        return m_auiEncounter[4];
                        //Lady Vashj
                    case DATA_LADYVASHJEVENT:
                        return m_auiEncounter[5];
                    case DATA_SHIELDGENERATOR1:
                        return ShieldGeneratorDeactivated[0];
                    case DATA_SHIELDGENERATOR2:
                        return ShieldGeneratorDeactivated[1];
                    case DATA_SHIELDGENERATOR3:
                        return ShieldGeneratorDeactivated[2];
                    case DATA_SHIELDGENERATOR4:
                        return ShieldGeneratorDeactivated[3];
                    case DATA_CANSTARTPHASE3:
                        if (ShieldGeneratorDeactivated[0] && ShieldGeneratorDeactivated[1] && ShieldGeneratorDeactivated[2] && ShieldGeneratorDeactivated[3])
                            return 1;
                        break;
                    case DATA_STRANGE_POOL:
                        return StrangePool;
                    case DATA_WATER:
                        return Water;
                    default:
                        break;
                }

                return 0;
            }

            std::string GetSaveData() override
            {
                OUT_SAVE_INST_DATA;
                std::ostringstream stream;
                stream << m_auiEncounter[0] << ' ' << m_auiEncounter[1] << ' ' << m_auiEncounter[2] << ' '
                    << m_auiEncounter[3] << ' ' << m_auiEncounter[4] << ' ' << m_auiEncounter[5] << ' ' << TrashCount;
                OUT_SAVE_INST_DATA_COMPLETE;
                return stream.str();
            }

            void Load(const char* in) override
            {
                if (!in)
                {
                    OUT_LOAD_INST_DATA_FAIL;
                    return;
                }

                OUT_LOAD_INST_DATA(in);
                std::istringstream stream(in);
                stream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
                    >> m_auiEncounter[4] >> m_auiEncounter[5] >> TrashCount;

                for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                    if (m_auiEncounter[i] == IN_PROGRESS)                // Do not load an encounter as "In Progress" - reset it instead.
                        m_auiEncounter[i] = NOT_STARTED;

                OUT_LOAD_INST_DATA_COMPLETE;
            }

        private:
            ObjectGuid LurkerBelow;
            ObjectGuid Sharkkis;
            ObjectGuid Tidalvess;
            ObjectGuid Caribdis;
            ObjectGuid LadyVashj;
            ObjectGuid Karathress;
            ObjectGuid KarathressEvent_Starter;
            ObjectGuid LeotherasTheBlind;
            ObjectGuid LeotherasEventStarter;

            ObjectGuid ControlConsole;
            ObjectGuid BridgePart[3];
            uint32 StrangePool;
            uint32 FishingTimer;
            uint32 WaterCheckTimer;
            uint32 FrenzySpawnTimer;
            uint32 Water;
            uint32 TrashCount;

            bool ShieldGeneratorDeactivated[4];
            uint32 m_auiEncounter[MAX_ENCOUNTER];
            bool DoSpawnFrenzy;
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const override
        {
            return new instance_serpentshrine_cavern_InstanceMapScript(map);
        }
};

void AddSC_instance_serpentshrine_cavern()
{
    new instance_serpent_shrine();
    new go_bridge_console();
}
