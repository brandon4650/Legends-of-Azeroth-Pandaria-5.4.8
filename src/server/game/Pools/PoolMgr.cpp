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

#include "PoolMgr.h"
#include "Containers.h"
#include "GameEventMgr.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "MapManager.h"

////////////////////////////////////////////////////////////
// template class ActivePoolData

ActivePoolData::ActivePoolData(Map* owner) : mOwner(owner) { }
ActivePoolData::~ActivePoolData() = default;

// Method that tell amount spawned objects/subpools
uint32 ActivePoolData::GetActiveObjectCount(uint32 pool_id) const
{
    ActivePoolPools::const_iterator itr = mSpawnedPools.find(pool_id);
    return itr != mSpawnedPools.end() ? itr->second : 0;
}

// Method that tell if a creature is spawned currently
template<>
bool ActivePoolData::IsActiveObject<Creature>(uint32 db_guid) const
{
    return mSpawnedCreatures.find(db_guid) != mSpawnedCreatures.end();
}

// Method that tell if a gameobject is spawned currently
template<>
bool ActivePoolData::IsActiveObject<GameObject>(uint32 db_guid) const
{
    return mSpawnedGameobjects.find(db_guid) != mSpawnedGameobjects.end();
}

// Method that tell if a pool is spawned currently
template<>
bool ActivePoolData::IsActiveObject<Pool>(uint32 sub_pool_id) const
{
    return mSpawnedPools.find(sub_pool_id) != mSpawnedPools.end();
}

// Method that tell if a quest can be started
template<>
bool ActivePoolData::IsActiveObject<Quest>(uint32 quest_id) const
{
    return mActiveQuests.find(quest_id) != mActiveQuests.end();
}

template<>
void ActivePoolData::ActivateObject<Creature>(uint32 db_guid, uint32 pool_id)
{
    mSpawnedCreatures.insert(db_guid);
    ++mSpawnedPools[pool_id];
}

template<>
void ActivePoolData::ActivateObject<GameObject>(uint32 db_guid, uint32 pool_id)
{
    mSpawnedGameobjects.insert(db_guid);
    ++mSpawnedPools[pool_id];
}

template<>
void ActivePoolData::ActivateObject<Pool>(uint32 sub_pool_id, uint32 pool_id)
{
    mSpawnedPools[sub_pool_id] = 0;
    ++mSpawnedPools[pool_id];
}

template<>
void ActivePoolData::ActivateObject<Quest>(uint32 quest_id, uint32 pool_id)
{
    mActiveQuests.insert(quest_id);
    ++mSpawnedPools[pool_id];
}

template<>
void ActivePoolData::RemoveObject<Creature>(uint32 db_guid, uint32 pool_id)
{
    mSpawnedCreatures.erase(db_guid);
    uint32& val = mSpawnedPools[pool_id];
    if (val > 0)
        --val;
}

template<>
void ActivePoolData::RemoveObject<GameObject>(uint32 db_guid, uint32 pool_id)
{
    mSpawnedGameobjects.erase(db_guid);
    uint32& val = mSpawnedPools[pool_id];
    if (val > 0)
        --val;
}

template<>
void ActivePoolData::RemoveObject<Pool>(uint32 sub_pool_id, uint32 pool_id)
{
    mSpawnedPools.erase(sub_pool_id);
    uint32& val = mSpawnedPools[pool_id];
    if (val > 0)
        --val;
}

template<>
void ActivePoolData::RemoveObject<Quest>(uint32 quest_id, uint32 pool_id)
{
    mActiveQuests.erase(quest_id);
    uint32& val = mSpawnedPools[pool_id];
    if (val > 0)
        --val;
}

////////////////////////////////////////////////////////////
// Methods of template class PoolGroup

template <class T>
bool PoolGroup<T>::isEmptyDeepCheck() const
{
    return isEmpty();
}

template <>
bool PoolGroup<Pool>::isEmptyDeepCheck() const
{
    for (PoolObject const& explicitlyChanced : ExplicitlyChanced)
        if (!sPoolMgr->IsEmpty(explicitlyChanced.guid))
            return false;

    for (PoolObject const& equalChanced : EqualChanced)
        if (!sPoolMgr->IsEmpty(equalChanced.guid))
            return false;

    return true;
}

// Method to add a gameobject/creature guid to the proper list depending on pool type and chance value
template <class T>
void PoolGroup<T>::AddEntry(PoolObject& poolitem, uint32 maxentries)
{
    if (poolitem.chance != 0 && maxentries == 1)
        ExplicitlyChanced.push_back(poolitem);
    else
        EqualChanced.push_back(poolitem);
}

// Method to add a quest id to the proper list. Both equal and explicit chanced objects are stored in the same list due to us reusing them in bulk.
template<>
void PoolGroup<Quest>::AddEntry(PoolObject& poolitem, uint32 /*maxentries*/)
{
    EqualChanced.push_back(poolitem);
}

// Method to check the chances are proper in this object pool
template <class T>
bool PoolGroup<T>::CheckPool() const
{
    if (EqualChanced.empty())
    {
        float chance = 0;
        for (uint32 i = 0; i < ExplicitlyChanced.size(); ++i)
            chance += ExplicitlyChanced[i].chance;
        if (chance != 100 && chance != 0)
            return false;
    }
    return true;
}

template <class T>
PoolObject* PoolGroup<T>::RollOne(ActivePoolData& spawns, uint32 triggerFrom)
{
    if (!ExplicitlyChanced.empty())
    {
        float roll = (float)rand_chance();

        for (uint32 i = 0; i < ExplicitlyChanced.size(); ++i)
        {
            roll -= ExplicitlyChanced[i].chance;
            // Triggering object is marked as spawned at this time and can be also rolled (respawn case)
            // so this need explicit check for this case
            if (roll < 0 && (ExplicitlyChanced[i].guid == triggerFrom || !spawns.IsActiveObject<T>(ExplicitlyChanced[i].guid)))
               return &ExplicitlyChanced[i];
        }
    }
    if (!EqualChanced.empty())
    {
        int32 index = irand(0, EqualChanced.size()-1);
        // Triggering object is marked as spawned at this time and can be also rolled (respawn case)
        // so this need explicit check for this case
        if (EqualChanced[index].guid == triggerFrom || !spawns.IsActiveObject<T>(EqualChanced[index].guid))
           return &EqualChanced[index];
    }

    return NULL;
}

// Main method to despawn a creature or gameobject in a pool
// If no guid is passed, the pool is just removed (event end case)
// If guid is filled, cache will be used and no removal will occur, it just fill the cache
template<class T>
void PoolGroup<T>::DespawnObject(ActivePoolData& spawns, uint32 guid)
{
    for (size_t i=0; i < EqualChanced.size(); ++i)
    {
        // if spawned
        if (spawns.IsActiveObject<T>(EqualChanced[i].guid))
        {
            if (!guid || EqualChanced[i].guid == guid)
            {
                Despawn1Object(spawns, EqualChanced[i].guid);
                spawns.RemoveObject<T>(EqualChanced[i].guid, poolId);
            }
        }
    }

    for (size_t i = 0; i < ExplicitlyChanced.size(); ++i)
    {
        // spawned
        if (spawns.IsActiveObject<T>(ExplicitlyChanced[i].guid))
        {
            if (!guid || ExplicitlyChanced[i].guid == guid)
            {
                Despawn1Object(spawns, ExplicitlyChanced[i].guid);
                spawns.RemoveObject<T>(ExplicitlyChanced[i].guid, poolId);
            }
        }
    }
}

// Method that is actualy doing the removal job on one creature
template<>
void PoolGroup<Creature>::Despawn1Object(ActivePoolData& spawns, ObjectGuid::LowType guid)
{
    auto creatureBounds = spawns.GetMap()->GetCreatureBySpawnIdStore().equal_range(guid);
    for (auto itr = creatureBounds.first; itr != creatureBounds.second;)
    {
        Creature* creature = itr->second;
        ++itr;
//        // For dynamic spawns, save respawn time here
//        if (saveRespawnTime && !creature->GetRespawnCompatibilityMode())
//            creature->SaveRespawnTime();
        creature->AddObjectToRemoveList();
    }
}

// Same on one gameobject
template<>
void PoolGroup<GameObject>::Despawn1Object(ActivePoolData& spawns, ObjectGuid::LowType guid)
{
    auto gameobjectBounds = spawns.GetMap()->GetGameObjectBySpawnIdStore().equal_range(guid);
    for (auto itr = gameobjectBounds.first; itr != gameobjectBounds.second;)
    {
        GameObject* go = itr->second;
        ++itr;

//        // For dynamic spawns, save respawn time here
//        if (saveRespawnTime && !go->GetRespawnCompatibilityMode())
//            go->SaveRespawnTime();
        go->AddObjectToRemoveList();
    }
}

// Same on one pool
template<>
void PoolGroup<Pool>::Despawn1Object(ActivePoolData& spawns, uint32 child_pool_id)
{
    sPoolMgr->DespawnPool(spawns, child_pool_id);
}

// Method for a pool only to remove any found record causing a circular dependency loop
template<>
void PoolGroup<Pool>::RemoveOneRelation(uint32 child_pool_id)
{
    for (PoolObjectList::iterator itr = ExplicitlyChanced.begin(); itr != ExplicitlyChanced.end(); ++itr)
    {
        if (itr->guid == child_pool_id)
        {
            ExplicitlyChanced.erase(itr);
            break;
        }
    }
    for (PoolObjectList::iterator itr = EqualChanced.begin(); itr != EqualChanced.end(); ++itr)
    {
        if (itr->guid == child_pool_id)
        {
            EqualChanced.erase(itr);
            break;
        }
    }
}

template <class T>
void PoolGroup<T>::SpawnObject(ActivePoolData& spawns, uint32 limit, uint32 triggerFrom)
{
    uint32 lastDespawned = 0;
    int count = limit - spawns.GetActiveObjectCount(poolId);

    // If triggered from some object respawn this object is still marked as spawned
    // and also counted into m_SpawnedPoolAmount so we need increase count to be
    // spawned by 1
    if (triggerFrom)
        ++count;

    // This will try to spawn the rest of pool, not guaranteed
    for (int i = 0; i < count; ++i)
    {
        PoolObject* obj = RollOne(spawns, triggerFrom);
        if (!obj)
            continue;
        if (obj->guid == lastDespawned)
            continue;

        if (obj->guid == triggerFrom)
        {
            ReSpawn1Object(spawns, obj);
            triggerFrom = 0;
            continue;
        }
        spawns.ActivateObject<T>(obj->guid, poolId);
        Spawn1Object(spawns, obj);

        if (triggerFrom)
        {
            // One spawn one despawn no count increase
            DespawnObject(spawns, triggerFrom);
            lastDespawned = triggerFrom;
            triggerFrom = 0;
        }
    }
}

// Method that is actualy doing the spawn job on 1 creature
template <>
void PoolGroup<Creature>::Spawn1Object(ActivePoolData& spawns, PoolObject* obj)
{
    if (CreatureData const* data = sObjectMgr->GetCreatureData(obj->guid))
    {
        if (data->gameEventId && !sGameEventMgr->IsActiveEvent(data->gameEventId))
            return;

        sObjectMgr->AddCreatureToGrid(obj->guid, data);

        // Spawn if necessary (loaded grids only)
        // We use spawn coords to spawn
        if (spawns.GetMap()->IsGridLoaded(data->posX, data->posY))
        {
            Creature* creature = new Creature;
            //TC_LOG_DEBUG("pool", "Spawning creature %u", guid);
            if (!creature->LoadCreatureFromDB(obj->guid, spawns.GetMap()))
            {
                delete creature;
                return;
            }
        }
    }
}

// Same for 1 gameobject
template <>
void PoolGroup<GameObject>::Spawn1Object(ActivePoolData& spawns, PoolObject* obj)
{
    if (GameObjectData const* data = sObjectMgr->GetGOData(obj->guid))
    {
        if (data->gameEventId && !sGameEventMgr->IsActiveEvent(data->gameEventId))
            return;

        sObjectMgr->AddGameobjectToGrid(obj->guid, data);

        // Spawn if necessary (loaded grids only)
        // We use current coords to unspawn, not spawn coords since creature can have changed grid
        if (spawns.GetMap()->IsGridLoaded(data->posX, data->posY))
        {
            GameObject* go = new GameObject;
            //TC_LOG_DEBUG("pool", "Spawning gameobject %u", guid);
            if (!go->LoadGameObjectFromDB(obj->guid, spawns.GetMap(), false))
            {
                delete go;
                return;
            }
            else
            {
                if (go->isSpawnedByDefault())
                    if (!spawns.GetMap()->AddToMap(go))
                        delete go;
            }
        }
    }
}

// Same for 1 pool
template <>
void PoolGroup<Pool>::Spawn1Object(ActivePoolData& spawns, PoolObject* obj)
{
    sPoolMgr->SpawnPool(spawns, obj->guid);
}

// Method that does the respawn job on the specified creature
template <>
void PoolGroup<Creature>::ReSpawn1Object(ActivePoolData& spawns, PoolObject* obj)
{
    if (CreatureData const* data = sObjectMgr->GetCreatureData(obj->guid))
        if (Creature* creature = spawns.GetMap()->GetCreature(ObjectGuid::Create<HighGuid::Unit>(data->id, obj->guid)))
            creature->GetMap()->AddToMap(creature);
}

// Method that does the respawn job on the specified gameobject
template <>
void PoolGroup<GameObject>::ReSpawn1Object(ActivePoolData& spawns, PoolObject* obj)
{
    if (GameObjectData const* data = sObjectMgr->GetGOData(obj->guid))
        if (GameObject* pGameobject = spawns.GetMap()->GetGameObject(ObjectGuid::Create<HighGuid::GameObject>(data->id, obj->guid)))
            pGameobject->GetMap()->AddToMap(pGameobject);
}

// Nothing to do for a child Pool
template <>
void PoolGroup<Pool>::ReSpawn1Object(ActivePoolData& spawns, PoolObject* /*obj*/) { }

// Nothing to do for a quest
template <>
void PoolGroup<Quest>::ReSpawn1Object(ActivePoolData& spawns, PoolObject* /*obj*/) { }

////////////////////////////////////////////////////////////
// Methods of class PoolMgr

PoolMgr::PoolMgr() = default;
PoolMgr::~PoolMgr() = default;

PoolMgr* PoolMgr::instance()
{
    static PoolMgr instance;
    return &instance;
}

void PoolMgr::Initialize()
{
    mGameobjectSearchMap.clear();
    mCreatureSearchMap.clear();
}

void PoolMgr::LoadFromDB()
{
    // Pool templates
    {
        uint32 oldMSTime = getMSTime();

        QueryResult result = WorldDatabase.Query("SELECT entry, max_limit FROM pool_template");
        if (!result)
        {
            mPoolTemplate.clear();
            TC_LOG_INFO("server.loading", ">> Loaded 0 object pools. DB table `pool_template` is empty.");
            return;
        }

        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();

            uint32 pool_id = fields[0].GetUInt32();

            PoolTemplateData& pPoolTemplate = mPoolTemplate[pool_id];
            pPoolTemplate.MaxLimit  = fields[1].GetUInt32();
            pPoolTemplate.MapId = -1;

            ++count;
        }
        while (result->NextRow());

        TC_LOG_INFO("server.loading", ">> Loaded %u objects pools in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }

    // Creatures

    TC_LOG_INFO("server.loading", "Loading Creatures Pooling Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                 1       2         3
        QueryResult result = WorldDatabase.Query("SELECT guid, pool_entry, chance FROM pool_creature");

        if (!result)
        {
            TC_LOG_INFO("server.loading", ">> Loaded 0 creatures in  pools. DB table `pool_creature` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint32 guid    = fields[0].GetUInt32();
                uint32 pool_id = fields[1].GetUInt32();
                float chance   = fields[2].GetFloat();

                CreatureData const* data = sObjectMgr->GetCreatureData(guid);
                if (!data)
                {
                    TC_LOG_ERROR("sql.sql", "`pool_creature` has a non existing creature spawn (GUID: %u) defined for pool id (%u), skipped.", guid, pool_id);
                    continue;
                }
                auto it = mPoolTemplate.find(pool_id);
                if (it == mPoolTemplate.end())
                {
                    TC_LOG_ERROR("sql.sql", "`pool_creature` pool id (%u) is not in `pool_template`, skipped.", pool_id);
                    continue;
                }
                if (chance < 0 || chance > 100)
                {
                    TC_LOG_ERROR("sql.sql", "`pool_creature` has an invalid chance (%f) for creature guid (%u) in pool id (%u), skipped.", chance, guid, pool_id);
                    continue;
                }
                PoolTemplateData* pPoolTemplate = &mPoolTemplate[pool_id];
                if (pPoolTemplate->MapId == -1)
                    pPoolTemplate->MapId = int32(data->mapId);

                if (pPoolTemplate->MapId != int32(data->mapId))
                {
                    TC_LOG_ERROR("sql.sql", "`pool_creature` has creature spawns on multiple different maps for creature guid %u in pool id (%u), skipped.", guid, pool_id);
                    continue;
                }

                PoolObject plObject = PoolObject(guid, chance);
                PoolGroup<Creature>& cregroup = mPoolCreatureGroups[pool_id];
                cregroup.SetPoolId(pool_id);
                cregroup.AddEntry(plObject, pPoolTemplate->MaxLimit);
                mCreatureSearchMap.emplace(guid, pool_id);

                ++count;
            }
            while (result->NextRow());

            TC_LOG_INFO("server.loading", ">> Loaded %u creatures in pools in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // Gameobjects

    TC_LOG_INFO("server.loading", "Loading Gameobject Pooling Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                 1        2         3
        QueryResult result = WorldDatabase.Query("SELECT guid, pool_entry, chance FROM pool_gameobject");

        if (!result)
        {
            TC_LOG_INFO("server.loading", ">> Loaded 0 gameobjects in  pools. DB table `pool_gameobject` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint32 guid    = fields[0].GetUInt32();
                uint32 pool_id = fields[1].GetUInt32();
                float chance   = fields[2].GetFloat();

                GameObjectData const* data = sObjectMgr->GetGOData(guid);
                if (!data)
                {
                    TC_LOG_ERROR("sql.sql", "`pool_gameobject` has a non existing gameobject spawn (GUID: %u) defined for pool id (%u), skipped.", guid, pool_id);
                    continue;
                }

                GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(data->id);
                if (goinfo->type != GAMEOBJECT_TYPE_CHEST &&
                    goinfo->type != GAMEOBJECT_TYPE_GOOBER &&
                    goinfo->type != GAMEOBJECT_TYPE_FISHINGHOLE)
                {
                    TC_LOG_ERROR("sql.sql", "`pool_gameobject` has a not lootable gameobject spawn (GUID: %u, type: %u) defined for pool id (%u), skipped.", guid, goinfo->type, pool_id);
                    continue;
                }

                auto it = mPoolTemplate.find(pool_id);
                if (it == mPoolTemplate.end())
                {
                    TC_LOG_ERROR("sql.sql", "`pool_gameobject` pool id (%u) is not in `pool_template`, skipped.", pool_id);
                    continue;
                }

                if (chance < 0 || chance > 100)
                {
                    TC_LOG_ERROR("sql.sql", "`pool_gameobject` has an invalid chance (%f) for gameobject guid (%u) in pool id (%u), skipped.", chance, guid, pool_id);
                    continue;
                }

                PoolTemplateData* pPoolTemplate = &mPoolTemplate[pool_id];
                if (pPoolTemplate->MapId == -1)
                    pPoolTemplate->MapId = int32(data->mapid);

                if (pPoolTemplate->MapId != int32(data->mapid))
                {
                    TC_LOG_ERROR("sql.sql", "`pool_gameobject` has gameobject spawns on multiple different maps for gameobject guid (%u) in pool id (%u), skipped.", guid, pool_id);
                    continue;
                }

                PoolObject plObject = PoolObject(guid, chance);
                PoolGroup<GameObject>& gogroup = mPoolGameobjectGroups[pool_id];
                gogroup.SetPoolId(pool_id);
                gogroup.AddEntry(plObject, pPoolTemplate->MaxLimit);
                mGameobjectSearchMap.emplace(guid, pool_id);

                ++count;
            }
            while (result->NextRow());

            TC_LOG_INFO("server.loading", ">> Loaded %u gameobject in pools in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // Pool of pools

    TC_LOG_INFO("server.loading", "Loading Mother Pooling Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                  1        2            3
        QueryResult result = WorldDatabase.Query("SELECT pool_id, mother_pool, chance FROM pool_pool");

        if (!result)
        {
            TC_LOG_INFO("server.loading", ">> Loaded 0 pools in pools");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint32 child_pool_id  = fields[0].GetUInt32();
                uint32 mother_pool_id = fields[1].GetUInt32();
                float chance          = fields[2].GetFloat();

                {
                    auto it = mPoolTemplate.find(mother_pool_id);
                    if (it == mPoolTemplate.end())
                    {
                        TC_LOG_ERROR("sql.sql", "`pool_pool` mother_pool id (%u) is not in `pool_template`, skipped.", mother_pool_id);
                        continue;
                    }
                }
                {
                    auto it = mPoolTemplate.find(child_pool_id);
                    if (it == mPoolTemplate.end())
                    {
                        TC_LOG_ERROR("sql.sql", "`pool_pool` included pool_id (%u) is not in `pool_template`, skipped.", child_pool_id);
                        continue;
                    }
                }
                if (mother_pool_id == child_pool_id)
                {
                    TC_LOG_ERROR("sql.sql", "`pool_pool` pool_id (%u) includes itself, dead-lock detected, skipped.", child_pool_id);
                    continue;
                }
                if (chance < 0 || chance > 100)
                {
                    TC_LOG_ERROR("sql.sql", "`pool_pool` has an invalid chance (%f) for pool id (%u) in mother pool id (%u), skipped.", chance, child_pool_id, mother_pool_id);
                    continue;
                }
                PoolTemplateData* pPoolTemplateMother = &mPoolTemplate[mother_pool_id];
                PoolObject plObject = PoolObject(child_pool_id, chance);
                PoolGroup<Pool>& plgroup = mPoolPoolGroups[mother_pool_id];
                plgroup.SetPoolId(mother_pool_id);
                plgroup.AddEntry(plObject, pPoolTemplateMother->MaxLimit);
                mPoolSearchMap.emplace(child_pool_id, mother_pool_id);

                ++count;
            }
            while (result->NextRow());

            // Now check for circular reference
            // All pool_ids are in pool_template
            for (auto const it : mPoolTemplate)
            {
                std::set<uint32> checkedPools;
                for (SearchMap::iterator poolItr = mPoolSearchMap.find(it.first); poolItr != mPoolSearchMap.end(); poolItr = mPoolSearchMap.find(poolItr->second))
                {
                    if (mPoolTemplate[poolItr->first].MapId != -1)
                    {
                        if (mPoolTemplate[poolItr->second].MapId == -1)
                            mPoolTemplate[poolItr->second].MapId = mPoolTemplate[poolItr->first].MapId;

                        if (mPoolTemplate[poolItr->second].MapId != mPoolTemplate[poolItr->first].MapId)
                        {
                            TC_LOG_ERROR("sql.sql", "`pool_pool` has child pools on multiple maps in pool id (%u), skipped.", poolItr->second);
                            mPoolPoolGroups[poolItr->second].RemoveOneRelation(poolItr->first);
                            mPoolSearchMap.erase(poolItr);
                            --count;
                            break;
                        }
                    }

                    checkedPools.insert(poolItr->first);
                    if (checkedPools.find(poolItr->second) != checkedPools.end())
                    {
                        std::ostringstream ss;
                        ss<< "The pool(s) ";
                        for (std::set<uint32>::const_iterator itr=checkedPools.begin(); itr != checkedPools.end(); ++itr)
                            ss << *itr << ' ';
                        ss << "create(s) a circular reference, which can cause the server to freeze.\nRemoving the last link between mother pool "
                            << poolItr->first << " and child pool " << poolItr->second;
                        TC_LOG_ERROR("sql.sql", "%s", ss.str().c_str());
                        mPoolPoolGroups[poolItr->second].RemoveOneRelation(poolItr->first);
                        mPoolSearchMap.erase(poolItr);
                        --count;
                        break;
                    }
                }
            }

            TC_LOG_INFO("server.loading", ">> Loaded %u pools in mother pools in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    for (auto const& [poolId, templateData] : mPoolTemplate)
    {
        if (IsEmpty(poolId))
        {
            TC_LOG_ERROR("sql.sql", "Pool Id %u is empty (has no creatures and no gameobects and either no child pools or child pools are all empty. The pool will not be spawned", poolId);
            continue;
        }

        // TODO: GameObject: what does this break?
        //ASSERT(templateData.MapId != -1);
    }

    // The initialize method will spawn all pools not in an event and not in another pool, this is why there is 2 left joins with 2 null checks
    TC_LOG_INFO("server.loading", "Starting objects pooling system...");
    {
        uint32 oldMSTime = getMSTime();

        QueryResult result = WorldDatabase.Query("SELECT DISTINCT pool_template.entry, pool_pool.pool_id, pool_pool.mother_pool FROM pool_template"
            " LEFT JOIN game_event_pool ON pool_template.entry=game_event_pool.pool_entry"
            " LEFT JOIN pool_pool ON pool_template.entry=pool_pool.pool_id WHERE game_event_pool.pool_entry IS NULL");

        if (!result)
        {
            TC_LOG_INFO("server.loading", ">> Pool handling system initialized, 0 pools spawned.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32 pool_entry = fields[0].GetUInt32();
                uint32 pool_pool_id = fields[1].GetUInt32();

                if (!CheckPool(pool_entry))
                {
                    if (pool_pool_id)
                        // The pool is a child pool in pool_pool table. Ideally we should remove it from the pool handler to ensure it never gets spawned,
                        // however that could recursively invalidate entire chain of mother pools. It can be done in the future but for now we'll do nothing.
                        TC_LOG_ERROR("sql.sql", "Pool Id %u has no equal chance pooled entites defined and explicit chance sum is not 100. This broken pool is a child pool of Id %u and cannot be safely removed.", pool_entry, fields[2].GetUInt32());
                    else
                        TC_LOG_ERROR("sql.sql", "Pool Id %u has no equal chance pooled entites defined and explicit chance sum is not 100. The pool will not be spawned.", pool_entry);
                    continue;
                }

                // Don't spawn child pools, they are spawned recursively by their parent pools
                if (!pool_pool_id)
                {
                    mAutoSpawnPoolsPerMap[mPoolTemplate[pool_entry].MapId].push_back(pool_entry);
                    count++;
                }
            }
            while (result->NextRow());

            TC_LOG_DEBUG("pool", "Pool handling system initialized, %u pools spawned in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

        }
    }
}

// Call to spawn a pool, if cache if true the method will spawn only if cached entry is different
// If it's same, the creature is respawned only (added back to map)
template<>
void PoolMgr::SpawnPool<Creature>(ActivePoolData& activePoolData, uint32 pool_id, uint32 db_guid)
{
    auto it = mPoolCreatureGroups.find(pool_id);
    if (it != mPoolCreatureGroups.end() && !it->second.isEmpty())
        it->second.SpawnObject(activePoolData, mPoolTemplate[pool_id].MaxLimit, db_guid);
}

// Call to spawn a pool, if cache if true the method will spawn only if cached entry is different
// If it's same, the gameobject is respawned only (added back to map)
template<>
void PoolMgr::SpawnPool<GameObject>(ActivePoolData& activePoolData, uint32 pool_id, uint32 db_guid)
{
    auto it = mPoolGameobjectGroups.find(pool_id);
    if (it != mPoolGameobjectGroups.end() && !it->second.isEmpty())
        it->second.SpawnObject(activePoolData, mPoolTemplate[pool_id].MaxLimit, db_guid);
}

// Call to spawn a pool, if cache if true the method will spawn only if cached entry is different
// If it's same, the pool is respawned only
template<>
void PoolMgr::SpawnPool<Pool>(ActivePoolData& activePoolData, uint32 pool_id, uint32 sub_pool_id)
{
    auto it = mPoolPoolGroups.find(pool_id);
    if (it != mPoolPoolGroups.end() && !it->second.isEmpty())
        it->second.SpawnObject(activePoolData, mPoolTemplate[pool_id].MaxLimit, sub_pool_id);
}

void PoolMgr::SpawnPool(ActivePoolData& activePoolData, uint32 pool_id)
{
    SpawnPool<Pool>(activePoolData, pool_id, 0);
    SpawnPool<GameObject>(activePoolData, pool_id, 0);
    SpawnPool<Creature>(activePoolData, pool_id, 0);
}

// Call to despawn a pool, all gameobjects/creatures in this pool are removed
void PoolMgr::DespawnPool(ActivePoolData& activePoolData, uint32 pool_id)
{
    {
        auto it = mPoolCreatureGroups.find(pool_id);
        if (it != mPoolCreatureGroups.end() && !it->second.isEmpty())
            it->second.DespawnObject(activePoolData, 0);
    }
    {
        auto it = mPoolGameobjectGroups.find(pool_id);
        if (it != mPoolGameobjectGroups.end() && !it->second.isEmpty())
            it->second.DespawnObject(activePoolData, 0);
    }
    {
        auto it = mPoolPoolGroups.find(pool_id);
        if (it != mPoolPoolGroups.end() && !it->second.isEmpty())
            it->second.DespawnObject(activePoolData, 0);
    }
}

// Method that check chance integrity of the creatures and gameobjects in this pool
bool PoolMgr::CheckPool(uint32 pool_id) const
{
    {
        auto it = mPoolGameobjectGroups.find(pool_id);
        if (it != mPoolGameobjectGroups.end() && !it->second.CheckPool())
            return false;
    }
    {
        auto it = mPoolCreatureGroups.find(pool_id);
        if (it != mPoolCreatureGroups.end() && !it->second.CheckPool())
            return false;
    }
    {
        auto it = mPoolPoolGroups.find(pool_id);
        if (it != mPoolPoolGroups.end() && !it->second.CheckPool())
            return false;
    }
    return true;
}

bool PoolMgr::IsEmpty(uint32 pool_id) const
{
    {
        auto it = mPoolGameobjectGroups.find(pool_id);
        if (it != mPoolGameobjectGroups.end() && !it->second.isEmptyDeepCheck())
            return false;
    }
    {
        auto it = mPoolCreatureGroups.find(pool_id);
        if (it != mPoolCreatureGroups.end() && !it->second.isEmptyDeepCheck())
            return false;
    }
    {
        auto it = mPoolPoolGroups.find(pool_id);
        if (it != mPoolPoolGroups.end() && !it->second.isEmptyDeepCheck())
            return false;
    }
    return true;
}

// Call to update the pool when a gameobject/creature part of pool [pool_id] is ready to respawn
// Here we cache only the creature/gameobject whose guid is passed as parameter
// Then the spawn pool call will use this cache to decide
template<typename T>
void PoolMgr::UpdatePool(ActivePoolData& activePoolData, uint32 pool_id, uint32 db_guid_or_pool_id)
{
    if (uint32 motherpoolid = IsPartOfAPool<Pool>(pool_id))
        SpawnPool<Pool>(activePoolData, motherpoolid, pool_id);
    else
        SpawnPool<T>(activePoolData, pool_id, db_guid_or_pool_id);
}

template void PoolMgr::UpdatePool<Pool>(ActivePoolData& activePoolData, uint32 pool_id, uint32 db_guid_or_pool_id);
template void PoolMgr::UpdatePool<GameObject>(ActivePoolData& activePoolData, uint32 pool_id, uint32 db_guid_or_pool_id);
template void PoolMgr::UpdatePool<Creature>(ActivePoolData& activePoolData, uint32 pool_id, uint32 db_guid_or_pool_id);

std::unique_ptr<ActivePoolData> PoolMgr::InitPoolsForMap(Map* map)
{
    std::unique_ptr<ActivePoolData> spawnedPoolData = std::make_unique<ActivePoolData>(map);
    if (std::vector<uint32> const* poolIds = Trinity::Containers::MapGetValuePtr(mAutoSpawnPoolsPerMap, spawnedPoolData->GetMap()->GetId()))
        for (uint32 poolId : *poolIds)
            SpawnPool(*spawnedPoolData, poolId);

    return spawnedPoolData;
}

PoolTemplateData const* PoolMgr::GetPoolTemplate(uint16 pool_id) const
{
    return Trinity::Containers::MapGetValuePtr(mPoolTemplate, pool_id);
}
