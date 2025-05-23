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

#ifndef SF_UNIT_H
#define SF_UNIT_H

#include "DBCStructure.h"
#include "EventProcessor.h"
#include "FollowerReference.h"
#include "FollowerRefManager.h"
#include "FunctionProcessor.h"
#include "HostileRefManager.h"
#include "MotionMaster.h"
#include "Object.h"
#include "SpellAuraDefines.h"
#include "SpellDefines.h"
#include "ThreatManager.h"
#include "MoveSplineInit.h"
#include "SpellMgr.h"
#include "TimeValue.h"
#include "SpellInfo.h"
#include "UnitDefines.h"
#include <array>
#include <memory>

#define WORLD_TRIGGER   12999

class CustomSpellValues
{
    friend class Spell;
    typedef std::pair<SpellValueMod, int32> CustomSpellValueMod;
    typedef std::vector<CustomSpellValueMod>::const_iterator iterator;
public:
    void AddSpellMod(SpellValueMod mod, int32 value)
    {
        m_storage.push_back(CustomSpellValueMod(mod, value));
    }

    void SetHealingBonusCalculated()
    {
        m_healingBonus = false;
    }

    void SetMultiplier(float mult)
    {
        m_multiplier = mult;
    }

    iterator begin() const { return m_storage.begin(); }
    iterator end() const { return m_storage.end(); }

private:
    std::vector<CustomSpellValueMod> m_storage;
    bool m_healingBonus = true;
    float m_multiplier = 1.0f;
};


// high byte (3 from 0..3) of UNIT_FIELD_BYTES_2
enum ShapeshiftForm
{
    FORM_NONE = 0x00,
    FORM_CAT = 0x01,
    FORM_TREE = 0x02,
    FORM_TRAVEL = 0x03,
    FORM_AQUA = 0x04,
    FORM_BEAR = 0x05,
    FORM_AMBIENT = 0x06,
    FORM_GHOUL = 0x07,
    FORM_DIREBEAR = 0x08, // Removed in 4.0.1
    FORM_STEVES_GHOUL = 0x09,
    FORM_THARONJA_SKELETON = 0x0A,
    FORM_TEST_OF_STRENGTH = 0x0B,
    FORM_BLB_PLAYER = 0x0C,
    FORM_SHADOW_DANCE = 0x0D,
    FORM_CREATUREBEAR = 0x0E,
    FORM_CREATURECAT = 0x0F,
    FORM_GHOSTWOLF = 0x10,
    FORM_BATTLESTANCE = 0x11,
    FORM_DEFENSIVESTANCE = 0x12,
    FORM_BERSERKERSTANCE = 0x13,
    FORM_WISE_SERPENT = 0x14,
    FORM_ZOMBIE = 0x15,
    FORM_METAMORPHOSIS = 0x16,
    FORM_STURDY_OX = 0x17,
    FORM_FIERCE_TIGER = 0x18,
    FORM_UNDEAD = 0x19,
    FORM_MASTER_ANGLER = 0x1A,
    FORM_FLIGHT_EPIC = 0x1B,
    FORM_SHADOW = 0x1C,
    FORM_FLIGHT = 0x1D,
    FORM_STEALTH = 0x1E,
    FORM_MOONKIN = 0x1F,
    FORM_SPIRITOFREDEMPTION = 0x20
};





#define CREATURE_MAX_SPELLS     8
#define MAX_SPELL_CHARM         4
#define MAX_SPELL_VEHICLE       6
#define MAX_SPELL_POSSESS       8
#define MAX_SPELL_CONTROL_BAR   10

#define MAX_AGGRO_RESET_TIME 10 // in seconds
#define MAX_AGGRO_RADIUS 45.0f  // yards

enum Swing
{
    NOSWING = 0,
    SINGLEHANDEDSWING = 1,
    TWOHANDEDSWING = 2
};

enum VictimState
{
    VICTIMSTATE_INTACT = 0, // set when attacker misses
    VICTIMSTATE_HIT = 1, // victim got clear/blocked hit
    VICTIMSTATE_DODGE = 2,
    VICTIMSTATE_PARRY = 3,
    VICTIMSTATE_INTERRUPT = 4,
    VICTIMSTATE_BLOCKS = 5, // unused? not set when blocked, even on full block
    VICTIMSTATE_EVADES = 6,
    VICTIMSTATE_IS_IMMUNE = 7,
    VICTIMSTATE_DEFLECTS = 8
};

enum HitInfo
{
    HITINFO_NORMALSWING = 0x00000000,
    HITINFO_UNK1 = 0x00000001,               // req correct packet structure
    HITINFO_AFFECTS_VICTIM = 0x00000002,
    HITINFO_OFFHAND = 0x00000004,
    HITINFO_UNK2 = 0x00000008,
    HITINFO_MISS = 0x00000010,
    HITINFO_FULL_ABSORB = 0x00000020,
    HITINFO_PARTIAL_ABSORB = 0x00000040,
    HITINFO_FULL_RESIST = 0x00000080,
    HITINFO_PARTIAL_RESIST = 0x00000100,
    HITINFO_CRITICALHIT = 0x00000200,               // critical hit
    // 0x00000400
    // 0x00000800
    HITINFO_UNK12 = 0x00001000,
    HITINFO_BLOCK = 0x00002000,               // blocked damage
    // 0x00004000                                           // Hides worldtext for 0 damage
    // 0x00008000                                           // Related to blood visual
    HITINFO_GLANCING = 0x00010000,
    HITINFO_CRUSHING = 0x00020000,
    HITINFO_NO_ANIMATION = 0x00040000,
    // 0x00080000
    // 0x00100000
    HITINFO_SWINGNOHITSOUND = 0x00200000,               // unused?
    // 0x00400000
    HITINFO_RAGE_GAIN = 0x00800000,
    HITINFO_FAKE_DAMAGE = 0x01000000, // enables damage animation even if no damage done, set only if no damage
    HITINFO_MISSFIRE = 0x02000000,
    HITINFO_UNK26 = 0x04000000,
};

//i would like to remove this: (it is defined in item.h
enum InventorySlot
{
    NULL_BAG = 0,
    NULL_SLOT = 255
};

enum AttackSwingResult : int32;

struct FactionTemplateEntry;
struct ProcTriggeredData;
struct SpellValue;

class AreaTrigger;
class AuraApplication;
class Aura;
class UnitAura;
class AuraEffect;
class Creature;
class Spell;
class SpellInfo;
class DynamicObject;
class GameObject;
class IAreaTrigger;
class Item;
class Pet;
class PetAura;
class Minion;
class Guardian;
class UnitAI;
class Totem;
class Transport;
class Vehicle;
class TransportBase;
class SpellCastTargets;
class SpellHistory;

namespace Movement
{
    class ExtraMovementStatusElement;
    class MoveSpline;
}

typedef std::list<Unit*> UnitList;
typedef std::list< std::pair<Aura*, uint8> > DispelChargesList;

struct SpellImmune
{
    uint32 type;
    uint32 spellId;
};

typedef std::list<SpellImmune> SpellImmuneList;

enum UnitModifierType
{
    BASE_VALUE = 0,
    BASE_PCT = 1,
    TOTAL_VALUE = 2,
    TOTAL_PCT = 3,
    MODIFIER_TYPE_END = 4
};

enum WeaponDamageRange
{
    MINDAMAGE,
    MAXDAMAGE
};

enum DamageTypeToSchool
{
    RESISTANCE,
    DAMAGE_DEALT,
    DAMAGE_TAKEN
};

enum AuraRemoveMode
{
    AURA_REMOVE_NONE = 0,
    AURA_REMOVE_BY_DEFAULT = 1,       // scripted remove, remove by stack with aura with different ids and sc aura remove
    AURA_REMOVE_BY_CANCEL,
    AURA_REMOVE_BY_ENEMY_SPELL,       // dispel and absorb aura destroy
    AURA_REMOVE_BY_EXPIRE,            // aura duration has ended
    AURA_REMOVE_BY_DEATH,
    AURA_REMOVE_BY_INTERRUPT,         // interrupt flags
};

enum UnitMods
{
    UNIT_MOD_STAT_STRENGTH,                                 // UNIT_MOD_STAT_STRENGTH..UNIT_MOD_STAT_SPIRIT must be in existed order, it's accessed by index values of Stats enum.
    UNIT_MOD_STAT_AGILITY,
    UNIT_MOD_STAT_STAMINA,
    UNIT_MOD_STAT_INTELLECT,
    UNIT_MOD_STAT_SPIRIT,
    UNIT_MOD_HEALTH,
    UNIT_MOD_MANA,                                          // UNIT_MOD_MANA..UNIT_MOD_RUNIC_POWER must be in existed order, it's accessed by index values of Powers enum.
    UNIT_MOD_RAGE,
    UNIT_MOD_FOCUS,
    UNIT_MOD_ENERGY,
    UNIT_MOD_UNUSED,                                        // Old UNIT_MOD_HAPPINESS
    UNIT_MOD_RUNE,
    UNIT_MOD_RUNIC_POWER,
    UNIT_MOD_SOUL_SHARDS,
    UNIT_MOD_ECLIPSE,
    UNIT_MOD_HOLY_POWER,
    UNIT_MOD_ALTERNATIVE,
    UNIT_MOD_UNK,
    UNIT_MOD_CHI,
    UNIT_MOD_SHADOW_ORB,
    UNIT_MOD_BURNING_EMBERS,
    UNIT_MOD_DEMONIC_FURY,
    UNIT_MOD_ARMOR,                                         // UNIT_MOD_ARMOR..UNIT_MOD_RESISTANCE_ARCANE must be in existed order, it's accessed by index values of SpellSchools enum.
    UNIT_MOD_RESISTANCE_HOLY,
    UNIT_MOD_RESISTANCE_FIRE,
    UNIT_MOD_RESISTANCE_NATURE,
    UNIT_MOD_RESISTANCE_FROST,
    UNIT_MOD_RESISTANCE_SHADOW,
    UNIT_MOD_RESISTANCE_ARCANE,
    UNIT_MOD_ATTACK_POWER,
    UNIT_MOD_ATTACK_POWER_RANGED,
    UNIT_MOD_DAMAGE_MAINHAND,
    UNIT_MOD_DAMAGE_OFFHAND,
    UNIT_MOD_DAMAGE_RANGED,
    UNIT_MOD_END,
    // synonyms
    UNIT_MOD_STAT_START = UNIT_MOD_STAT_STRENGTH,
    UNIT_MOD_STAT_END = UNIT_MOD_STAT_SPIRIT + 1,
    UNIT_MOD_RESISTANCE_START = UNIT_MOD_ARMOR,
    UNIT_MOD_RESISTANCE_END = UNIT_MOD_RESISTANCE_ARCANE + 1,
    UNIT_MOD_POWER_START = UNIT_MOD_MANA,
    UNIT_MOD_POWER_END = UNIT_MOD_ALTERNATIVE + 1
};

enum BaseModGroup
{
    CRIT_PERCENTAGE,
    RANGED_CRIT_PERCENTAGE,
    OFFHAND_CRIT_PERCENTAGE,
    SHIELD_BLOCK_VALUE,
    BASEMOD_END
};

enum BaseModType
{
    FLAT_MOD,
    PCT_MOD,
    MOD_END
};

enum DeathState
{
    ALIVE = 0,
    JUST_DIED = 1,
    CORPSE = 2,
    DEAD = 3,
    JUST_RESPAWNED = 4
};

enum UnitState : uint32
{
    UNIT_STATE_DIED                  = 0x00000001, // player has fake death aura
    UNIT_STATE_MELEE_ATTACKING       = 0x00000002, // player is melee attacking someone
    UNIT_STATE_CHARMED               = 0x00000004, // having any kind of charm aura on self
    UNIT_STATE_STUNNED               = 0x00000008,
    UNIT_STATE_ROAMING               = 0x00000010,
    UNIT_STATE_CHASE                 = 0x00000020,
    UNIT_STATE_FOCUSING              = 0x00000040,
    UNIT_STATE_FLEEING               = 0x00000080,
    UNIT_STATE_IN_FLIGHT             = 0x00000100, // player is in flight mode
    UNIT_STATE_FOLLOW                = 0x00000200,
    UNIT_STATE_ROOT                  = 0x00000400,
    UNIT_STATE_CONFUSED              = 0x00000800,
    UNIT_STATE_DISTRACTED            = 0x00001000,
    UNIT_STATE_ISOLATED              = 0x00002000, // area auras do not affect other players
    UNIT_STATE_ATTACK_PLAYER         = 0x00004000,
    UNIT_STATE_CASTING               = 0x00008000,
    UNIT_STATE_POSSESSED             = 0x00010000, // being possessed by another unit
    UNIT_STATE_CHARGING              = 0x00020000,
    UNIT_STATE_JUMPING               = 0x00040000,
    UNIT_STATE_FOLLOW_FORMATION      = 0x00080000,
    UNIT_STATE_MOVE                  = 0x00100000,
    UNIT_STATE_ROTATING              = 0x00200000,
    UNIT_STATE_EVADE                 = 0x00400000,
    UNIT_STATE_ROAMING_MOVE          = 0x00800000,
    UNIT_STATE_CONFUSED_MOVE         = 0x01000000,
    UNIT_STATE_FLEEING_MOVE          = 0x02000000,
    UNIT_STATE_CHASE_MOVE            = 0x04000000,
    UNIT_STATE_FOLLOW_MOVE           = 0x08000000,
    UNIT_STATE_IGNORE_PATHFINDING    = 0x10000000, // do not use pathfinding in any MovementGenerator
    UNIT_STATE_FOLLOW_FORMATION_MOVE = 0x20000000,

    UNIT_STATE_ALL_STATE_SUPPORTED = UNIT_STATE_DIED | UNIT_STATE_MELEE_ATTACKING | UNIT_STATE_CHARMED | UNIT_STATE_STUNNED | UNIT_STATE_ROAMING | UNIT_STATE_CHASE
                                   | UNIT_STATE_FOCUSING | UNIT_STATE_FLEEING | UNIT_STATE_IN_FLIGHT | UNIT_STATE_FOLLOW | UNIT_STATE_ROOT | UNIT_STATE_CONFUSED
                                   | UNIT_STATE_DISTRACTED | UNIT_STATE_ISOLATED | UNIT_STATE_ATTACK_PLAYER | UNIT_STATE_CASTING
                                   | UNIT_STATE_POSSESSED | UNIT_STATE_CHARGING | UNIT_STATE_JUMPING | UNIT_STATE_MOVE | UNIT_STATE_ROTATING
                                   | UNIT_STATE_EVADE | UNIT_STATE_ROAMING_MOVE | UNIT_STATE_CONFUSED_MOVE | UNIT_STATE_FLEEING_MOVE
                                   | UNIT_STATE_CHASE_MOVE | UNIT_STATE_FOLLOW_MOVE | UNIT_STATE_IGNORE_PATHFINDING | UNIT_STATE_FOLLOW_FORMATION_MOVE,

    UNIT_STATE_UNATTACKABLE        = UNIT_STATE_IN_FLIGHT,
    UNIT_STATE_MOVING              = UNIT_STATE_ROAMING_MOVE | UNIT_STATE_CONFUSED_MOVE | UNIT_STATE_FLEEING_MOVE | UNIT_STATE_CHASE_MOVE | UNIT_STATE_FOLLOW_MOVE | UNIT_STATE_FOLLOW_FORMATION_MOVE,
    UNIT_STATE_CONTROLLED          = UNIT_STATE_CONFUSED | UNIT_STATE_STUNNED | UNIT_STATE_FLEEING,
    UNIT_STATE_LOST_CONTROL        = UNIT_STATE_CONTROLLED | UNIT_STATE_POSSESSED | UNIT_STATE_JUMPING | UNIT_STATE_CHARGING,
    UNIT_STATE_CANNOT_AUTOATTACK   = UNIT_STATE_CONTROLLED | UNIT_STATE_CHARGING | UNIT_STATE_CASTING,
    UNIT_STATE_SIGHTLESS           = UNIT_STATE_LOST_CONTROL | UNIT_STATE_EVADE,
    UNIT_STATE_CANNOT_TURN         = UNIT_STATE_LOST_CONTROL | UNIT_STATE_ROTATING | UNIT_STATE_FOCUSING,
    UNIT_STATE_NOT_MOVE            = UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DIED | UNIT_STATE_DISTRACTED,

    UNIT_STATE_ALL_ERASABLE        = UNIT_STATE_ALL_STATE_SUPPORTED & ~(UNIT_STATE_IGNORE_PATHFINDING),
    UNIT_STATE_ALL_STATE           = 0xffffffff
};

extern float baseMoveSpeed [MAX_MOVE_TYPE];
extern float playerBaseMoveSpeed [MAX_MOVE_TYPE];

enum WeaponAttackType
{
    BASE_ATTACK = 0,
    OFF_ATTACK = 1,
    RANGED_ATTACK = 2,
    MAX_ATTACK
};

enum CombatRating
{
    CR_WEAPON_SKILL = 0,
    CR_DEFENSE_SKILL = 1, // Removed in 4.0.1
    CR_DODGE = 2,
    CR_PARRY = 3,
    CR_BLOCK = 4,
    CR_HIT_MELEE = 5,
    CR_HIT_RANGED = 6,
    CR_HIT_SPELL = 7,
    CR_CRIT_MELEE = 8,
    CR_CRIT_RANGED = 9,
    CR_CRIT_SPELL = 10,
    CR_HIT_TAKEN_MELEE = 11, // Deprecated since Cataclysm
    CR_HIT_TAKEN_RANGED = 12, // Deprecated since Cataclysm
    CR_HIT_TAKEN_SPELL = 13, // Deprecated since Cataclysm
    CR_RESILIENCE_CRIT_TAKEN = 14,
    CR_RESILIENCE_PLAYER_DAMAGE_TAKEN = 15,
    CR_CRIT_TAKEN_SPELL = 16, // Deprecated since Cataclysm
    CR_HASTE_MELEE = 17,
    CR_HASTE_RANGED = 18,
    CR_HASTE_SPELL = 19,
    CR_WEAPON_SKILL_MAINHAND = 20,
    CR_WEAPON_SKILL_OFFHAND = 21,
    CR_WEAPON_SKILL_RANGED = 22,
    CR_EXPERTISE = 23,
    CR_ARMOR_PENETRATION = 24,
    CR_MASTERY = 25,
    CR_PVP_POWER = 26,

    CR_MAX_COMBAT_RATING
};

enum DamageEffectType
{
    DIRECT_DAMAGE = 0,                            // used for normal weapon damage (not for class abilities or spells)
    SPELL_DIRECT_DAMAGE = 1,                            // spell/class abilities damage
    DOT = 2,
    HEAL = 3,
    NODAMAGE = 4,                            // used also in case when damage applied to health but not applied to spell channelInterruptFlags/etc
    SELF_DAMAGE = 5
};


enum NPCFlags2
{
    UNIT_NPC_FLAG2_NONE              = 0x00000000,
    UNIT_NPC_FLAG2_ITEM_UPGRADE      = 0x00000001,    // Item Upgrade
};




enum UnitTypeMask
{
    UNIT_MASK_NONE = 0x00000000,
    UNIT_MASK_SUMMON = 0x00000001,
    UNIT_MASK_MINION = 0x00000002,
    UNIT_MASK_GUARDIAN = 0x00000004,
    UNIT_MASK_TOTEM = 0x00000008,
    UNIT_MASK_PET = 0x00000010,
    UNIT_MASK_VEHICLE = 0x00000020,
    UNIT_MASK_PUPPET = 0x00000040,
    UNIT_MASK_HUNTER_PET = 0x00000080,
    UNIT_MASK_CONTROLABLE_GUARDIAN = 0x00000100,
    UNIT_MASK_ACCESSORY = 0x00000200
};

struct DiminishingReturn
{
    DiminishingReturn(DiminishingGroup group, uint32 t, uint32 count)
        : DRGroup(group), stack(0), hitTime(t), hitCount(count)
    { }

    DiminishingGroup        DRGroup : 16;
    uint16                  stack : 16;
    uint32                  hitTime;
    uint32                  hitCount;
};

enum MeleeHitOutcome
{
    MELEE_HIT_EVADE,
    MELEE_HIT_MISS,
    MELEE_HIT_DODGE,
    MELEE_HIT_BLOCK,
    MELEE_HIT_PARRY,
    MELEE_HIT_GLANCING,
    MELEE_HIT_CRIT,
    MELEE_HIT_CRUSHING,
    MELEE_HIT_MISSFIRE,
    MELEE_HIT_NORMAL,
};

class DispelInfo
{
    public:
    explicit DispelInfo(Unit* dispeller, uint32 dispellerSpellId, uint8 chargesRemoved) :
        _dispellerUnit(dispeller), _dispellerSpell(dispellerSpellId), _chargesRemoved(chargesRemoved)
    { }

    Unit* GetDispeller() const
    {
        return _dispellerUnit;
    }
    uint32 GetDispellerSpellId() const
    {
        return _dispellerSpell;
    }
    uint8 GetRemovedCharges() const
    {
        return _chargesRemoved;
    }
    void SetRemovedCharges(uint8 amount)
    {
        _chargesRemoved = amount;
    }
    private:
    Unit* _dispellerUnit;
    uint32 _dispellerSpell;
    uint8 _chargesRemoved;
};

struct CleanDamage
{
    CleanDamage(uint32 mitigated, uint32 absorbed, WeaponAttackType _attackType, MeleeHitOutcome _hitOutCome) :
        absorbed_damage(absorbed), mitigated_damage(mitigated), attackType(_attackType), hitOutCome(_hitOutCome)
    { }

    uint32 absorbed_damage;
    uint32 mitigated_damage;

    WeaponAttackType attackType;
    MeleeHitOutcome hitOutCome;
};

struct CalcDamageInfo;

class DamageInfo
{
private:
    Unit* const m_attacker;
    Unit* const m_victim;
    uint32 m_damage;
    SpellInfo const* const m_spellInfo;
    SpellSchoolMask const m_schoolMask;
    DamageEffectType const m_damageType;
    WeaponAttackType m_attackType;
    uint32 m_absorb;
    uint32 m_resist;
    uint32 m_block;
    uint32 m_absorbedSplit = 0;

public:
    explicit DamageInfo(Unit* _attacker, Unit* _victim, uint32 _damage, SpellInfo const* _spellInfo, SpellSchoolMask _schoolMask, DamageEffectType _damageType);
    explicit DamageInfo(CalcDamageInfo& dmgInfo);

    void ModifyDamage(int32 amount);
    void AbsorbDamage(uint32 amount);
    void ResistDamage(uint32 amount);
    void BlockDamage(uint32 amount);
    void AbsorbSpilttedDamage(uint32 amount)
    {
        m_absorbedSplit = amount;
    }

    Unit* GetAttacker() const { return m_attacker; }
    Unit* GetVictim() const { return m_victim; }
    SpellInfo const* GetSpellInfo() const { return m_spellInfo; }
    SpellSchoolMask GetSchoolMask() const
    {
        return m_schoolMask;
    }
    DamageEffectType GetDamageType() const
    {
        return m_damageType;
    }
    WeaponAttackType GetAttackType() const
    {
        return m_attackType;
    }
    uint32 GetDamage() const
    {
        return m_damage;
    }
    uint32 GetAbsorb() const
    {
        return m_absorb;
    }
    uint32 GetResist() const
    {
        return m_resist;
    }
    uint32 GetBlock() const
    {
        return m_block;
    }
    uint32 GetAbsorbedSplit() const
    {
        return m_absorbedSplit;
    }
};

class HealInfo
{
public:
    HealInfo(uint32 heal, uint32 effective, uint32 overheal)
        : m_heal(heal), m_effective(effective), m_overheal(overheal)
    {
    }

    uint32 GetHeal() const
    {
        return m_heal;
    }

    uint32 GetEffectiveHeal() const
    {
        return m_effective;
    }

    uint32 GetOverheal() const
    {
        return m_overheal;
    }

private:
    uint32 m_heal;
    uint32 m_effective;
    uint32 m_overheal;
};

struct ProcTriggerContext;

class ProcEventInfo
{
    public:
    ProcEventInfo(Unit* actor, Unit* actionTarget, Unit* procTarget, uint32 typeMask,
                  uint32 spellTypeMask, uint32 spellPhaseMask, uint32 hitMask,
                  DamageInfo* damageInfo, HealInfo* healInfo, ProcTriggerContext const* ctx);

    Unit* GetActor()
    {
        return _actor;
    }
    Unit* GetActionTarget() const
    {
        return _actionTarget;
    }
    Unit* GetProcTarget() const
    {
        return _procTarget;
    }

    uint32 GetTypeMask() const
    {
        return _typeMask;
    }
    uint32 GetSpellTypeMask() const
    {
        return _spellTypeMask;
    }
    uint32 GetSpellPhaseMask() const
    {
        return _spellPhaseMask;
    }
    uint32 GetHitMask() const
    {
        return _hitMask;
    }

    SpellInfo const* GetSpellInfo() const
    {
        return _damageInfo->GetSpellInfo();
    }
    SpellSchoolMask GetSchoolMask() const
    {
        return _damageInfo->GetSchoolMask();
    }

    DamageInfo* GetDamageInfo() const
    {
        return _damageInfo;
    }
    HealInfo* GetHealInfo() const
    {
        return _healInfo;
    }

    ProcTriggerContext const* GetProcTriggerContext() const
    {
        return _ctx;
    }

    Spell const* TriggeredBySpell() const { return _spell; }
    AuraEffect const* TriggeredByPeriodic() const { return _periodicEffect; }

    uint32 GetDamage() const { return _damageInfo->GetDamage(); }
    uint32 GetDamageWithoutResilience() const;
    uint32 GetHeal() const { return _healInfo->GetHeal(); }

private:
    Unit* const _actor;
    Unit* const _actionTarget;
    Unit* const _procTarget;
    uint32 _typeMask;
    uint32 _spellTypeMask;
    uint32 _spellPhaseMask;
    uint32 _hitMask;
    Spell* _spell;
    AuraEffect* _periodicEffect;
    DamageInfo* _damageInfo;
    HealInfo* _healInfo;
    ProcTriggerContext const* _ctx;
};

// Struct for use in Unit::CalculateMeleeDamage
// Need create structure like in SMSG_ATTACKERSTATEUPDATE opcode
struct CalcDamageInfo
{
    Unit  *attacker;             // Attacker
    Unit  *target;               // Target for damage
    uint32 damageSchoolMask;
    uint32 damage;
    uint32 absorb;
    uint32 resist;
    uint32 blocked_amount;
    uint32 HitInfo;
    uint32 TargetState;
    // Helper
    WeaponAttackType attackType; //
    uint32 procAttacker;
    uint32 procVictim;
    uint32 procEx;
    uint32 cleanDamage;          // Used only for rage calculation
    uint32 unmitigatedDamage = 0;
    MeleeHitOutcome hitOutCome;  /// @todo remove this field (need use TargetState)
};

// Spell damage info structure based on structure sending in SMSG_SPELLNONMELEEDAMAGELOG opcode
struct SpellNonMeleeDamage
{
    SpellNonMeleeDamage(Unit* _attacker, Unit* _target, uint32 _SpellID, uint32 _schoolMask)
        : target(_target), attacker(_attacker), SpellID(_SpellID), damage(0), overkill(0), schoolMask(_schoolMask),
        absorb(0), resist(0), physicalLog(false), unused(false), blocked(0), HitInfo(0), cleanDamage(0)
    { }

    Unit   *target;
    Unit   *attacker;
    uint32 SpellID;
    uint32 damage;
    uint32 overkill;
    uint32 schoolMask;
    uint32 absorb;
    uint32 resist;
    bool   physicalLog;
    bool   unused;
    uint32 blocked;
    uint32 HitInfo;
    // Used for help
    uint32 cleanDamage;
};

struct SpellPeriodicAuraLogInfo
{
    SpellPeriodicAuraLogInfo(AuraEffect const* _auraEff, uint32 _damage, uint32 _overDamage, uint32 _absorb, uint32 _resist, float _multiplier, bool _critical)
        : auraEff(_auraEff), damage(_damage), overDamage(_overDamage), absorb(_absorb), resist(_resist), multiplier(_multiplier), critical(_critical)
    { }

    AuraEffect const* auraEff;
    uint32 damage;
    uint32 overDamage;                                      // overkill/overheal
    uint32 absorb;
    uint32 resist;
    float  multiplier;
    bool   critical;
};

uint32 createProcExtendMask(SpellNonMeleeDamage* damageInfo, SpellMissInfo missCondition);

struct RedirectThreatInfo
{
    ObjectGuid TargetGUID;
    uint32 ThreatPct;
};

enum CurrentSpellTypes
{
    CURRENT_MELEE_SPELL = 0,
    CURRENT_GENERIC_SPELL = 1,
    CURRENT_CHANNELED_SPELL = 2,
    CURRENT_AUTOREPEAT_SPELL = 3
};

#define CURRENT_FIRST_NON_MELEE_SPELL 1
#define CURRENT_MAX_SPELL             4

struct GlobalCooldown
{
    explicit GlobalCooldown(uint32 _dur = 0, uint32 _time = 0) : duration(_dur), cast_time(_time)
    { }

    uint32 duration;
    uint32 cast_time;
};

typedef std::unordered_map<uint32 /*category*/, GlobalCooldown> GlobalCooldownList;

class GlobalCooldownMgr                                     // Shared by Player and CharmInfo
{
    public:
    GlobalCooldownMgr()
    { }

    public:
    bool HasGlobalCooldown(SpellInfo const* spellInfo) const;
    void AddGlobalCooldown(SpellInfo const* spellInfo, uint32 gcd);
    void CancelGlobalCooldown(SpellInfo const* spellInfo);

    private:
    GlobalCooldownList m_GlobalCooldowns;
};

enum ActiveStates
{
    ACT_PASSIVE = 0x01,                                    // 0x01 - passive
    ACT_DISABLED = 0x81,                                    // 0x80 - castable
    ACT_ENABLED = 0xC1,                                    // 0x40 | 0x80 - auto cast + castable
    ACT_COMMAND = 0x07,                                    // 0x01 | 0x02 | 0x04
    ACT_REACTION = 0x06,                                    // 0x02 | 0x04
    ACT_DECIDE = 0x00                                     // custom
};

enum ReactStates
{
    REACT_PASSIVE = 0,
    REACT_DEFENSIVE = 1,
    REACT_AGGRESSIVE = 2,
    REACT_ASSIST = 3
};

enum CommandStates
{
    COMMAND_STAY = 0,
    COMMAND_FOLLOW = 1,
    COMMAND_ATTACK = 2,
    COMMAND_ABANDON = 3,
    COMMAND_MOVE_TO = 4
};

#define UNIT_ACTION_BUTTON_ACTION(X) (uint32(X) & 0x00FFFFFF)
#define UNIT_ACTION_BUTTON_TYPE(X)   ((uint32(X) & 0xFF000000) >> 24)
#define MAKE_UNIT_ACTION_BUTTON(A, T) (uint32(A) | (uint32(T) << 24))

struct UnitActionBarEntry
{
    UnitActionBarEntry() : packedData(uint32(ACT_DISABLED) << 24)
    { }

    uint32 packedData;

    // helper
    ActiveStates GetType() const
    {
        return ActiveStates(UNIT_ACTION_BUTTON_TYPE(packedData));
    }
    uint32 GetAction() const
    {
        return UNIT_ACTION_BUTTON_ACTION(packedData);
    }
    bool IsActionBarForSpell() const
    {
        ActiveStates Type = GetType();
        return Type == ACT_DISABLED || Type == ACT_ENABLED || Type == ACT_PASSIVE;
    }

    void SetActionAndType(uint32 action, ActiveStates type)
    {
        packedData = MAKE_UNIT_ACTION_BUTTON(action, type);
    }

    void SetType(ActiveStates type)
    {
        packedData = MAKE_UNIT_ACTION_BUTTON(UNIT_ACTION_BUTTON_ACTION(packedData), type);
    }

    void SetAction(uint32 action)
    {
        packedData = (packedData & 0xFF000000) | UNIT_ACTION_BUTTON_ACTION(action);
    }
};

typedef std::list<Player*> SharedVisionList;

enum CharmType
{
    CHARM_TYPE_CHARM,
    CHARM_TYPE_POSSESS,
    CHARM_TYPE_VEHICLE,
    CHARM_TYPE_CONVERT
};

typedef UnitActionBarEntry CharmSpellInfo;

enum ActionBarIndex
{
    ACTION_BAR_INDEX_START = 0,
    ACTION_BAR_INDEX_PET_SPELL_START = 3,
    ACTION_BAR_INDEX_PET_SPELL_END = 7,
    ACTION_BAR_INDEX_END = 10
};

#define MAX_UNIT_ACTION_BAR_INDEX (ACTION_BAR_INDEX_END-ACTION_BAR_INDEX_START)

struct TC_GAME_API CharmInfo
{
    public:
        explicit CharmInfo(Unit* unit);
        ~CharmInfo();
        void RestoreState();
        uint32 GetPetNumber() const { return _petnumber; }
        void SetPetNumber(uint32 petnumber, bool statwindow);

        void SetCommandState(CommandStates st) { _CommandState = st; }
        CommandStates GetCommandState() const { return _CommandState; }
        bool HasCommandState(CommandStates state) const { return (_CommandState == state); }

        void InitPossessCreateSpells();
        void InitCharmCreateSpells();
        void InitPetActionBar();
        void InitEmptyActionBar(bool withAttack = true);

        //return true if successful
        bool AddSpellToActionBar(SpellInfo const* spellInfo, ActiveStates newstate = ACT_DECIDE, uint8 preferredSlot = 0);
        bool RemoveSpellFromActionBar(uint32 spell_id);
        void LoadPetActionBar(const std::string& data);
        void BuildActionBar(WorldPacket* data);
        void SetSpellAutocast(SpellInfo const* spellInfo, bool state);
        void SetActionBar(uint8 index, uint32 spellOrAction, ActiveStates type)
        {
            PetActionBar [index].SetActionAndType(spellOrAction, type);
        }
        UnitActionBarEntry const* GetActionBarEntry(uint8 index) const
        {
            return &(PetActionBar [index]);
        }

        void ToggleCreatureAutocast(SpellInfo const* spellInfo, bool apply);

        CharmSpellInfo* GetCharmSpell(uint8 index)
        {
            return &(_charmspells [index]);
        }

        GlobalCooldownMgr& GetGlobalCooldownMgr()
        {
            return m_GlobalCooldownMgr;
        }

        void SetIsCommandAttack(bool val);
        bool IsCommandAttack();
        void SetIsCommandFollow(bool val);
        bool IsCommandFollow();
        void SetIsAtStay(bool val);
        bool IsAtStay();
        void SetIsFollowing(bool val);
        bool IsFollowing();
        void SetIsReturning(bool val);
        bool IsReturning();
        void SaveStayPosition();
        void SaveStayPosition(float x, float y, float z);
        void GetStayPosition(float &x, float &y, float &z);

    private:

        Unit* _unit;
        UnitActionBarEntry PetActionBar [MAX_UNIT_ACTION_BAR_INDEX];
        CharmSpellInfo _charmspells [4];
        CommandStates _CommandState;
        uint32 _petnumber;
        bool _barInit;

        //for restoration after charmed
        ReactStates     _oldReactState;

        bool _isCommandAttack;
        bool _isCommandFollow;
        bool _isAtStay;
        bool _isFollowing;
        bool _isReturning;
        float _stayX;
        float _stayY;
        float _stayZ;

        GlobalCooldownMgr m_GlobalCooldownMgr;
};

// for clearing special attacks
#define REACTIVE_TIMER_START 4000

enum ReactiveType
{
    REACTIVE_DEFENSE = 0,
    REACTIVE_HUNTER_PARRY = 1,
    REACTIVE_OVERPOWER = 2
};

#define MAX_REACTIVE 3

enum SummonSlot
{
    SUMMON_SLOT_PET                         = 0,
    SUMMON_SLOT_TOTEM                       = 1,
    SUMMON_SLOT_TOTEM_FIRE                  = 1,
    SUMMON_SLOT_TOTEM_EARTH                 = 2,
    SUMMON_SLOT_TOTEM_WATER                 = 3,
    SUMMON_SLOT_TOTEM_AIR                   = 4,
    SUMMON_SLOT_MAX_TOTEM                   = 5,
    SUMMON_SLOT_MINIPET                     = 5,
    SUMMON_SLOT_QUEST                       = 6,
    SUMMON_SLOT_TOTEM_EXTRA                 = 7,
    SUMMON_SLOT_MAX                         = 8
};

#define MAX_GAMEOBJECT_SLOT 4

enum PlayerTotemType
{
    SUMMON_TYPE_TOTEM_FIRE                  = 63,
    SUMMON_TYPE_TOTEM_EARTH                 = 81,
    SUMMON_TYPE_TOTEM_WATER                 = 82,
    SUMMON_TYPE_TOTEM_AIR                   = 83
};

// delay time next attack to prevent client attack animation problems
#define ATTACK_DISPLAY_DELAY 200
#define MAX_PLAYER_STEALTH_DETECT_RANGE 30.0f               // max distance for detection targets by player

struct SpellProcEventEntry;                                 // used only privately

struct ProcTriggerContext
{
    ProcTriggerContext() { }
    ProcTriggerContext(Spell* spell, SpellNonMeleeDamage* spellDamage = nullptr) : TriggeredBySpell(spell), SpellDamage(spellDamage) { }
    ProcTriggerContext(AuraEffect const* auraEffect, SpellNonMeleeDamage* spellDamage = nullptr) : TriggeredByAuraEffect(const_cast<AuraEffect*>(auraEffect)), SpellDamage(spellDamage) { }
    ProcTriggerContext(CalcDamageInfo* meleeDamage) : MeleeDamage(meleeDamage) { }

    Spell* TriggeredBySpell = nullptr;
    AuraEffect* TriggeredByAuraEffect = nullptr;
    SpellNonMeleeDamage* SpellDamage = nullptr;
    CalcDamageInfo* MeleeDamage = nullptr;
    uint32 EffectiveHeal = 0;
    uint32 Overheal = 0;
    uint32 UnmitigatedDamage = 0;
    ProcPhase Phase = ProcPhase::Hit;
};

class TC_GAME_API Unit : public WorldObject
{
public:

    typedef std::set<Unit*> AttackerSet;
    typedef std::set<Unit*> ControlList;
    typedef std::vector<Unit*> UnitVector;

    typedef std::multimap<uint32, Aura*> AuraMap;
    typedef std::pair<AuraMap::const_iterator, AuraMap::const_iterator> AuraMapBounds;
    typedef std::pair<AuraMap::iterator, AuraMap::iterator> AuraMapBoundsNonConst;

    typedef std::multimap<uint32, AuraApplication*> AuraApplicationMap;
    typedef std::pair<AuraApplicationMap::const_iterator, AuraApplicationMap::const_iterator> AuraApplicationMapBounds;
    typedef std::pair<AuraApplicationMap::iterator, AuraApplicationMap::iterator> AuraApplicationMapBoundsNonConst;

    typedef std::multimap<AuraStateType, AuraApplication*> AuraStateAurasMap;
    typedef std::pair<AuraStateAurasMap::const_iterator, AuraStateAurasMap::const_iterator> AuraStateAurasMapBounds;

    typedef std::list<AuraEffect*> AuraEffectList;
    typedef std::list<Aura*> AuraList;
    typedef std::list<AuraApplication *> AuraApplicationList;
    typedef std::list<DiminishingReturn> Diminishing;

    // typedef std::vector<std::pair<uint8 /*procEffectMask*/, AuraApplication*>> AuraApplicationProcContainer; // TC

    typedef std::map<uint8, AuraApplication*> VisibleAuraMap;

    class RemainingPeriodicAmount final
    {
    public:

        RemainingPeriodicAmount(int32 total, int32 ticks) : _total(total), _ticks(ticks) { }
        int32 Total() const { return _total; }
        int32 PerTick() const { return _total ? _total / _ticks : 0; }
    private:
        int32 _total;
        int32 _ticks;
    };

    virtual ~Unit();

    UnitAI* GetAI() const { return i_AI; }
    void SetAI(UnitAI* newAI)
    {
        i_AI = newAI;
    }

public:

    void AddToWorld() override;
    void RemoveFromWorld() override;
    void AddDelayedEvent(uint64 timeOffset, std::function<void()>&& function);

    void CleanupBeforeRemoveFromMap(bool finalCleanup);
    void CleanupsBeforeDelete(bool finalCleanup = true) override;                        // used in ~Creature/~Player (or before mass creature delete to remove cross-references to already deleted units)

    // uint32 GetDynamicFlags() const override { return GetUInt32Value(UNIT_DYNAMIC_FLAGS); }
    // void ReplaceAllDynamicFlags(uint32 flag) override { SetUInt32Value(UNIT_DYNAMIC_FLAGS, flag); }

    DiminishingLevels GetDiminishing(DiminishingGroup  group);
    void IncrDiminishing(DiminishingGroup group);
    float ApplyDiminishingToDuration(DiminishingGroup  group, int32& duration, Unit* caster, DiminishingLevels Level);
    void ApplyDiminishingAura(DiminishingGroup  group, bool apply);
    void ClearDiminishings()
    {
        m_Diminishing.clear();
    }

    // target dependent range checks
    float GetSpellMaxRangeForTarget(Unit const* target, SpellInfo const* spellInfo) const;
    float GetSpellMinRangeForTarget(Unit const* target, SpellInfo const* spellInfo) const;

    virtual void Update(uint32 time) override;

    void UpdateAttackTimer(WeaponAttackType type, uint32 diff);
    void setAttackTimer(WeaponAttackType type, uint32 time)
    {
        m_attackTimer [type] = time;
    }
    void resetAttackTimer(WeaponAttackType type = BASE_ATTACK);
    uint32 getAttackTimer(WeaponAttackType type) const
    {
        return m_attackTimer [type];
    }
    bool isAttackReady(WeaponAttackType type = BASE_ATTACK) const
    {
        return m_attackTimer [type] == 0;
    }
    bool HasOffhandWeapon() const;
    bool CanDualWield() const
    {
        return m_canDualWield;
    }
    void SetCanDualWield(bool value)
    {
        m_canDualWield = value;
    }
    float GetCombatReach() const override { return m_floatValues [UNIT_FIELD_COMBAT_REACH]; }
    float GetMeleeRange(Unit const* target) const;
    float GetMeleeReach() const;
    bool IsWithinCombatRange(const Unit* obj, float dist2compare) const;
    bool IsWithinMeleeRange(const Unit* obj, float dist = NOMINAL_MELEE_RANGE) const;
    bool IsWithinRangedMeleeRange(Unit const* obj, float dist) const;
    void GetRandomContactPoint(const Unit* target, float &x, float &y, float &z, float distance2dMin, float distance2dMax) const;
    uint32 m_extraAttacks;
    bool m_canDualWield;

    uint32 RecaculateDamage(uint32 damage); 

    uint32 RecaculateHealing(uint32 healamount);


    void _addAttacker(Unit* pAttacker);                  // must be called only from Unit::Attack(Unit*)
    void _removeAttacker(Unit* pAttacker);               // must be called only from Unit::AttackStop()
    Unit* getAttackerForHelper() const;                 // If someone wants to help, who to give them
    bool Attack(Unit* victim, bool meleeAttack);
    void CastStop(uint32 except_spellid = 0);
    bool AttackStop();
    void RemoveAllAttackers();
    AttackerSet const& getAttackers() const
    {
        return m_attackers;
    }
    bool isAttackingPlayer() const;
    Unit* GetVictim() const
    {
        return m_attacking;
    }

    void CombatStop(bool includingCast = false);
    void CombatStopWithPets(bool includingCast = false);
    void StopAttackFaction(uint32 faction_id);
    void GetAttackableUnitListInRange(std::list<Unit*> &list, float fMaxSearchRange) const;
    Unit* SelectNearbyTarget(Unit* exclude = NULL, float dist = NOMINAL_MELEE_RANGE) const;
    Unit* SelectNearbyAlly(Unit* exclude = NULL, float dist = NOMINAL_MELEE_RANGE) const;
    Unit* SelectNearestTarget(float dist = 0) const;
    void SendMeleeAttackStop(Unit* victim = NULL);
    void SendMeleeAttackStart(Unit* victim);

    void AddUnitState(uint32 f) { m_state |= f; }
    bool HasUnitState(const uint32 f) const { return (m_state & f) != 0; }
    void ClearUnitState(uint32 f) { m_state &= ~f; }
    bool CanFreeMove() const;

    uint32 HasUnitTypeMask(uint32 mask) const { return mask & m_unitTypeMask; }
    void AddUnitTypeMask(uint32 mask) { m_unitTypeMask |= mask; }
    bool IsSummon() const   { return (m_unitTypeMask & UNIT_MASK_SUMMON) != 0; }
    bool IsGuardian() const { return (m_unitTypeMask & UNIT_MASK_GUARDIAN) != 0; }  
    bool IsPet() const      { return (m_unitTypeMask & UNIT_MASK_PET) != 0; }
    bool IsHunterPet() const{ return (m_unitTypeMask & UNIT_MASK_HUNTER_PET) != 0; }
    bool IsTotem() const    { return (m_unitTypeMask & UNIT_MASK_TOTEM) != 0; }
    bool IsVehicle() const  { return (m_unitTypeMask & UNIT_MASK_VEHICLE) != 0; }
    bool IsPetGuardianStuff() const { return m_unitTypeMask & (UNIT_MASK_SUMMON | UNIT_MASK_GUARDIAN | UNIT_MASK_PET | UNIT_MASK_HUNTER_PET | UNIT_MASK_TOTEM); }

    uint8 GetLevel() const { return uint8(GetUInt32Value(UNIT_FIELD_LEVEL)); }
    uint8 GetLevelForTarget(WorldObject const* /*target*/) const override { return GetLevel(); }
    void SetLevel(uint8 lvl, bool sendUpdate = true);
    // uint8 GetRace() const { return GetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_RACE); }
    // void SetRace(uint8 race) { SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_RACE, race); }
    uint8 GetRace() const { return GetByteValue(UNIT_FIELD_BYTES_0, 0); }
    void SetRace(uint8 race) { SetByteValue(UNIT_FIELD_BYTES_0, 0, race);}
    uint32 GetRaceMask() const { return 1 << (GetRace() - 1); }
    uint8 GetClass() const { return GetByteValue(UNIT_FIELD_BYTES_0, 1); }
    void SetClass(uint8 newClass) { SetByteValue(UNIT_FIELD_BYTES_0, 1, newClass);}
    uint32 GetClassMask() const { return 1 << (GetClass() - 1); }
    uint8 GetGender() const { return GetByteValue(UNIT_FIELD_BYTES_0, 3); }
    void SetGender(uint8 gender) { SetByteValue(UNIT_FIELD_BYTES_0, 3, gender); }

    // Gender GetGender() const { return Gender(GetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_GENDER)); }
    // void SetGender(Gender gender) { SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_GENDER, gender); }

    float GetStat(Stats stat) const { return float(GetUInt32Value(UNIT_FIELD_STATS + stat)); }
    void SetStat(Stats stat, int32 val) { SetStatInt32Value(UNIT_FIELD_STATS + stat, val); }
    uint32 GetArmor() const { return GetResistance(SPELL_SCHOOL_NORMAL); }
    void SetArmor(int32 val) { SetResistance(SPELL_SCHOOL_NORMAL, val); }

    uint32 GetResistance(SpellSchools school) const
    {
        return GetUInt32Value(UNIT_FIELD_RESISTANCES + school);
    }
    uint32 GetResistance(SpellSchoolMask mask) const;
    void SetResistance(SpellSchools school, int32 val)
    {
        SetStatInt32Value(UNIT_FIELD_RESISTANCES + school, val);
    }


    uint32 GetHealth()    const { return GetUInt32Value(UNIT_FIELD_HEALTH); }
    uint32 GetMaxHealth() const
    {
        return GetUInt32Value(UNIT_FIELD_MAX_HEALTH);
    }

    bool IsFullHealth() const
    {
        return GetHealth() == GetMaxHealth();
    }
    bool HealthBelowPct(int32 pct) const
    {
        return GetHealth() < CountPctFromMaxHealth(pct);
    }
    bool HealthBelowPctDamaged(int32 pct, uint32 damage) const
    {
        return int64(GetHealth()) - int64(damage) < int64(CountPctFromMaxHealth(pct));
    }
    bool HealthAbovePct(int32 pct) const
    {
        return GetHealth() > CountPctFromMaxHealth(pct);
    }
    bool HealthAbovePctHealed(int32 pct, uint32 heal) const
    {
        return uint64(GetHealth()) + uint64(heal) > CountPctFromMaxHealth(pct);
    }
    float GetHealthPct() const
    {
        return GetMaxHealth() ? 100.f * GetHealth() / GetMaxHealth() : 0.0f;
    }
    uint32 CountPctFromMaxHealth(int32 pct) const
    {
        return CalculatePct(GetMaxHealth(), pct);
    }
    uint32 CountPctFromCurHealth(int32 pct) const
    {
        return CalculatePct(GetHealth(), pct);
    }

    void SetHealth(uint32 val);
    void SetMaxHealth(uint32 val);
    void RegenerateHealth();
    inline void SetFullHealth()
    {
        SetHealth(GetMaxHealth());
    }
    int32 ModifyHealth(int32 val);
    int32 GetHealthGain(int32 dVal);

    Powers GetPowerType() const
    {
        return Powers(GetUInt32Value(UNIT_FIELD_DISPLAY_POWER));
    }
    void SetFieldPowerType(uint32 powerType)
    {
        SetUInt32Value(UNIT_FIELD_DISPLAY_POWER, powerType);
    }

    void SetPowerType(Powers power);
    int32 GetPower(Powers power) const;
    int32 GetMinPower(Powers power) const
    {
        return power == POWER_ECLIPSE ? -100 : 0;
    }
    int32 GetMaxPower(Powers power) const;
    float GetPowerPct(Powers power) const { return GetMaxPower(power) ? 100.f * GetPower(power) / GetMaxPower(power) : 0.0f; }

    int32 CountPctFromMaxPower(Powers power, int32 pct) const
    {
        return CalculatePct(GetMaxPower(power), pct);
    }
    void SetPower(Powers power, int32 val);
    void SetMaxPower(Powers power, int32 val);
    // returns the change in power
    int32 ModifyPower(Powers power, int32 val);
    int32 ModifyPowerPct(Powers power, float pct, bool apply = true);

    float GetBasePowerRegen(Powers power) const;
    float GetPowerRegen(Powers power) const;

    uint32 GetAttackTime(WeaponAttackType att) const;
    void SetAttackTime(WeaponAttackType att, uint32 val)
    {
        SetFloatValue(UNIT_FIELD_ATTACK_ROUND_BASE_TIME + att, val*m_modAttackSpeedPct [att]);
    }

    UnitFlags GetUnitFlags() const { return UnitFlags(GetUInt32Value(UNIT_FIELD_FLAGS)); }
    bool HasUnitFlag(UnitFlags flags) const { return HasFlag(UNIT_FIELD_FLAGS, flags); }
    void SetUnitFlag(UnitFlags flags) { SetFlag(UNIT_FIELD_FLAGS, flags); }
    void RemoveUnitFlag(UnitFlags flags) { RemoveFlag(UNIT_FIELD_FLAGS, flags); }
    void ReplaceAllUnitFlags(UnitFlags flags) { SetUInt32Value(UNIT_FIELD_FLAGS, flags); }

    UnitFlags2 GetUnitFlags2() const { return UnitFlags2(GetUInt32Value(UNIT_FIELD_FLAGS_2)); }
    bool HasUnitFlag2(UnitFlags2 flags) const { return HasFlag(UNIT_FIELD_FLAGS_2, flags); }
    void SetUnitFlag2(UnitFlags2 flags) { SetFlag(UNIT_FIELD_FLAGS_2, flags); }
    void RemoveUnitFlag2(UnitFlags2 flags) { RemoveFlag(UNIT_FIELD_FLAGS_2, flags); }
    void ReplaceAllUnitFlags2(UnitFlags2 flags) { SetUInt32Value(UNIT_FIELD_FLAGS_2, flags); }

    Emote GetEmoteState() const { return Emote(GetUInt32Value(UNIT_FIELD_NPC_EMOTESTATE)); }
    void SetEmoteState(Emote emote) { SetUInt32Value(UNIT_FIELD_NPC_EMOTESTATE, emote); }

    SheathState GetSheath() const { return SheathState(GetByteValue(UNIT_FIELD_BYTES_2, 0)); }
    virtual void SetSheath(SheathState sheathed) { SetByteValue(UNIT_FIELD_BYTES_2, 0, sheathed); }

    // faction template id
    uint32 GetFaction() const { return GetUInt32Value(UNIT_FIELD_FACTION_TEMPLATE); }
    void SetFaction(uint32 faction) { SetUInt32Value(UNIT_FIELD_FACTION_TEMPLATE, faction); }
    FactionTemplateEntry const* GetFactionTemplateEntry() const;

    ReputationRank GetReactionTo(Unit const* target) const;
    ReputationRank static GetFactionReactionTo(FactionTemplateEntry const* factionTemplateEntry, Unit const* target);

    // Spell Aura helpers
    [[nodiscard]] bool HasGhostAura()               const { return HasAuraType(SPELL_AURA_GHOST); };
    [[nodiscard]] bool HasMountedAura()             const { return HasAuraType(SPELL_AURA_MOUNTED); };
    [[nodiscard]] bool HasWaterWalkAura()           const { return HasAuraType(SPELL_AURA_WATER_WALK); };
    [[nodiscard]] bool HasFeatherFallAura()         const { return HasAuraType(SPELL_AURA_FEATHER_FALL); };
    [[nodiscard]] bool HasHoverAura()               const { return HasAuraType(SPELL_AURA_HOVER); };
    [[nodiscard]] bool HasFlyAura()                 const { return HasAuraType(SPELL_AURA_FLY); };
    [[nodiscard]] bool HasSpiritOfRedemptionAura()  const { return HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION); };
    [[nodiscard]] bool HasPreventsFleeingAura()     const { return HasAuraType(SPELL_AURA_PREVENTS_FLEEING); };
    //[[nodiscard]] bool HasPreventDurabilityLossAura()  const { return HasAuraType(SPELL_AURA_PREVENT_DURABILITY_LOSS); };
    [[nodiscard]] bool HasPreventResurectionAura()  const { return HasAuraType(SPELL_AURA_PREVENT_RESURRECTION); };
    [[nodiscard]] bool HasTransformAura()           const { return HasAuraType(SPELL_AURA_TRANSFORM); };
    [[nodiscard]] bool HasInterruptRegenAura()      const { return HasAuraType(SPELL_AURA_INTERRUPT_REGEN); };
    [[nodiscard]] bool HasNoPVPCreditAura()         const { return HasAuraType(SPELL_AURA_NO_PVP_CREDIT); };
    [[nodiscard]] bool HasWaterBreathingAura()      const { return HasAuraType(SPELL_AURA_WATER_BREATHING); };
    [[nodiscard]] bool HasIgnoreHitDirectionAura()  const { return HasAuraType(SPELL_AURA_IGNORE_HIT_DIRECTION); };
    [[nodiscard]] bool HasSpellMagnetAura()         const { return HasAuraType(SPELL_AURA_SPELL_MAGNET); };
    [[nodiscard]] bool HasOpenStableAura()          const { return HasAuraType(SPELL_AURA_OPEN_STABLE); };
    [[nodiscard]] bool HasCloneCasterAura()         const { return HasAuraType(SPELL_AURA_CLONE_CASTER); };
    [[nodiscard]] bool HasReflectSpellsAura()       const { return HasAuraType(SPELL_AURA_REFLECT_SPELLS); };
    [[nodiscard]] bool HasDetectAmoreAura()         const { return HasAuraType(SPELL_AURA_DETECT_AMORE); };
    [[nodiscard]] bool HasAllowOnlyAbilityAura()    const { return HasAuraType(SPELL_AURA_ALLOW_ONLY_ABILITY); };
    [[nodiscard]] bool HasPeriodicDummyAura()       const { return HasAuraType(SPELL_AURA_PERIODIC_DUMMY); };
    [[nodiscard]] bool HasControlVehicleAura()      const { return HasAuraType(SPELL_AURA_CONTROL_VEHICLE); };
    [[nodiscard]] bool HasAOECharmAura()            const { return HasAuraType(SPELL_AURA_AOE_CHARM); };
    [[nodiscard]] bool HasDetectSpellsAura()        const { return HasAuraType(SPELL_AURA_DEFLECT_SPELLS); };
    [[nodiscard]] bool HasPacifySilenceAura()       const { return HasAuraType(SPELL_AURA_MOD_PACIFY_SILENCE); }
    [[nodiscard]] bool HasSilenceAura()             const { return HasAuraType(SPELL_AURA_MOD_SILENCE); }
    [[nodiscard]] bool HasShapeshiftAura()          const { return HasAuraType(SPELL_AURA_MOD_SHAPESHIFT); }
    [[nodiscard]] bool HasDecreaseSpeedAura()       const { return HasAuraType(SPELL_AURA_MOD_DECREASE_SPEED); }
    [[nodiscard]] bool HasPacifyAura()              const { return HasAuraType(SPELL_AURA_MOD_PACIFY); }
    [[nodiscard]] bool HasIgnoreTargetResistAura()  const { return HasAuraType(SPELL_AURA_MOD_IGNORE_TARGET_RESIST); }
    [[nodiscard]] bool HasIncreaseMountedSpeedAura() const { return HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED); }
    [[nodiscard]] bool HasIncreaseMountedFlightSpeedAura() const { return HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED); }
    [[nodiscard]] bool HasThreatAura()              const { return HasAuraType(SPELL_AURA_MOD_THREAT); }
    [[nodiscard]] bool HasAttackerSpellCritChanceAura() const { return HasAuraType(SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE); }
    [[nodiscard]] bool HasUnattackableAura()        const { return HasAuraType(SPELL_AURA_MOD_UNATTACKABLE); }
    [[nodiscard]] bool HasHealthRegenInCombatAura() const { return HasAuraType(SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT); }
    [[nodiscard]] bool HasRegenDuringCombatAura()   const { return HasAuraType(SPELL_AURA_MOD_REGEN_DURING_COMBAT); }
    [[nodiscard]] bool HasFearAura()                const { return HasAuraType(SPELL_AURA_MOD_FEAR); }
    [[nodiscard]] bool HasConfuseAura()             const { return HasAuraType(SPELL_AURA_MOD_CONFUSE); }
    [[nodiscard]] bool HasRootAura()                const { return HasAuraType(SPELL_AURA_MOD_ROOT); }
    [[nodiscard]] bool HasStunAura()                const { return HasAuraType(SPELL_AURA_MOD_STUN); }
    [[nodiscard]] bool HasTauntAura()               const { return HasAuraType(SPELL_AURA_MOD_TAUNT); }
    [[nodiscard]] bool HasStealthAura()             const { return HasAuraType(SPELL_AURA_MOD_STEALTH); }
    [[nodiscard]] bool HasStealthDetectAura()       const { return HasAuraType(SPELL_AURA_MOD_STEALTH_DETECT); }
    [[nodiscard]] bool HasInvisibilityAura()        const { return HasAuraType(SPELL_AURA_MOD_INVISIBILITY); }
    [[nodiscard]] bool HasInvisibilityDetectAura()  const { return HasAuraType(SPELL_AURA_MOD_INVISIBILITY_DETECT); }

    bool IsHostileTo(Unit const* unit) const;
    bool IsHostileToPlayers() const;
    bool IsFriendlyTo(Unit const* unit) const;
    bool IsNeutralToAll() const;
    bool IsInPartyWith(Unit const* unit) const;
    bool IsInRaidWith(Unit const* unit, bool ignoreCharmer = false) const;
    void GetPartyMembers(std::list<Unit*> &units);
    bool IsContestedGuard() const;

    UnitPVPStateFlags GetPvpFlags() const { return UnitPVPStateFlags(GetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG)); }
    bool HasPvpFlag(UnitPVPStateFlags flags) const { return HasByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, flags); }
    void SetPvpFlag(UnitPVPStateFlags flags) { SetByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, flags); }
    void RemovePvpFlag(UnitPVPStateFlags flags) { RemoveByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, flags); }
    void ReplaceAllPvpFlags(UnitPVPStateFlags flags) { SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, flags); }
    
    bool IsPvP() const { return HasPvpFlag(UNIT_BYTE2_FLAG_PVP); }
    void SetPvP(bool state);
    [[nodiscard]] bool IsFFAPvP() const { return HasByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP); }
    [[nodiscard]] bool IsInSanctuary() const { return HasByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_SANCTUARY); }

    UnitPetFlag GetPetFlags() const { return UnitPetFlag(GetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS)); }
    bool HasPetFlag(UnitPetFlag flags) const { return HasByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, flags); }
    void SetPetFlag(UnitPetFlag flags) { SetByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, flags); }
    void RemovePetFlag(UnitPetFlag flags) { RemoveByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, flags); }
    void ReplaceAllPetFlags(UnitPetFlag flags) { SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, flags); }

    uint32 GetCreatureType() const;
    uint32 GetCreatureTypeMask() const;

    UnitStandStateType GetStandState() const { return UnitStandStateType(GetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_STAND_STATE)); }
    bool IsSitState() const;
    bool IsStandState() const;
    void SetStandState(UnitStandStateType state);

    void SetStandFlags(uint8 flags)
    {
        SetByteFlag(UNIT_FIELD_BYTES_1, 2, flags);
    }
    void RemoveStandFlags(uint8 flags)
    {
        RemoveByteFlag(UNIT_FIELD_BYTES_1, 2, flags);
    }

    void SetVisFlag(UnitVisFlags flags) { SetByteFlag(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_VIS_FLAG, flags); }
    void RemoveVisFlag(UnitVisFlags flags) { RemoveByteFlag(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_VIS_FLAG, flags); }
    void ReplaceAllVisFlags(UnitVisFlags flags) { SetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_VIS_FLAG, flags); }

    AnimTier GetAnimTier() const { return AnimTier(GetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_ANIM_TIER)); }
    void SetAnimTier(AnimTier animTier);

    bool IsMounted() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT); }
    uint32 GetMountDisplayId() const { return GetUInt32Value(UNIT_FIELD_MOUNT_DISPLAY_ID); }
    void SetMountDisplayId(uint32 mountDisplayId) { SetUInt32Value(UNIT_FIELD_MOUNT_DISPLAY_ID, mountDisplayId); }
    void Mount(uint32 mount, uint32 vehicleId = 0, uint32 creatureEntry = 0);
    void Dismount();
    MountCapabilityEntry const* GetMountCapability(uint32 mountType) const;

    void SendDurabilityLoss(Player* receiver, uint32 percent);
    void PlayOneShotAnimKitId(uint16 animKitId);
    void SetAIAnimKitId(uint16 animKitId);
    uint16 GetAIAnimKitId() const override { return _aiAnimKitId; }
    void SetMovementAnimKitId(uint16 animKitId);
    uint16 GetMovementAnimKitId() const override { return _movementAnimKitId; }
    void SetMeleeAnimKitId(uint16 animKitId);
    uint16 GetMeleeAnimKitId() const override { return _meleeAnimKitId; }

    uint16 GetMaxSkillValueForLevel(Unit const* target = NULL) const
    {
        return (target ? GetLevelForTarget(target) : GetLevel()) * 5;
    }
    void DealDamageMods(Unit* victim, uint32 &damage, uint32* absorb);
    uint32 DealDamage(Unit* victim, uint32 damage, CleanDamage const* cleanDamage = NULL, DamageEffectType damagetype = DIRECT_DAMAGE, SpellSchoolMask damageSchoolMask = SPELL_SCHOOL_MASK_NORMAL, SpellInfo const* spellProto = NULL, bool durabilityLoss = true);
    void Kill(Unit* victim, bool durabilityLoss = true, SpellInfo const* spellInfo = nullptr);
    int32 DealHeal(Unit* victim, uint32 addhealth);

    void ProcDamageAndSpell(Unit* victim, uint32 procAttacker, uint32 procVictim, uint32 procEx, uint32 amount, WeaponAttackType attType = BASE_ATTACK, SpellInfo const* procSpell = nullptr, SpellInfo const* procAura = nullptr, ProcTriggerContext const& context = ProcTriggerContext());
    void ProcDamageAndSpellFor(bool isVictim, Unit* target, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, SpellInfo const* procSpell, uint32 damage, SpellInfo const* procAura = nullptr, ProcTriggerContext const& context = ProcTriggerContext());

    void HandleEmoteCommand(uint32 anim_id);
    void HandleEmoteStateCommand(uint32 anim_id);
    bool HandleSwitchEmoteCommand(uint32 anim_id);
    void AttackerStateUpdate(Unit* victim, bool ignoreLos, WeaponAttackType attType = BASE_ATTACK, bool extra = false);
    void AttackerStateUpdate(Unit *victim, WeaponAttackType attType = BASE_ATTACK, bool extra = false)
    {
        AttackerStateUpdate(victim, false, attType, extra);
    }

    void CalculateMeleeDamage(Unit* victim, uint32 damage, CalcDamageInfo* damageInfo, WeaponAttackType attackType = BASE_ATTACK);
    void DealMeleeDamage(CalcDamageInfo* damageInfo, bool durabilityLoss);
    void HandleProcExtraAttackFor(Unit* victim);

    void CalculateSpellDamageTaken(SpellNonMeleeDamage* damageInfo, int32 damage, SpellInfo const* spellInfo, WeaponAttackType attackType = BASE_ATTACK, bool crit = false, uint32 procVictim = 0, uint32 procEx = 0);
    void DealSpellDamage(SpellNonMeleeDamage* damageInfo, bool durabilityLoss);

    float GetResilienceReduction() const;
    void ApplyResilience(Unit const* victim, int32* damage) const;
    void ApplyPvPHealMods(Unit const* victim, uint32& damage, SpellInfo const* spellInfo);

    float MeleeSpellMissChance(Unit const* victim, WeaponAttackType attType, uint32 spellId) const;
    SpellMissInfo MeleeSpellHitResult(Unit* victim, SpellInfo const* spellInfo);
    SpellMissInfo MagicSpellHitResult(Unit* victim, SpellInfo const* spellInfo);
    SpellMissInfo SpellHitResult(Unit* victim, SpellInfo const* spellInfo, uint32 effectMask, bool canReflect = false, Spell const* spellInstance = NULL);

    float GetUnitDodgeChance() const;
    float GetUnitParryChance() const;
    float GetUnitBlockChance() const;
    float GetDodgeChanceAgainst(Unit const* attacker, WeaponAttackType attType) const;
    float GetParryChanceAgainst(Unit const* attacker, WeaponAttackType attType) const;
    float GetBlockChangeAgainst(Unit const* attacker, WeaponAttackType attType) const;
    float GetUnitMissChance(WeaponAttackType attType) const;
    float GetUnitCriticalChance(WeaponAttackType attackType, const Unit* victim) const;
    int32 GetMechanicResistChance(SpellInfo const* spellInfo) const;
    bool CanUseAttackType(uint8 attacktype) const;

    virtual uint32 GetBlockPercent() const;

    uint32 GetUnitMeleeSkill(Unit const* target = NULL) const;

    float GetWeaponProcChance() const;
    float GetRealPPMProcChance(SpellInfo const* spellProto, Item const* item, WeaponAttackType attackType) const;
    float GetPPMProcChance(uint32 WeaponSpeed, float PPM, const SpellInfo* spellProto) const;

    MeleeHitOutcome RollMeleeOutcomeAgainst(const Unit* victim, WeaponAttackType attType) const;

    NPCFlags GetNpcFlags() const { return NPCFlags(GetUInt32Value(UNIT_FIELD_NPC_FLAGS)); }
    bool HasNpcFlag(NPCFlags flags) const { return HasFlag(UNIT_FIELD_NPC_FLAGS, flags) != 0; }
    void SetNpcFlag(NPCFlags flags) { SetFlag(UNIT_FIELD_NPC_FLAGS, flags); }
    void RemoveNpcFlag(NPCFlags flags) { RemoveFlag(UNIT_FIELD_NPC_FLAGS, flags); }
    void ReplaceAllNpcFlags(NPCFlags flags) { SetUInt32Value(UNIT_FIELD_NPC_FLAGS, flags); } // UNIT_NPC_FLAGS

    bool IsVendor() const { return HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR); }
    bool IsTrainer() const
    {
        return HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER);
    }
    bool IsQuestGiver() const
    {
        return HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
    }
    bool IsGossip() const
    {
        return HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }
    bool IsTaxi() const
    {
        return HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_FLIGHTMASTER);
    }
    bool IsGuildMaster() const
    {
        return HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_PETITIONER);
    }
    bool IsBattleMaster() const
    {
        return HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_BATTLEMASTER);
    }
    bool IsBanker() const
    {
        return HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_BANKER);
    }
    bool IsInnkeeper() const
    {
        return HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER);
    }
    bool IsSpiritHealer() const
    {
        return HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER);
    }
    bool IsSpiritGuide() const
    {
        return HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITGUIDE);
    }
    bool IsTabardDesigner() const
    {
        return HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_TABARDDESIGNER);
    }
    bool IsAuctioner() const
    {
        return HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_AUCTIONEER);
    }
    bool IsArmorer() const
    {
        return HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_REPAIR);
    }
    bool IsServiceProvider() const;
    bool IsSpiritService() const
    {
        return HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER | UNIT_NPC_FLAG_SPIRITGUIDE);
    }

    bool IsInFlight() const
    {
        return HasUnitState(UNIT_STATE_IN_FLIGHT);
    }

    bool IsInCombat() const
    {
        return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
    }
    void CombatStart(Unit* target, bool initialAggro = true);
    void SetInCombatState(bool PvP, Unit* enemy = NULL);
    void SetInCombatWith(Unit* enemy);
    virtual void ClearInCombat();
    uint32 GetPvPCombatTimer() const { return m_combatTimerPvP; }
    uint32 GetPvECombatTimer() const { return m_combatTimerPvE; }
    void ResetPvECombatTimer() { m_combatTimerPvE = 0; }

    bool HasAuraTypeWithFamilyFlags(AuraType auraType, uint32 familyName, uint32 familyFlags) const;
    bool virtual HasSpell(uint32 /*spellID*/) const { return false; }

    bool HasCrowdControlAuraType(AuraType type, uint32 excludeAura = 0, uint32 dispelType = 0) const;
    bool HasCrowdControlAura(Unit* excludeCasterChannel = NULL, uint32 dispelType = 0) const;
    bool HasBreakableByDamageAuraType(AuraType type, uint32 excludeAura = 0) const;
    bool HasBreakableByDamageCrowdControlAura(Unit* excludeCasterChannel = NULL) const;

    bool isFeared() const
    {
        return HasAuraType(SPELL_AURA_MOD_FEAR) || HasAuraType(SPELL_AURA_MOD_FEAR_2);
    }
    bool isInRoots() const
    {
        return HasAuraType(SPELL_AURA_MOD_ROOT);
    }
    bool IsPolymorphed() const;

    bool isFrozen() const;
    bool IsSef() const;

    bool isTargetableForAttack(bool checkFakeDeath = true) const;

    bool IsValidAttackTarget(Unit const* target) const;
    bool IsValidAttackTarget(Unit const* target, SpellInfo const* bySpell, WorldObject const* obj = nullptr, bool ignoreVisibility = false) const;
    bool IsValidAttackTarget(Unit const* target, SpellInfo const* bySpell, WorldObject const* obj, bool ignoreVisibility, AttackSwingResult* error) const;

    bool IsValidAssistTarget(Unit const* target) const;
    bool _IsValidAssistTarget(Unit const* target, SpellInfo const* bySpell) const;

    bool IsInWater() const;
    bool IsUnderWater() const;
    bool isInAccessiblePlaceFor(Creature const* c) const;

    bool IsInteractionAllowedWhileHostile() const { return HasUnitFlag2(UNIT_FLAG2_ALLOW_ENEMY_INTERACT); }

    void SendHealSpellLog(Unit* victim, uint32 SpellID, uint32 Damage, uint32 OverHeal, uint32 Absorb, bool critical = false);
    int32 HealBySpell(Unit* victim, SpellInfo const* spellInfo, uint32 addHealth, bool critical = false);
    void SendEnergizeSpellLog(Unit* victim, uint32 spellID, int32 damage, Powers powertype);
    void EnergizeBySpell(Unit* victim, uint32 SpellID, int32 Damage, Powers powertype);
    uint32 SpellNonMeleeDamageLog(Unit* victim, uint32 spellID, uint32 damage);

    void CastSpell(SpellCastTargets const& targets, SpellInfo const* spellInfo, CustomSpellValues const* value, TriggerCastFlags triggerFlags = TRIGGERED_NONE, Item* castItem = NULL, AuraEffect const* triggeredByAura = NULL, ObjectGuid originalCaster = ObjectGuid::Empty);
    void CastSpell(Unit* victim, uint32 spellId, bool triggered, Item* castItem = NULL, AuraEffect const* triggeredByAura = NULL, ObjectGuid originalCaster = ObjectGuid::Empty);
    void CastSpell(Unit* victim, uint32 spellId, TriggerCastFlags triggerFlags = TRIGGERED_NONE, Item* castItem = NULL, AuraEffect const* triggeredByAura = NULL, ObjectGuid originalCaster = ObjectGuid::Empty);
    void CastSpell(Unit* victim, SpellInfo const* spellInfo, bool triggered, Item* castItem = NULL, AuraEffect const* triggeredByAura = NULL, ObjectGuid originalCaster = ObjectGuid::Empty);
    void CastSpell(Unit* victim, SpellInfo const* spellInfo, TriggerCastFlags triggerFlags = TRIGGERED_NONE, Item* castItem = NULL, AuraEffect const* triggeredByAura = NULL, ObjectGuid originalCaster = ObjectGuid::Empty);
    void CastSpell(float x, float y, float z, uint32 spellId, bool triggered, Item* castItem = NULL, AuraEffect const* triggeredByAura = NULL, ObjectGuid originalCaster = ObjectGuid::Empty);
    void CastSpell(GameObject* go, uint32 spellId, bool triggered, Item* castItem = NULL, AuraEffect* triggeredByAura = NULL, ObjectGuid originalCaster = ObjectGuid::Empty);
    void CastSpell(Position const &pos, uint32 spellId, TriggerCastFlags triggerFlags = TRIGGERED_NONE, Item* castItem = NULL, AuraEffect const* triggeredByAura = NULL, ObjectGuid originalCaster = ObjectGuid::Empty);

    void CastSpell(Position const &pos, uint32 spellId, bool triggered, Item* castItem = NULL, AuraEffect const* triggeredByAura = NULL, ObjectGuid originalCaster = ObjectGuid::Empty)
    {
        CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), spellId, triggered, castItem, triggeredByAura, originalCaster);
    }

    void CastCustomSpell(Unit* victim, uint32 spellId, int32 const* bp0, int32 const* bp1, int32 const* bp2, bool triggered, Item* castItem = NULL, AuraEffect const* triggeredByAura = NULL, ObjectGuid originalCaster = ObjectGuid::Empty);
    void CastCustomSpell(float x, float y, float z, uint32 spellId, int32 const* bp0, int32 const* bp1, int32 const* bp2, bool triggered, Item* castItem = NULL, AuraEffect const *triggeredByAura = NULL, ObjectGuid originalCaster = ObjectGuid::Empty);
    void CastCustomSpell(uint32 spellId, SpellValueMod mod, int32 value, Unit* victim, bool triggered, Item* castItem = NULL, AuraEffect const* triggeredByAura = NULL, ObjectGuid originalCaster = ObjectGuid::Empty);
    void CastCustomSpell(uint32 spellId, SpellValueMod mod, int32 value, Unit* victim = NULL, TriggerCastFlags triggerFlags = TRIGGERED_NONE, Item* castItem = NULL, AuraEffect const* triggeredByAura = NULL, ObjectGuid originalCaster = ObjectGuid::Empty);
    void CastCustomSpell(uint32 spellId, CustomSpellValues const &value, Unit* victim = NULL, TriggerCastFlags triggerFlags = TRIGGERED_NONE, Item* castItem = NULL, AuraEffect const* triggeredByAura = NULL, ObjectGuid originalCaster = ObjectGuid::Empty);
    void CastCustomSpell(float x, float y, float z, uint32 spellId, CustomSpellValues const &value, bool triggered = true, Item* castItem = NULL, AuraEffect const *triggeredByAura = NULL, ObjectGuid originalCaster = ObjectGuid::Empty);

    Aura* AddAura(uint32 spellId, Unit* target);
    Aura* AddAura(SpellInfo const* spellInfo, uint32 effMask, Unit* target);
    Aura* AddAuraForTarget(Aura* aura, Unit* target);
    void SetAuraStack(uint32 spellId, Unit* target, uint32 stack);
    void SendPlaySpellVisualKit(uint32 SpellVisualId, uint32 Duration, int32 Type);
    void SendPlaySpellVisual(uint32 spellVisualId, ObjectGuid target, float speed, uint16 missReason = 0, uint16 reflectStatus = 0)
    {
        SendPlaySpellVisual(spellVisualId, target, 0.0f, 0.0f, 0.0f, speed, false, missReason, reflectStatus);
    }
    void SendPlaySpellVisual(uint32 spellVisualId, float x, float y, float z, float speed, uint16 missReason = 0, uint16 reflectStatus = 0)
    {
        SendPlaySpellVisual(spellVisualId, ObjectGuid::Empty, x, y, z, speed, true, missReason, reflectStatus);
    }
    void SendPlaySpellVisual(uint32 spellVisualId, ObjectGuid targetGuid, float x, float y, float z, float speed, bool hasDest, uint16 missReason, uint16 reflectStatus);

    void DeMorph();

    void SendAttackStateUpdate(CalcDamageInfo* damageInfo);
    void SendAttackStateUpdate(uint32 HitInfo, Unit* target, uint8 SwingType, SpellSchoolMask damageSchoolMask, uint32 Damage, uint32 AbsorbDamage, uint32 Resist, VictimState TargetState, uint32 BlockedAmount);
    void SendSpellNonMeleeDamageLog(SpellNonMeleeDamage* log);
    void SendSpellNonMeleeDamageLog(Unit* target, uint32 SpellID, uint32 Damage, SpellSchoolMask damageSchoolMask, uint32 AbsorbedDamage, uint32 Resist, bool PhysicalDamage, uint32 Blocked, bool CriticalHit = false);
    void SendPeriodicAuraLog(SpellPeriodicAuraLogInfo* pInfo);
    void SendSpellMiss(Unit* target, uint32 spellID, SpellMissInfo missInfo);
    void SendSpellDamageResist(Unit* target, uint32 spellId);
    void SendSpellDamageImmune(Unit* target, uint32 spellId);

    void NearTeleportTo(float x, float y, float z, float orientation, bool casting = false);
    void SendTeleportPacket(Position& pos);
    void BuildTeleportUpdateData(WorldPacket* data);
    virtual bool UpdatePosition(float x, float y, float z, float ang, bool teleport = false);
    // returns true if unit's position really changed
    virtual bool UpdatePosition(const Position &pos, bool teleport = false);
    void UpdateOrientation(float orientation);
    void UpdateHeight(float newZ);

    void SendMoveKnockBack(Player* player, float speedXY, float speedZ, float vcos, float vsin);
    void KnockbackFrom(float x, float y, float speedXY, float speedZ);
    void JumpTo(float speedXY, float speedZ, bool forward = true, Position* dest = NULL);
    void JumpTo(WorldObject* obj, float speedZ);

    void MonsterMoveWithSpeed(float x, float y, float z, float speed, bool generatePath = false, bool forceDestination = false);
    void SendMovementFlagUpdate(bool self = false);

    void SendSetPlayHoverAnim(bool PlayHoverAnim);

    [[nodiscard]] float GetHoverHeight() const { return IsHovering() ? GetFloatValue(UNIT_FIELD_HOVER_HEIGHT) : 0.0f; }
    bool IsGravityDisabled() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY); }
    bool IsWalking() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_WALKING); }
    bool IsHovering() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_HOVER); }
    bool isSwimming() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_SWIMMING); }
    bool SetWalk(bool enable);
    bool SetDisableGravity(bool disable, bool packetOnly = false);
    bool SetFall(bool enable);
    bool SetSwim(bool enable);
    bool SetCanFly(bool enable);
    bool SetWaterWalking(bool enable, bool packetOnly = false);
    bool SetFeatherFall(bool enable, bool packetOnly = false);
    bool SetHover(bool enable, bool packetOnly = false);
    void SendSetVehicleRecId(uint32 vehicleId);

    void SetInFront(WorldObject const* target);
    void SetFacingTo(float ori);
    void SetFacingToObject(WorldObject* object);

    void SendChangeCurrentVictimOpcode(HostileReference* pHostileReference);
    void SendClearThreatListOpcode();
    void SendRemoveFromThreatListOpcode(HostileReference* pHostileReference);
    void SendThreatListUpdate();

    void SendClearTarget();

    bool IsAlive() const
    {
        return (m_deathState == ALIVE);
    }
    bool isDying() const
    {
        return (m_deathState == JUST_DIED);
    }
    bool isDead() const
    {
        return (m_deathState == DEAD || m_deathState == CORPSE);
    }
    DeathState getDeathState() const
    {
        return m_deathState;
    }
    virtual void setDeathState(DeathState s);           // overwrited in Creature/Player/Pet

    ObjectGuid GetOwnerGUID() const { return GetGuidValue(UNIT_FIELD_SUMMONED_BY); }
    void SetOwnerGUID(ObjectGuid owner);
    ObjectGuid GetCreatorGUID() const { return GetGuidValue(UNIT_FIELD_CREATED_BY); }
    void SetCreatorGUID(ObjectGuid creator) { SetGuidValue(UNIT_FIELD_CREATED_BY, creator); }
    ObjectGuid GetMinionGUID() const { return GetGuidValue(UNIT_FIELD_SUMMON); }
    void SetMinionGUID(ObjectGuid guid) { SetGuidValue(UNIT_FIELD_SUMMON, guid); }
    ObjectGuid GetCharmerGUID() const { return GetGuidValue(UNIT_FIELD_CHARMED_BY); }
    void SetCharmerGUID(ObjectGuid owner) { SetGuidValue(UNIT_FIELD_CHARMED_BY, owner); }
    ObjectGuid GetCharmGUID() const { return GetGuidValue(UNIT_FIELD_CHARM); }

    void SetPetGUID(ObjectGuid guid) { m_SummonSlot [SUMMON_SLOT_PET] = guid; }
    ObjectGuid GetPetGUID() const { return m_SummonSlot [SUMMON_SLOT_PET]; }
    void SetCritterGUID(ObjectGuid guid) { SetGuidValue(UNIT_FIELD_CRITTER, guid); }
    ObjectGuid GetCritterGUID() const { return GetGuidValue(UNIT_FIELD_CRITTER); }

    bool IsControlledByPlayer() const
    {
        return m_ControlledByPlayer;
    }
    ObjectGuid GetCharmerOrOwnerGUID() const;
    ObjectGuid GetCharmerOrOwnerOrOwnGUID() const;
    bool IsCharmedOwnedByPlayerOrPlayer() const { return GetCharmerOrOwnerOrOwnGUID().IsPlayer(); }

    Player* GetSpellModOwner() const;

    Unit* GetOwner() const;
    Guardian* GetGuardianPet() const;
    Minion* GetFirstMinion() const;
    Unit* GetCharmer() const;
    Unit* GetCharm() const;
    Unit* GetCharmerOrOwner() const;
    Unit* GetCharmerOrOwnerOrSelf() const;
    Player* GetCharmerOrOwnerPlayerOrPlayerItself() const;
    Player* GetAffectingPlayer() const;

    void SetMinion(Minion *minion, bool apply);
    void GetAllMinionsByEntry(std::list<Creature*>& Minions, uint32 entry);
    Unit* GetFirstMinionByEntry(uint32 entry);
    void RemoveAllMinionsByEntry(uint32 entry);
    void SetCharm(Unit* target, bool apply);
    Unit* GetNextRandomRaidMemberOrPet(float radius);
    bool SetCharmedBy(Unit* charmer, CharmType type, AuraApplication const* aurApp = NULL);
    void RemoveCharmedBy(Unit* charmer);
    void RestoreFaction();

    ControlList m_Controlled;
    Unit* GetFirstControlled() const;
    void RemoveAllControlled();

    bool IsCharmed() const
    {
        return GetCharmerGUID() != 0;
    }
    bool isPossessed() const
    {
        return HasUnitState(UNIT_STATE_POSSESSED);
    }
    bool isPossessedByPlayer() const;
    bool isPossessing() const;
    bool isPossessing(Unit* u) const;

    CharmInfo* GetCharmInfo() { return m_charmInfo; }
    CharmInfo* InitCharmInfo();
    void DeleteCharmInfo();
    void UpdateCharmAI();
    //Player* GetMoverSource() const;
    Player* m_movedPlayer;
    SharedVisionList const& GetSharedVisionList()
    {
        return m_sharedVision;
    }
    void AddPlayerToVision(Player* player);
    void RemovePlayerFromVision(Player* player);
    bool HasSharedVision() const
    {
        return !m_sharedVision.empty();
    }
    void RemoveBindSightAuras();
    void RemoveCharmAuras();

    Pet* CreateTamedPetFrom(Creature* creatureTarget, uint32 spell_id = 0);
    Pet* CreateTamedPetFrom(uint32 creatureEntry, uint32 spell_id = 0);
    bool InitTamedPet(Pet* pet, uint8 level, uint32 spell_id);

    // aura apply/remove helpers - you should better not use these
    Aura* _TryStackingOrRefreshingExistingAura(SpellInfo const* newAura, uint32 effMask, Unit* caster, int32* baseAmount = NULL, Item* castItem = NULL, ObjectGuid casterGUID = ObjectGuid::Empty);
    void _AddAura(UnitAura* aura, Unit* caster);
    AuraApplication * _CreateAuraApplication(Aura* aura, uint32 effMask);
    void _ApplyAuraEffect(Aura* aura, uint8 effIndex);
    void _ApplyAura(AuraApplication * aurApp, uint32 effMask);
    void _UnapplyAura(AuraApplicationMap::iterator &i, AuraRemoveMode removeMode);
    void _UnapplyAura(AuraApplication * aurApp, AuraRemoveMode removeMode);
    void _RemoveNoStackAurasDueToAura(Aura* aura);
    void _RegisterAuraEffect(AuraEffect* aurEff, bool apply);

    // m_ownedAuras container management
    AuraMap      & GetOwnedAuras()
    {
        return m_ownedAuras;
    }
    AuraMap const& GetOwnedAuras() const
    {
        return m_ownedAuras;
    }

    void RemoveOwnedAura(AuraMap::iterator &i, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
    void RemoveOwnedAura(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, uint32 reqEffMask = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
    void RemoveOwnedAura(Aura* aura, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);

    Aura* GetOwnedAura(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, ObjectGuid itemCasterGUID = ObjectGuid::Empty, uint32 reqEffMask = 0, Aura* except = NULL) const;

    // m_appliedAuras container management
    AuraApplicationMap      & GetAppliedAuras()
    {
        return m_appliedAuras;
    }
    AuraApplicationMap const& GetAppliedAuras() const
    {
        return m_appliedAuras;
    }

    void RemoveAura(AuraApplicationMap::iterator &i, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
    void RemoveAura(uint32 spellId, uint64 casterGUID = 0, uint32 reqEffMask = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
    void RemoveAura(AuraApplication * aurApp, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
    void RemoveAura(Aura* aur, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);

    void RemoveAurasDueToSpell(uint32 spellId, uint64 casterGUID = 0, uint32 reqEffMask = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
    void RemoveAuraFromStack(uint32 spellId, uint64 casterGUID = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT, int32 num = 1);
    void RemoveAurasDueToSpellByDispel(uint32 spellId, uint32 dispellerSpellId, uint64 casterGUID, Unit* dispeller, uint8 chargesRemoved = 1);
    void RemoveAurasDueToSpellBySteal(uint32 spellId, uint64 casterGUID, Unit* stealer);
    void RemoveAurasDueToItemSpell(uint32 spellId, uint64 castItemGuid);
    void RemoveAurasByType(AuraType auraType, uint64 casterGUID = 0, Aura* except = NULL, bool negative = true, bool positive = true);
    void RemoveAurasByTypeOnImmunity(AuraType auraType);
    void RemoveBoundAuras(uint32 newPhase = 0x0, bool phaseid = false);
    void RemoveAurasWithInterruptFlags(SpellAuraInterruptFlags flag, uint32 except = 0, SpellInfo const* bySpell = nullptr)
    {
        RemoveAurasWithInterruptFlags(flag64{ uint32(flag), 0 }, except, bySpell);
    }
    void RemoveAurasWithInterruptFlags(SpellAuraInterruptFlags2 flag, uint32 except = 0, SpellInfo const* bySpell = nullptr)
    {
        RemoveAurasWithInterruptFlags(flag64{ 0, uint32(flag) }, except, bySpell);
    }

private:
    void RemoveAurasWithInterruptFlags(flag64 flag, uint32 except, SpellInfo const* bySpell);
    bool CanAuraBeInterrupted(Aura* aura, flag64 interruptFlag, SpellInfo const* bySpell);
    bool CanChannelBeInterrupted(flag64 flag, uint32 except, SpellInfo const* bySpell);
public:
    void RemoveAurasWithAttribute(uint32 flags);
    void RemoveAurasWithFamily(SpellFamilyNames family, uint32 familyFlag1, uint32 familyFlag2, uint32 familyFlag3, uint64 casterGUID);
    void RemoveAurasWithMechanic(uint32 mechanic_mask, AuraRemoveMode removemode = AURA_REMOVE_BY_DEFAULT, uint32 except = 0, bool partial = false, bool onImmunity = false);
    void RemoveAurasWithMechanicOnImmunity(uint32 mechanicMask, uint32 except = 0, bool partial = false);
    void RemoveMovementImpairingAuras(bool partial = false);

    void RemoveAreaAurasDueToLeaveWorld();
    void RemoveAllAuras();
    void RemoveArenaAuras();
    void RemoveBattlegroundStartingAuras();
    void RemoveAllAurasOnDeath();
    void RemoveAllAurasRequiringDeadTarget();
    void RemoveAllAurasExceptType(AuraType type);
    void RemoveAllAurasExceptType(AuraType type1, AuraType type2); /// @todo: once we support variadic templates use them here
    void DelayOwnedAuras(uint32 spellId, uint64 caster, int32 delaytime);

    void _RemoveAllAuraStatMods();
    void _ApplyAllAuraStatMods();

    void BindAura(Aura* aura);
    void UnbindAura(Aura* aura);

    AuraEffectList const& GetAuraEffectsByType(AuraType type) const
    {
        return m_modAuras [type];
    }

    AuraEffect* GetAuraEffect(uint32 spellId, uint8 effIndex, uint64 casterGUID = 0) const;
    AuraEffect* GetAuraEffectOfRankedSpell(uint32 spellId, uint8 effIndex, uint64 casterGUID = 0) const;
    AuraEffect* GetAuraEffect(AuraType type, SpellFamilyNames name, uint32 iconId, uint8 effIndex) const; // spell mustn't have familyflags
    AuraEffect* GetAuraEffect(AuraType type, SpellFamilyNames family, flag128 const& flags, uint64 casterGUID = 0) const;
    AuraEffect* GetDummyAuraEffect(SpellFamilyNames name, uint32 iconId, uint8 effIndex) const;

    AuraApplication * GetAuraApplication(uint32 spellId, uint64 casterGUID = 0, uint64 itemCasterGUID = 0, uint32 reqEffMask = 0, AuraApplication * except = NULL) const;
    Aura* GetAura(uint32 spellId, uint64 casterGUID = 0, uint64 itemCasterGUID = 0, uint32 reqEffMask = 0) const;

    AuraApplication * GetAuraApplicationOfRankedSpell(uint32 spellId, uint64 casterGUID = 0, uint64 itemCasterGUID = 0, uint32 reqEffMask = 0, AuraApplication * except = NULL) const;
    Aura* GetAuraOfRankedSpell(uint32 spellId, uint64 casterGUID = 0, uint64 itemCasterGUID = 0, uint32 reqEffMask = 0) const;

    AuraList const* GetBoundAurasBySpellId(uint32 id)
    {
        auto it = m_boundAuras.find(id);
        if (it == m_boundAuras.end() || it->second.empty())
            return nullptr;            
        return &it->second;
    }

    void GetDispellableAuraList(Unit* caster, uint32 dispelMask, DispelChargesList& dispelList);

    bool HasAuraEffect(uint32 spellId, uint8 effIndex, uint64 caster = 0) const;
    uint32 GetAuraCount(uint32 spellId) const;
    bool HasAura(uint32 spellId, uint64 casterGUID = 0, uint64 itemCasterGUID = 0, uint32 reqEffMask = 0) const;
    bool HasAuraType(AuraType auraType) const;
    bool HasAuraTypeWithCaster(AuraType auratype, uint64 caster) const;
    bool HasAuraTypeWithMiscvalue(AuraType auratype, int32 miscvalue) const;
    bool HasAuraTypeWithAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
    bool HasAuraTypeWithValue(AuraType auratype, int32 value) const;
    bool HasNegativeAuraWithInterruptFlag(SpellAuraInterruptFlags flag, uint64 guid = 0) const;
    bool HasNegativeAuraWithAttribute(uint32 flag, uint64 guid = 0) const;
    bool HasAuraWithMechanic(uint32 mechanicMask) const;

    AuraEffect* IsScriptOverriden(SpellInfo const* spell, int32 script) const;
    uint32 GetDiseasesByCaster(uint64 casterGUID, bool remove = false);
    uint32 GetDoTsByCaster(uint64 casterGUID) const;

    int32 GetTotalAuraModifier(AuraType auratype) const;
    float GetTotalAuraMultiplier(AuraType auratype) const;
    int32 GetMaxPositiveAuraModifier(AuraType auratype) const;
    int32 GetMaxNegativeAuraModifier(AuraType auratype) const;

    int32 GetTotalAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;
    float GetTotalAuraMultiplierByMiscMask(AuraType auratype, uint32 misc_mask) const;
    int32 GetMaxPositiveAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask, const AuraEffect* except = NULL) const;
    int32 GetMaxNegativeAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;

    int32 GetTotalAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
    float GetTotalAuraMultiplierByMiscValue(AuraType auratype, int32 misc_value) const;
    int32 GetMaxPositiveAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
    int32 GetMaxNegativeAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;

    int32 GetTotalAuraModifierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
    float GetTotalAuraMultiplierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
    int32 GetMaxPositiveAuraModifierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
    int32 GetMaxNegativeAuraModifierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;

    template <class Cond>
    float GetTotalAuraMultiplier(AuraType auraType, Cond cond) const;
    template <class Cond>
    int32 GetTotalAuraModifier(AuraType auraType, Cond cond) const;

    float GetTotalHaseMultiplier(AuraType auraType) const;

    float GetResistanceBuffMods(SpellSchools school, bool positive) const;
    void SetResistanceBuffMods(SpellSchools school, bool positive, float val);
    void ApplyResistanceBuffModsMod(SpellSchools school, bool positive, float val, bool apply);
    void ApplyResistanceBuffModsPercentMod(SpellSchools school, bool positive, float val, bool apply);
    void InitStatBuffMods();
    void ApplyStatBuffMod(Stats stat, float val, bool apply);
    void ApplyStatPercentBuffMod(Stats stat, float val, bool apply);
    void SetCreateStat(Stats stat, float val)
    {
        m_createStats [stat] = val;
    }
    void SetCreateHealth(uint32 val)
    {
        SetUInt32Value(UNIT_FIELD_BASE_HEALTH, val);
    }
    uint32 GetCreateHealth() const
    {
        return GetUInt32Value(UNIT_FIELD_BASE_HEALTH);
    }
    void SetCreateMana(uint32 val)
    {
        SetUInt32Value(UNIT_FIELD_BASE_MANA, val);
    }
    uint32 GetCreateMana() const
    {
        return GetUInt32Value(UNIT_FIELD_BASE_MANA);
    }
    uint32 GetPowerIndex(uint32 powerType) const;
    int32 GetCreatePowers(Powers power) const;
    float GetPosStat(Stats stat) const
    {
        return GetFloatValue((uint16)UNIT_FIELD_STAT_POS_BUFF + (uint16)stat);
    }
    float GetNegStat(Stats stat) const
    {
        return GetFloatValue((uint16)UNIT_FIELD_STAT_NEG_BUFF + (uint16)stat);
    }
    float GetCreateStat(Stats stat) const
    {
        return m_createStats [stat];
    }

    void SetCurrentCastedSpell(Spell* pSpell);
    virtual void ProhibitSpellSchool(SpellSchoolMask /*idSchoolMask*/, uint32 /*unTimeMs*/)
    { }
    void InterruptSpell(CurrentSpellTypes spellType, bool withDelayed = true, bool withInstant = true);
    void FinishSpell(CurrentSpellTypes spellType, bool ok = true);

    // set withDelayed to true to account delayed spells as casted
    // delayed+channeled spells are always accounted as casted
    // we can skip channeled or delayed checks using flags
    bool IsNonMeleeSpellCasted(bool withDelayed, bool skipChanneled = false, bool skipAutorepeat = false, bool isAutoshoot = false, bool skipInstant = true) const;

    // set withDelayed to true to interrupt delayed spells too
    // delayed+channeled spells are always interrupted
    void InterruptNonMeleeSpells(bool withDelayed, uint32 spellid = 0, bool withInstant = true);

    Spell* GetCurrentSpell(CurrentSpellTypes spellType) const
    {
        return m_currentSpells [spellType];
    }
    Spell* GetCurrentSpell(uint32 spellType) const
    {
        return m_currentSpells [spellType];
    }
    Spell* FindCurrentSpellBySpellId(uint32 spell_id) const;
    int32 GetCurrentSpellCastTime(uint32 spell_id) const;

    uint16 _aiAnimKitId;
    uint16 _movementAnimKitId;
    uint16 _meleeAnimKitId;

    std::array<ObjectGuid, SUMMON_SLOT_MAX> m_SummonSlot;
    std::array<ObjectGuid, MAX_GAMEOBJECT_SLOT> m_ObjectSlot;

    ShapeshiftForm GetShapeshiftForm() const
    {
        return ShapeshiftForm(GetByteValue(UNIT_FIELD_BYTES_2, 3));
    }
    void SetShapeshiftForm(ShapeshiftForm form);

    bool IsInFeralForm() const;

    bool IsInDisallowedMountForm() const;

    float m_modMeleeHitChance;
    float m_modRangedHitChance;
    float m_modSpellHitChance;
    int32 m_baseSpellCritChance;

    float m_threatModifier [MAX_SPELL_SCHOOL];
    float m_modAttackSpeedPct [3];

    // Event handler
    EventProcessor m_Events;

    void Schedule(TimeValue const& timer, std::function<void()>const& func) { m_Events.Schedule(timer.ToMilliseconds(), func); }
    void Schedule(TimeValue const& timer, std::function<void(Unit*)> const& func) { m_Events.Schedule(timer.ToMilliseconds(), std::bind(func, this)); }

    // stat system
    bool HandleStatModifier(UnitMods unitMod, UnitModifierType modifierType, float amount, bool apply);
    void SetModifierValue(UnitMods unitMod, UnitModifierType modifierType, float value)
    {
        m_auraModifiersGroup [unitMod] [modifierType] = value;
    }
    float GetModifierValue(UnitMods unitMod, UnitModifierType modifierType) const;
    float GetTotalStatValue(Stats stat) const;
    float GetTotalAuraModValue(UnitMods unitMod) const;
    SpellSchools GetSpellSchoolByAuraGroup(UnitMods unitMod) const;
    Stats GetStatByAuraGroup(UnitMods unitMod) const;
    Powers GetPowerTypeByAuraGroup(UnitMods unitMod) const;
    bool CanModifyStats() const
    {
        return m_canModifyStats;
    }
    void SetCanModifyStats(bool modifyStats)
    {
        m_canModifyStats = modifyStats;
    }
    virtual bool UpdateStats(Stats stat) = 0;
    virtual bool UpdateAllStats() = 0;
    virtual void UpdateResistances(uint32 school) = 0;
    virtual void UpdateArmor() = 0;
    virtual void UpdateMaxHealth() = 0;
    virtual void UpdateMaxPower(Powers power) = 0;
    virtual void UpdateAttackPowerAndDamage(bool ranged = false) = 0;
    virtual void UpdateDamagePhysical(WeaponAttackType attType) = 0;
    void UpdateCastingSpeed();
    void UpdateAttackSpeed(WeaponAttackType att);
    void UpdateHasteRegen();

    // "True" haste. Used for power regen, rppm and global/normal cooldown modifications
    float GetHasteMultiplier() const;
    float GetTotalAttackPowerValue(WeaponAttackType attType) const;
    float GetWeaponDamageRange(WeaponAttackType attType, WeaponDamageRange type) const;
    void SetBaseWeaponDamage(WeaponAttackType attType, WeaponDamageRange damageRange, float value)
    {
        m_weaponDamage [attType] [damageRange] = value;
    }

    bool isInFrontInMap(Unit const* target, float distance, float arc = M_PI) const;
    bool isInBackInMap(Unit const* target, float distance, float arc = M_PI) const;

    // Visibility system
    bool IsVisible() const;
    void SetVisible(bool x);
    void ClearPhases(bool update = false);
    bool SetPhased(uint32 id, bool update, bool apply);

    // common function for visibility checks for player/creatures with detection code
    void SetPhaseMask(uint32 newPhaseMask, bool update) override;// overwrite WorldObject::SetPhaseMask
    void UpdateObjectVisibility(bool forced = true) override;

    SpellImmuneList m_spellImmune [MAX_SPELL_IMMUNITY];
    uint32 m_lastSanctuaryTime;

    // Threat related methods
    bool CanHaveThreatList() const;
    void AddThreat(Unit* victim, float fThreat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL, SpellInfo const* threatSpell = NULL);
    float ApplyTotalThreatModifier(float fThreat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL);
    void DeleteThreatList();
    void TauntApply(Unit* victim);
    void TauntFadeOut(Unit* taunter);
    ThreatManager& GetThreatManager() { return m_ThreatManager; }
    ThreatManager const& GetThreatManager() const { return m_ThreatManager; }
    void addHatedBy(HostileReference* pHostileReference)
    {
        m_HostileRefManager.insertFirst(pHostileReference);
    };
    void removeHatedBy(HostileReference* /*pHostileReference*/)
    { /* nothing to do yet */
    }
    HostileRefManager& getHostileRefManager()
    {
        return m_HostileRefManager;
    }

    VisibleAuraMap const* GetVisibleAuras()
    {
        return &m_visibleAuras;
    }
    AuraApplication * GetVisibleAura(uint8 slot) const;
    void SetVisibleAura(uint8 slot, AuraApplication * aur);
    void RemoveVisibleAura(uint8 slot);

    void AddInterruptMask(flag64 const& mask)
    {
        m_interruptMask |= mask;
    }
    void UpdateInterruptMask();
    bool HasVisionObscured(Unit const* target) const;

    uint32 GetDisplayId() const
    {
        return GetUInt32Value(UNIT_FIELD_DISPLAY_ID);
    }
    virtual void SetDisplayId(uint32 modelId, float displayScale = 1.f);
    uint32 GetNativeDisplayId() const { return GetUInt32Value(UNIT_FIELD_NATIVE_DISPLAY_ID); }
    float GetNativeDisplayScale() const { return 1.0f /**GetFloatValue(UNIT_FIELD_NATIVE_X_DISPLAY_SCALE)**/; }    
    void RestoreDisplayId();
    void SetNativeDisplayId(uint32 displayId, float displayScale = 1.f) { SetUInt32Value(UNIT_FIELD_NATIVE_DISPLAY_ID, displayId); /**SetFloatValue(UNIT_FIELD_NATIVE_X_DISPLAY_SCALE, displayScale);**/ }

    void setTransForm(uint32 spellid)
    {
        m_transform = spellid;
    }
    uint32 getTransForm() const
    {
        return m_transform;
    }

    void AddSummon(TempSummon* summon) { m_summons.push_back(summon); }
    void RemoveSummon(TempSummon* summon) { m_summons.remove(summon); }
    void GetSummons(std::list<TempSummon*>& list, uint32 entry);
    std::list<TempSummon*> const& GetSummons() const { return m_summons; }

    // DynamicObject management
    void _RegisterDynObject(DynamicObject* dynObj);
    void _UnregisterDynObject(DynamicObject* dynObj);
    DynamicObject* GetDynObject(uint32 spellId);
    int32 CountDynObject(uint32 spellId);
    void GetDynObjectList(std::list<DynamicObject*> &list, uint32 spellId);
    void RemoveDynObject(uint32 spellId);
    void RemoveAllDynObjects();

    // AreaTrigger management
    void RegisterAreaTrigger(AreaTrigger* areaTrigger);
    void UnregisterAreaTrigger(AreaTrigger* areaTrigger);
    AreaTrigger* GetAreaTrigger(uint32 spellId);
    int32 CountAreaTrigger(uint32 spellId);
    void GetAreaTriggerList(std::list<AreaTrigger*> &list, uint32 spellId);
    void RemoveAreaTrigger(uint32 spellId);
    void RemoveAreaTrigger(AuraEffect const* eff);
    void RemoveAllAreasTrigger();

    void AddAuraAreaTrigger(IAreaTrigger* interfaceAreaTrigger);
    IAreaTrigger* RemoveAuraAreaTrigger(AuraEffect const* auraEffect, AuraApplication const* auraApplication);

    GameObject* GetGameObject(uint32 spellId) const;
    void AddGameObject(GameObject* gameObj);
    void RemoveGameObject(GameObject* gameObj, bool del);
    void RemoveGameObject(uint32 spellid, bool del);
    void RemoveAllGameObjects();

    uint32 CalculateDamage(WeaponAttackType attType, bool normalized, bool addTotalPct);
    float GetAPMultiplier(WeaponAttackType attType, bool normalized);
    void ModifyAuraState(AuraStateType flag, bool apply);
    uint32 BuildAuraStateUpdateForTarget(Unit* target) const;
    bool HasAuraState(AuraStateType flag, SpellInfo const* spellProto = NULL, Unit const* Caster = NULL) const;
    void UnsummonAllTotems();
    Unit* GetMagicHitRedirectTarget(Unit* victim, SpellInfo const* spellInfo);
    Unit* GetMeleeHitRedirectTarget(Unit* victim, SpellInfo const* spellInfo = NULL);

    int32 SpellBaseDamageBonusDone(SpellSchoolMask schoolMask) const;
    int32 SpellBaseDamageBonusTaken(SpellSchoolMask schoolMask) const;
    int32 SpellDamageBonusDone(Unit* victim, SpellInfo const* spellProto, uint32 effIndex, int32 damage, DamageEffectType damagetype, uint32 stack = 1) const;
    int32 SpellDamageBonusTaken(Unit* caster, SpellInfo const* spellProto, uint32 effIndex, int32 damage, DamageEffectType damagetype, uint32 stack = 1) const;
    int32 SpellBaseHealingBonusDone(SpellSchoolMask schoolMask) const;
    int32 SpellBaseHealingBonusTaken(SpellSchoolMask schoolMask) const;
    uint32 SpellHealingBonusDone(Unit* victim, SpellInfo const* spellProto, uint32 effIndex, uint32 healamount, DamageEffectType damagetype, uint32 stack = 1) const;
    uint32 SpellHealingBonusTaken(Unit* caster, SpellInfo const* spellProt, uint32 effIndexo, uint32 healamount, DamageEffectType damagetype, uint32 stack = 1) const;
    uint32 SpellAbsorbBonusDone(Unit* victim, SpellInfo const* spellProto, uint32 effIndex, uint32 absorbAmount, uint32 stack = 1) const;

    uint32 MeleeDamageBonusDone(Unit* pVictim, uint32 damage, WeaponAttackType attType, SpellInfo const* spellProto = NULL);
    uint32 MeleeDamageBonusTaken(Unit* attacker, uint32 pdamage, WeaponAttackType attType, SpellInfo const* spellProto = NULL);


    bool   isSpellBlocked(Unit* victim, SpellInfo const* spellProto, WeaponAttackType attackType = BASE_ATTACK);
    bool   isBlockCritical();
    bool   isSpellCrit(Unit* victim, SpellInfo const* spellProto, SpellSchoolMask schoolMask, WeaponAttackType attackType = BASE_ATTACK) const;
    float GetSpellCrit(Unit* victim, SpellInfo const* spellProto, SpellSchoolMask schoolMask, WeaponAttackType attackType = BASE_ATTACK, bool periodic = false) const;
    int32 SpellCriticalDamageBonus(SpellInfo const* spellProto, int32 damage, Unit* victim);
    uint32 SpellCriticalHealingBonus(SpellInfo const* spellProto, uint32 damage, Unit* victim);

    void SetContestedPvP(Player* attackedPlayer = NULL);

    uint32 GetCastingTimeForBonus(SpellInfo const* spellProto, DamageEffectType damagetype, uint32 CastingTime) const;

    RemainingPeriodicAmount GetRemainingPeriodicAmount(uint64 caster, uint32 spellId, AuraType auraType, uint8 effectIndex = 0) const;
    int32 GetSplineDuration() const;

    void ApplyUberImmune(uint32 spellid, bool apply);
    void ApplySpellImmune(uint32 spellId, uint32 op, uint32 type, bool apply);
    void ApplySpellDispelImmunity(const SpellInfo* spellProto, DispelType type, bool apply);
    virtual bool IsImmunedToSpell(SpellInfo const* spellInfo, uint32 effectMask) const; // redefined in Creature

    uint32 GetSchoolImmunityMask() const;
    uint32 GetMechanicImmunityMask() const;

    bool IsImmunedToDamage(SpellSchoolMask meleeSchoolMask) const;
    bool IsImmunedToDamage(SpellInfo const* spellInfo) const;
    virtual bool IsImmunedToSpellEffect(SpellInfo const* spellInfo, uint32 index) const; // redefined in Creature

    static bool IsDamageReducedByArmor(SpellSchoolMask damageSchoolMask, SpellInfo const* spellInfo = NULL, uint8 effIndex = MAX_SPELL_EFFECTS);
    uint32 CalcArmorReducedDamage(Unit* victim, const uint32 damage, SpellInfo const* spellInfo, WeaponAttackType attackType = MAX_ATTACK);
    uint32 CalcSpellResistance(Unit* victim, SpellSchoolMask schoolMask, SpellInfo const* spellInfo) const;
    void CalcAbsorbResist(Unit* victim, SpellSchoolMask schoolMask, DamageEffectType damagetype, uint32 const damage, uint32* absorb, uint32* resist, SpellInfo const* spellInfo = nullptr, uint32 procVictim = 0, uint32 procEx = 0);
    void CalcHealAbsorb(Unit* victim, SpellInfo const* spellInfo, uint32& healAmount, uint32& absorb);

    void  UpdateSpeed(UnitMoveType mtype, bool forced);
    float GetSpeed(UnitMoveType mtype) const;
    float GetSpeedRate(UnitMoveType mtype) const
    {
        return m_speed_rate [mtype];
    }
    void SetSpeed(UnitMoveType mtype, float rate, bool forced = false);

    float ApplyEffectModifiers(SpellInfo const* spellProto, uint8 effect_index, float value) const;
    int32 CalculateSpellDamage(Unit const* target, SpellInfo const* spellProto, uint8 effect_index, int32 const* basePoints = nullptr, Item const* castItem = nullptr) const;
    int32 CalcSpellDuration(SpellInfo const* spellProto);
    int32 ModSpellDuration(SpellInfo const* spellProto, Unit const* target, int32 duration, bool positive, uint32 effectMask);
    void  ModSpellCastTime(SpellInfo const* spellProto, int32& castTime, Spell* spell = NULL);
    float CalculateLevelPenalty(SpellInfo const* spellProto) const;

    void addFollower(FollowerReference* pRef)
    {
        m_FollowingRefManager.insertFirst(pRef);
    }
    void removeFollower(FollowerReference* /*pRef*/)
    { /* nothing to do yet */
    }
    static Unit* GetUnit(WorldObject& object, ObjectGuid guid);
    static Player* GetPlayer(WorldObject& object, ObjectGuid guid);
    static Creature* GetCreature(WorldObject& object, ObjectGuid guid);

    MotionMaster* GetMotionMaster()
    {
        return i_motionMaster;
    }
    const MotionMaster* GetMotionMaster() const
    {
        return i_motionMaster;
    }

    bool IsStopped() const { return !(HasUnitState(UNIT_STATE_MOVING)); }
    void StopMoving();
    void PauseMovement(uint32 timer = 0, uint8 slot = 0, bool forced = true); // timer in ms
    void ResumeMovement(uint32 timer = 0, uint8 slot = 0); // timer in ms

    void AddUnitMovementFlag(uint32 f) { m_movementInfo.AddMovementFlag(f); }
    void RemoveUnitMovementFlag(uint32 f) { m_movementInfo.RemoveMovementFlag(f); }
    bool HasUnitMovementFlag(uint32 f) const { return m_movementInfo.HasMovementFlag(f); }
    uint32 GetUnitMovementFlags() const
    {
        return m_movementInfo.GetMovementFlags();
    }
    void SetUnitMovementFlags(uint32 f)
    {
        m_movementInfo.SetMovementFlags(f);
    }

    void AddExtraUnitMovementFlag(uint16 f)
    {
        m_movementInfo.AddExtraMovementFlag(f);
    }
    void RemoveExtraUnitMovementFlag(uint16 f)
    {
        m_movementInfo.RemoveExtraMovementFlag(f);
    }
    uint16 HasExtraUnitMovementFlag(uint16 f) const
    {
        return m_movementInfo.HasExtraMovementFlag(f);
    }
    uint16 GetExtraUnitMovementFlags() const
    {
        return m_movementInfo.GetExtraMovementFlags();
    }
    void SetExtraUnitMovementFlags(uint16 f)
    {
        m_movementInfo.SetExtraMovementFlags(f);
    }
    bool IsSplineEnabled() const;

    float GetPositionZMinusOffset() const;

    void SetControlled(bool apply, UnitState state);

    void AddComboPointHolder(ObjectGuid lowguid) { m_comboPointHolders.insert(lowguid); }
    void RemoveComboPointHolder(ObjectGuid lowguid) { m_comboPointHolders.erase(lowguid); }
    void ClearComboPointHolders();

    ///----------Pet responses methods-----------------
    void SendPetActionFeedback(uint8 msg);
    void SendPetTalk(uint32 pettalk);
    void SendPetAIReaction(ObjectGuid UnitGUID);
    ///----------End of Pet responses methods----------

    void propagateSpeedChange()
    {
        GetMotionMaster()->propagateSpeedChange();
    }

    // reactive attacks
    void ClearAllReactives();
    void StartReactiveTimer(ReactiveType reactive)
    {
        m_reactiveTimer [reactive] = REACTIVE_TIMER_START;
    }
    void UpdateReactives(uint32 p_time);

    // group updates
    void UpdateAuraForGroup(uint8 slot);

    // proc trigger system
    bool CanProc() const
    {
        return !m_procDeep;
    }
    void SetCantProc(bool apply);

    // pet auras
    typedef std::set<PetAura const*> PetAuraSet;
    PetAuraSet m_petAuras;
    void AddPetAura(PetAura const* petSpell);
    void RemovePetAura(PetAura const* petSpell);

    uint32 GetModelForForm(ShapeshiftForm form) const;
    uint32 GetModelForTotem(uint32 totemType) const;

    // Redirect Threat
    void SetRedirectThreat(ObjectGuid guid, uint32 pct)
    {
        _redirectThreatInfo.TargetGUID = guid;
        _redirectThreatInfo.ThreatPct = pct; 
    }
    void ResetRedirectThreat()
    {
        _redirectThreatInfo.TargetGUID.Clear();
    }
    uint32 GetRedirectThreatPercent() const
    {
        return _redirectThreatInfo.ThreatPct;
    }
    Unit* GetRedirectThreatTarget();

    friend class VehicleJoinEvent;
    bool IsAIEnabled, NeedChangeAI;
    ObjectGuid LastCharmerGUID;
    bool CreateVehicleKit(uint32 id, uint32 creatureEntry, bool loading = false);
    void RemoveVehicleKit(bool remove = false);
    Vehicle* GetVehicleKit() const
    {
        return m_vehicleKit.get();
    }
    Vehicle* GetVehicle() const
    {
        return m_vehicle;
    }
    void SetVehicle(Vehicle* vehicle)
    {
        m_vehicle = vehicle;
    }
    bool IsOnVehicle() const { return m_vehicle != NULL; }
    bool IsOnVehicle(const Unit* vehicle) const;
    Unit* GetVehicleBase()  const;
    Creature* GetVehicleCreatureBase() const;
    ObjectGuid GetTransGUID() const override;
    /// Returns the transport this unit is on directly (if on vehicle and transport, return vehicle)
    TransportBase* GetDirectTransport() const;

    bool m_ControlledByPlayer;

    bool HandleSpellClick(Unit* clicker, int8 seatId = -1);
    void EnterVehicle(Unit* base, int8 seatId = -1, bool fullTriggered = false);
    void ExitVehicle(Position const* exitPosition = NULL);
    void ChangeSeat(int8 seatId, bool next = true);

    // Should only be called by AuraEffect::HandleAuraControlVehicle(AuraApplication const* auraApp, uint8 mode, bool apply) const;
    void _ExitVehicle(Position const* exitPosition = NULL);
    void _EnterVehicle(Vehicle* vehicle, int8 seatId, AuraApplication const* aurApp = NULL);

    void WriteMovementInfo(WorldPacket& data, Movement::ExtraMovementStatusElement* extras = NULL);

    bool isMoving() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_MASK_MOVING); }
    bool isTurning() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_MASK_TURNING); }
    virtual bool CanFly() const = 0;
    bool IsFlying() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_DISABLE_GRAVITY); }
    bool IsFalling() const;
    virtual bool CanEnterWater() const = 0;
    virtual bool CanSwim() const;    

    float GetHoverOffset() const
    {
        return HasUnitMovementFlag(MOVEMENTFLAG_HOVER) ? GetFloatValue(UNIT_FIELD_HOVER_HEIGHT) : 0.0f;
    }

    void RewardRage(float baseRage, bool attacker);

    virtual float GetFollowAngle() const
    {
        return static_cast<float>(M_PI / 2);
    }

    bool CanLeadFollowTarget(Unit* target = nullptr) const;

    void OutDebugInfo() const;
    virtual bool isBeingLoaded() const
    {
        return false;
    }
    bool IsDuringRemoveFromWorld() const
    {
        return m_duringRemoveFromWorld;
    }

    Pet* ToPet() { if (IsPet()) return reinterpret_cast<Pet*>(this); else return nullptr; }
    Pet const* ToPet() const { if (IsPet()) return reinterpret_cast<Pet const*>(this); else return nullptr; }

    Totem* ToTotem() { if (IsTotem()) return reinterpret_cast<Totem*>(this); else return nullptr; }
    Totem const* ToTotem() const { if (IsTotem()) return reinterpret_cast<Totem const*>(this); else return nullptr; }

    TempSummon* ToTempSummon() { if (IsSummon()) return reinterpret_cast<TempSummon*>(this); else return nullptr; }
    TempSummon const* ToTempSummon() const { if (IsSummon()) return reinterpret_cast<TempSummon const*>(this); else return nullptr; }

    ObjectGuid GetTarget() const { return GetGuidValue(UNIT_FIELD_TARGET); }
    virtual void SetTarget(ObjectGuid /*guid*/) { };

    void OnRelocated();

    // Movement info
    Movement::MoveSpline* movespline;

    virtual void Talk(std::string const& text, ChatMsg msgType, Language language, float textRange, WorldObject const* target);
    virtual void Say(std::string const& text, Language language, WorldObject const* target = nullptr);
    virtual void Yell(std::string const& text, Language language, WorldObject const* target = nullptr);
    virtual void TextEmote(std::string const& text, WorldObject const* target = nullptr, bool isBossEmote = false);
    virtual void Whisper(std::string const& text, Language language, Player* target, bool isBossWhisper = false);
    virtual void Talk(uint32 textId, ChatMsg msgType, float textRange, WorldObject const* target);
    virtual void Say(uint32 textId, WorldObject const* target = nullptr);
    virtual void Yell(uint32 textId, WorldObject const* target = nullptr);
    virtual void TextEmote(uint32 textId, WorldObject const* target = nullptr, bool isBossEmote = false);
    virtual void Whisper(uint32 textId, Player* target, bool isBossWhisper = false);

    float GetCollisionHeight() const override;

    uint32 GetMovementCounter() const
    {
        return m_movementCounter;
    }

    uint32 GetAutoAttackSpell(WeaponAttackType attType) const { return attType == OFF_ATTACK ? m_autoattackOverrideOffhandSpell : m_autoattackOverrideSpell; }

    void SetAutoattackOverrideSpell(uint32 spellId, uint32 offhandSpellId)
    {
        m_autoattackOverrideSpell = spellId;
        m_autoattackOverrideOffhandSpell = offhandSpellId;
    }

    void SetAutoattackOverrideRange(uint32 range)
    {
        m_autoattackOverrideRange = range;
    }

    uint32 GetNpcDamageTakenInPastSecs(uint32 secs) const;
    uint32 GetPlayerDamageTakenInPastSecs(uint32 secs) const;
    uint32 GetDamageTakenInPastSecs(uint32 secs) const;

    bool HasStealthBreakImmunity() const;

    float GetTotalAuraEffectValue(AuraType type) const
    {
        auto it = m_totalAuraEffectValue.find(type);

        if (it == m_totalAuraEffectValue.end())
            return 0.0f;
        return it->second;
    }

    void SetTotalAuraEffectValue(AuraType type, float val)
    {
        m_totalAuraEffectValue[type] = val;
    }

    bool VisualizePathfinding = false;
    bool IcyVeinsHack = false;

    void SendMissileCancel(uint32 spellId, bool cancel = true);
    void SendResumeCastBar(Player* player);

    virtual SpellSchoolMask GetMeleeDamageSchoolMask() const;

    SpellHistory* GetSpellHistory() const { return m_spellHistory.get(); }

    float GetScallingDamageMod() const;

    bool PetNeedsLOSCheckOnAttackStart() const;

    void HandleEmoteCommandWithDelay(uint32 Delay, uint8 Id);
    void CastWithDelay(uint32 delay, Unit* victim, uint32 spellid, bool triggered = false, bool repeat = false);
    void JumpWithDelay(uint32 delay, float x, float y, float z, float speedXY, float speedZ, uint32 id);

protected:
    explicit Unit (bool isWorldObject);

    void BuildValuesUpdate(uint8 updatetype, ByteBuffer* data, Player* target) const override;

    UnitAI* i_AI, *i_disabledAI;

    void _UpdateSpells(uint32 time);
    void _DeleteRemovedAuras();

    void _UpdateAutoRepeatSpell();

    uint32 m_attackTimer[MAX_ATTACK];
    uint32 m_attackTimerReminder[MAX_ATTACK];

    float m_createStats [MAX_STATS];

    AttackerSet m_attackers;
    Unit* m_attacking;

    DeathState m_deathState;

    int32 m_procDeep;

    std::list<TempSummon*> m_summons;

    typedef std::list<DynamicObject*> DynObjectList;
    DynObjectList m_dynObj;

    typedef std::list<AreaTrigger*> AreaTriggerList;
    AreaTriggerList m_areaTrigger;

    typedef std::list<GameObject*> GameObjectList;
    GameObjectList m_gameObj;
    bool m_isSorted;
    uint32 m_transform;

    Spell* m_currentSpells [CURRENT_MAX_SPELL];
    uint32 m_autoattackOverrideSpell = 0;
    uint32 m_autoattackOverrideOffhandSpell = 0;
    uint32 m_autoattackOverrideRange = 0;

    AuraMap m_ownedAuras;
    AuraApplicationMap m_appliedAuras;
    AuraApplicationMap m_procAuras[uint32(ProcPhase::Max)];
    AuraList m_removedAuras;
    AuraMap::iterator m_auraUpdateIterator;
    uint32 m_removedAurasCount;

    AuraEffectList m_modAuras [TOTAL_AURAS];
    AuraApplicationList m_interruptableAuras;             // auras which have interrupt mask applied on unit
    AuraStateAurasMap m_auraStateAuras;        // Used for improve performance of aura state checks on aura apply/remove
    std::map<uint32, AuraList> m_boundAuras;
    std::map<AuraType, float> m_totalAuraEffectValue;
    flag64 m_interruptMask;

    float m_auraModifiersGroup [UNIT_MOD_END] [MODIFIER_TYPE_END];
    float m_weaponDamage [MAX_ATTACK] [2];
    bool m_canModifyStats;
    VisibleAuraMap m_visibleAuras;

    float m_speed_rate [MAX_MOVE_TYPE];

    CharmInfo* m_charmInfo;
    SharedVisionList m_sharedVision;

    MotionMaster* i_motionMaster;

    uint32 m_reactiveTimer [MAX_REACTIVE];
    uint32 m_regenTimer;

    ThreatManager m_ThreatManager;

    Vehicle* m_vehicle;
    std::shared_ptr<Vehicle> m_vehicleKit;

    uint32 m_unitTypeMask;
    LiquidTypeEntry const* _lastLiquid;

    bool IsAlwaysVisibleFor(WorldObject const* seer) const override;
    bool IsAlwaysDetectableFor(WorldObject const* seer) const override;

    void DisableSpline();

    void ProcessPositionDataChanged(PositionFullTerrainStatus const& data) override;
    virtual void ProcessTerrainStatusUpdate(ZLiquidStatus oldLiquidStatus, Optional<LiquidData> const& newLiquidData);

private:
    bool IsTriggeredAtSpellProcEvent(Unit* victim, ProcTriggeredData& triggerData, SpellInfo const* procSpell, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, bool isVictim, bool active);
    bool HandleAuraProcOnPowerAmount(Unit* victim, uint32 damage, AuraEffect* triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown, ProcTriggerContext const& context);
    bool HandleDummyAuraProc(Unit* victim, uint32 damage, AuraEffect* triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown, ProcTriggerContext const& context);
    bool HandleAuraProc(Unit* victim, uint32 damage, Aura* triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown, bool * handled, ProcTriggerContext const& context);
    bool HandleProcTriggerSpell(Unit* victim, uint32 damage, AuraEffect* triggeredByAura, SpellInfo const* procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown, ProcTriggerContext const& context);
    bool HandleOverrideClassScriptAuraProc(Unit* victim, uint32 damage, AuraEffect* triggeredByAura, SpellInfo const* procSpell, uint32 cooldown, ProcTriggerContext const& context);
    bool HandleAuraRaidProcFromChargeWithValue(AuraEffect* triggeredByAura);
    bool HandleAuraRaidProcFromCharge(AuraEffect* triggeredByAura);

    void UpdateSplineMovement(uint32 t_diff);
    void UpdateSplinePosition();

    void GetMovementSpeedModifiers(int32& mainBonus, float& stackBonus, float& nonStackBonus);

    // player or player's pet
    float GetCombatRatingReduction(CombatRating cr) const;

protected:
    void SetFeared(bool apply);
    void SetConfused(bool apply);
    void SetStunned(bool apply);
    void SetRooted(bool apply, bool packetOnly = false);

    uint32 m_movementCounter;       ///< Incrementing counter used in movement packets

private:
    class AINotifyTask;
    class VisibilityUpdateTask;
    Position m_lastVisibilityUpdPos;
    bool m_VisibilityUpdScheduled;

    uint32 m_state;                                     // Even derived shouldn't modify
    uint32 m_combatTimerPvP = 0;
    uint32 m_combatTimerPvE = 0;
    TimeTrackerSmall m_splineSyncTimer;

    Diminishing m_Diminishing;
    // Manage all Units that are threatened by us
    HostileRefManager m_HostileRefManager;

    FollowerRefManager m_FollowingRefManager;

    GuidSet m_comboPointHolders;
    uint32 m_comboPointResetTimer = 0;

    std::unique_ptr<SpellHistory> m_spellHistory;

    RedirectThreatInfo _redirectThreatInfo;

    bool m_cleanupDone; // lock made to not add stuff after cleanup before delete
    bool m_duringRemoveFromWorld; // lock made to not add stuff after begining removing from world

    bool _isWalkingBeforeCharm; // Are we walking before we were charmed?

    enum DamageTrackingInfo
    {
        DAMAGE_TRACKING_PERIOD = 120,
        DAMAGE_TRACKING_UPDATE_INTERVAL = 1 * IN_MILLISECONDS
    };

    int32 _damageTrackingTimer = 0;

    std::array<uint32, DAMAGE_TRACKING_PERIOD> _playerDamageTaken;
    std::array<uint32, DAMAGE_TRACKING_PERIOD> _npcDamageTaken;

    // used to track total damage each player has made to the unit
    std::map<uint64, uint32> _playerTotalDamage;
    FunctionProcessor _functionsDelayed;
};

namespace Trinity
{
    // Binary predicate for sorting Units based on percent value of a power
    class PowerPctOrderPred
    {
        public:
            PowerPctOrderPred(Powers power, bool ascending = true) : m_power(power), m_ascending(ascending) { }

            bool operator()(WorldObject const* objA, WorldObject const* objB) const
            {
                Unit const* a = objA->ToUnit();
                Unit const* b = objB->ToUnit();
                float rA = a ? a->GetPowerPct(m_power) : 0.0f;
                float rB = b ? b->GetPowerPct(m_power) : 0.0f;
                return m_ascending ? rA < rB : rA > rB;
            }

            bool operator() (const Unit* a, const Unit* b) const
            {
                float rA = a->GetMaxPower(m_power) ? float(a->GetPower(m_power)) / float(a->GetMaxPower(m_power)) : 0.0f;
                float rB = b->GetMaxPower(m_power) ? float(b->GetPower(m_power)) / float(b->GetMaxPower(m_power)) : 0.0f;
                return m_ascending ? rA < rB : rA > rB;
            }
            private:
            const Powers m_power;
            const bool m_ascending;
    };

    // Binary predicate for sorting Units based on percent value of health
    class HealthPctOrderPred
    {
        public:
            HealthPctOrderPred(bool ascending = true) : m_ascending(ascending) { }

            bool operator()(WorldObject const* objA, WorldObject const* objB) const
            {
                Unit const* a = objA->ToUnit();
                Unit const* b = objB->ToUnit();
                float rA = (a && a->GetMaxHealth()) ? float(a->GetHealth()) / float(a->GetMaxHealth()) : 0.0f;
                float rB = (b && b->GetMaxHealth()) ? float(b->GetHealth()) / float(b->GetMaxHealth()) : 0.0f;
                return m_ascending ? rA < rB : rA > rB;
            }

            bool operator() (const Unit* a, const Unit* b) const
            {
                float rA = a->GetMaxHealth() ? float(a->GetHealth()) / float(a->GetMaxHealth()) : 0.0f;
                float rB = b->GetMaxHealth() ? float(b->GetHealth()) / float(b->GetMaxHealth()) : 0.0f;
                return m_ascending ? rA < rB : rA > rB;
            }
        private:
            const bool m_ascending;
    };

    struct HealthPctOrderWorldObjectPred
    {
        HealthPctOrderWorldObjectPred(bool ascending = true) : m_ascending(ascending) { }

        bool operator()(WorldObject const* l, WorldObject const* r)
        {
            Unit const* a = l->ToUnit();
            Unit const* b = r->ToUnit();

            if (!a)
                return false;
            if (!b)
                return true;

            float rA = a->GetMaxHealth() ? float(a->GetHealth()) / float(a->GetMaxHealth()) : 0.0f;
            float rB = b->GetMaxHealth() ? float(b->GetHealth()) / float(b->GetMaxHealth()) : 0.0f;
            return m_ascending ? rA < rB : rA > rB;
        }

        private:
            bool m_ascending;
    };

    // Binary predicate for sorting Units based on value of distance of an GameObject
    class DistanceCompareOrderPred
    {
        public:
            DistanceCompareOrderPred(const WorldObject* object, bool ascending = true) : m_object(object), m_ascending(ascending) {}
            bool operator() (const WorldObject* a, const WorldObject* b) const
            {
                return m_ascending ? a->GetExactDist(m_object) < b->GetExactDist(m_object) :
                    a->GetExactDist(m_object) > b->GetExactDist(m_object);
            }
        private:
            const WorldObject* m_object;
            const bool m_ascending;
    };
}

// Till the normal implementation (that is, perhaps forever)
inline bool IsNoFlyZone(uint32 zoneId)
{
    switch (zoneId)
    {
        case 3430: // Eversong Woods
        case 3431: // Sunstrider Isle
        case 3433: // Ghostlands
        case 3487: // Silvermoon City
        case 4080: // Isle of Quel'Danas
        case 6757: // Timeless Isle
        case 6661: // Isle of Giants
            return true;;
        default:
            break;
    }
    return false;
}

#endif
