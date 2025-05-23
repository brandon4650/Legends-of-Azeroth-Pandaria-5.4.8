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

#include "ScriptPCH.h"
#include "pit_of_saron.h"
#include "Vehicle.h"

enum Spells
{
    SPELL_MIGHTY_KICK                           = 69021, // ick's spell
    SPELL_SHADOW_BOLT                           = 69028, // krick's spell
    SPELL_TOXIC_WASTE                           = 69024, // krick's spell
    SPELL_EXPLOSIVE_BARRAGE_KRICK               = 69012, // special spell 1
    SPELL_EXPLOSIVE_BARRAGE_ICK                 = 69263, // special spell 1
    SPELL_POISON_NOVA                           = 68989, // special spell 2
    SPELL_PURSUIT                               = 68987, // special spell 3

    SPELL_EXPLOSIVE_BARRAGE_SUMMON              = 69015,
    SPELL_EXPLODING_ORB                         = 69017, // visual on exploding orb
    SPELL_AUTO_GROW                             = 69020, // grow effect on exploding orb
    SPELL_HASTY_GROW                            = 44851, // need to check growing stacks
    SPELL_EXPLOSIVE_BARRAGE_DAMAGE              = 69019, // damage done by orb while exploding

    SPELL_STRANGULATING                         = 69413, // krick's selfcast in intro
    SPELL_SUICIDE                               = 7,
    SPELL_KRICK_KILL_CREDIT                     = 71308,
    SPELL_NECROMANTIC_POWER                     = 69753,
};

enum Yells
{
    // Krick
    SAY_KRICK_AGGRO                             = 0,
    SAY_KRICK_SLAY                              = 1,
    SAY_KRICK_BARRAGE_1                         = 2,
    SAY_KRICK_BARRAGE_2                         = 3,
    SAY_KRICK_POISON_NOVA                       = 4,
    SAY_KRICK_CHASE                             = 5,
    SAY_KRICK_OUTRO_1                           = 6,
    SAY_KRICK_OUTRO_3                           = 7,
    SAY_KRICK_OUTRO_5                           = 8,
    SAY_KRICK_OUTRO_8                           = 9,

    // Ick
    SAY_ICK_POISON_NOVA                         = 0,
    SAY_ICK_CHASE_1                             = 1,

    // Jayna
    SAY_JAYNA_OUTRO_2                           = 0,
    SAY_JAYNA_OUTRO_4                           = 1,
    SAY_JAYNA_OUTRO_10                          = 2,

    // Sylvanas
    SAY_SYLVANAS_OUTRO_2                        = 0,
    SAY_SYLVANAS_OUTRO_4                        = 1,
    SAY_SYLVANAS_OUTRO_10                       = 2,

    // Tyrannus
    SAY_TYRANNUS_OUTRO_7                        = 1,
    SAY_TYRANNUS_OUTRO_9                        = 2,
};

enum Events
{
    EVENT_MIGHTY_KICK                           = 1,
    EVENT_SHADOW_BOLT                           = 2,
    EVENT_TOXIC_WASTE                           = 3,
    EVENT_SPECIAL                               = 4, // special spell selection (one of event 5, 6 or 7)
    EVENT_PURSUIT                               = 5,
    EVENT_POISON_NOVA                           = 6,
    EVENT_EXPLOSIVE_BARRAGE                     = 7,

    // Krick OUTRO
    EVENT_OUTRO_1                               = 8,
    EVENT_OUTRO_2                               = 9,
    EVENT_OUTRO_3                               = 10,
    EVENT_OUTRO_4                               = 11,
    EVENT_OUTRO_5                               = 12,
    EVENT_OUTRO_6                               = 13,
    EVENT_OUTRO_7                               = 14,
    EVENT_OUTRO_8                               = 15,
    EVENT_OUTRO_9                               = 16,
    EVENT_OUTRO_10                              = 17,
    EVENT_OUTRO_11                              = 18,
    EVENT_OUTRO_12                              = 19,
    EVENT_OUTRO_13                              = 20,
    EVENT_OUTRO_END                             = 21,
};

enum KrickPhase
{
    PHASE_COMBAT                                = 1,
    PHASE_OUTRO                                 = 2,
};

enum Actions
{
    ACTION_OUTRO                                = 1,
};

enum Points
{
    POINT_KRICK_INTRO                           = 364770,
    POINT_KRICK_DEATH                           = 364771,
};

enum Misc
{
    SEAT_KRICK                                  = 0,
};

static const Position outroPos[8] =
{
    { 828.9342f, 118.6247f, 509.5190f, 0.0000000f },  // Krick's Outro Position
    { 841.0100f, 196.2450f, 573.9640f, 0.2046099f },  // Scourgelord Tyrannus Outro Position (Tele to...)
    { 777.2274f, 119.5521f, 510.0363f, 6.0562930f },  // Sylvanas / Jaine Outro Spawn Position (NPC_SYLVANAS_PART1)
    { 823.3984f, 114.4907f, 509.4899f, 0.0000000f },  // Sylvanas / Jaine Outro Move Position (1)
    { 835.5887f, 139.4345f, 530.9526f, 0.0000000f },  // Tyrannus fly down Position (not sniffed)
    { 828.9342f, 118.6247f, 514.5190f, 0.0000000f },  // Krick's Choke Position
    { 828.9342f, 118.6247f, 509.4958f, 0.0000000f },  // Kirck's Death Position
    { 914.4820f, 143.1602f, 633.3624f, 0.0000000f },  // Tyrannus fly up (not sniffed)
};

class boss_ick : public CreatureScript
{
    public:
        boss_ick() : CreatureScript("boss_ick") { }

        struct boss_ickAI : public BossAI
        {
            boss_ickAI(Creature* creature) : BossAI(creature, DATA_ICK), _vehicle(creature->GetVehicleKit())
            {
                ASSERT(_vehicle);
            }

            void InitializeAI() override
            {
                if (!instance || static_cast<InstanceMap*>(me->GetMap())->GetScriptId() != sObjectMgr->GetScriptId(PoSScriptName))
                    me->IsAIEnabled = false;
                else if (!me->isDead())
                    Reset();
            }

            void Reset() override
            {
                events.Reset();
                instance->SetBossState(DATA_ICK, NOT_STARTED);
            }

            Creature* GetKrick()
            {
                return ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_KRICK));
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                if (Creature* krick = GetKrick())
                    krick->AI()->Talk(SAY_KRICK_AGGRO);

                events.ScheduleEvent(EVENT_MIGHTY_KICK, 20000);
                events.ScheduleEvent(EVENT_TOXIC_WASTE, 5000);
                events.ScheduleEvent(EVENT_SHADOW_BOLT, 10000);
                events.ScheduleEvent(EVENT_SPECIAL, urand(30000, 35000));

                instance->SetBossState(DATA_ICK, IN_PROGRESS);
            }

            void EnterEvadeMode() override
            {
                me->GetMotionMaster()->Clear();
                ScriptedAI::EnterEvadeMode();
            }

            void JustDied(Unit* /*killer*/) override
            {
                if (Creature* krick = GetKrick())
                {
                    _vehicle->RemoveAllPassengers();
                    if (krick->AI())
                        krick->AI()->DoAction(ACTION_OUTRO);
                }

                instance->SetBossState(DATA_ICK, DONE);
            }

            void SetTempThreat(float threat)
            {
                _tempThreat = threat;
            }

            void _ResetThreat(Unit* target)
            {
                DoModifyThreatPercent(target, -100);
                me->AddThreat(target, _tempThreat);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!me->IsInCombat())
                    return;

                if (!me->GetVictim() && me->GetThreatManager().isThreatListEmpty())
                {
                    EnterEvadeMode();
                    return;
                }

                UpdateVictim();

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_TOXIC_WASTE:
                            if (Creature* krick = GetKrick())
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                    krick->CastSpell(target, SPELL_TOXIC_WASTE, TRIGGERED_IGNORE_CASTER_MOUNTED_OR_ON_VEHICLE);
                            events.ScheduleEvent(EVENT_TOXIC_WASTE, urand(7000, 10000));
                            break;
                        case EVENT_SHADOW_BOLT:
                            if (Creature* krick = GetKrick())
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1))
                                    krick->CastSpell(target, SPELL_SHADOW_BOLT, TRIGGERED_IGNORE_CASTER_MOUNTED_OR_ON_VEHICLE);
                            events.ScheduleEvent(EVENT_SHADOW_BOLT, 15000);
                            return;
                        case EVENT_MIGHTY_KICK:
                            DoCastVictim(SPELL_MIGHTY_KICK);
                            events.ScheduleEvent(EVENT_MIGHTY_KICK, 25000);
                            return;
                        case EVENT_SPECIAL:
                            //select one of these three special events
                            events.ScheduleEvent(RAND(EVENT_EXPLOSIVE_BARRAGE, EVENT_POISON_NOVA, EVENT_PURSUIT), 1000);
                            events.ScheduleEvent(EVENT_SPECIAL, urand(23000, 28000));
                            break;
                        case EVENT_EXPLOSIVE_BARRAGE:
                            if (Creature* krick = GetKrick())
                            {
                                krick->AI()->Talk(SAY_KRICK_BARRAGE_1);
                                krick->AI()->Talk(SAY_KRICK_BARRAGE_2);
                                krick->CastSpell(krick, SPELL_EXPLOSIVE_BARRAGE_KRICK, true);
                                DoCast(me, SPELL_EXPLOSIVE_BARRAGE_ICK);
                            }
                            events.DelayEvents(20000);
                            break;
                        case EVENT_POISON_NOVA:
                            if (Creature* krick = GetKrick())
                                krick->AI()->Talk(SAY_KRICK_POISON_NOVA, krick);

                            Talk(SAY_ICK_POISON_NOVA);
                            DoCast(me, SPELL_POISON_NOVA);
                            break;
                        case EVENT_PURSUIT:
                            if (Creature* krick = GetKrick())
                                krick->AI()->Talk(SAY_KRICK_CHASE);
                            DoCast(me, SPELL_PURSUIT);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            Vehicle* _vehicle;
            float _tempThreat;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_ickAI(creature);
        }
};

class boss_krick : public CreatureScript
{
    public:
        boss_krick() : CreatureScript("boss_krick") { }

        struct boss_krickAI : public ScriptedAI
        {
            boss_krickAI(Creature* creature) : ScriptedAI(creature), _instanceScript(creature->GetInstanceScript()), _summons(creature) { }

            void InitializeAI() override
            {
                if (!_instanceScript || static_cast<InstanceMap*>(me->GetMap())->GetScriptId() != sObjectMgr->GetScriptId(PoSScriptName))
                    me->IsAIEnabled = false;
                else if (!me->isDead())
                    Reset();
            }

            void Reset() override
            {
                _events.Reset();
                _phase = PHASE_COMBAT;
                _outroNpcGUID = ObjectGuid::Empty;
                _tyrannusGUID = ObjectGuid::Empty;

                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }

            Creature* GetIck()
            {
                return ObjectAccessor::GetCreature(*me, _instanceScript->GetGuidData(DATA_ICK));
            }

            void KilledUnit(Unit* victim) override
            {
                if (victim->GetTypeId() != TYPEID_PLAYER)
                    return;

                Talk(SAY_KRICK_SLAY);
            }

            void JustSummoned(Creature* summon) override
            {
                _summons.Summon(summon);
                if (summon->GetEntry() == NPC_EXPLODING_ORB)
                {
                    summon->CastSpell(summon, SPELL_EXPLODING_ORB, true);
                    summon->CastSpell(summon, SPELL_AUTO_GROW, true);
                }
            }

            void DoAction(int32 action) override
            {
                if (action == ACTION_OUTRO)
                {
                    Creature* tyrannusPtr = ObjectAccessor::GetCreature(*me, _instanceScript->GetGuidData(DATA_TYRANNUS_EVENT));
                    if (tyrannusPtr)
                        tyrannusPtr->NearTeleportTo(outroPos[1].GetPositionX(), outroPos[1].GetPositionY(), outroPos[1].GetPositionZ(), outroPos[1].GetOrientation());
                    else
                        tyrannusPtr = me->SummonCreature(NPC_TYRANNUS_EVENTS, outroPos[1], TEMPSUMMON_MANUAL_DESPAWN);

                    tyrannusPtr->SetFlying(true);
                    me->GetMotionMaster()->MovePoint(POINT_KRICK_INTRO, outroPos[0].GetPositionX(), outroPos[0].GetPositionY(), outroPos[0].GetPositionZ());
                    tyrannusPtr->SetFacingToObject(me);
                }
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (type != POINT_MOTION_TYPE || pointId != POINT_KRICK_INTRO)
                    return;

                Talk(SAY_KRICK_OUTRO_1);
                _phase = PHASE_OUTRO;
                _events.Reset();
                _events.ScheduleEvent(EVENT_OUTRO_1, 1000);
            }

            void UpdateAI(uint32 diff) override
            {
                if (_phase != PHASE_OUTRO)
                    return;

                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_OUTRO_1:
                        {
                            if (Creature* temp = me->GetCreature(*me, _instanceScript->GetGuidData(DATA_JAINA_SYLVANAS_1)))
                                temp->DespawnOrUnsummon();

                            Creature* jainaOrSylvanas = NULL;
                            if (_instanceScript->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                                jainaOrSylvanas = me->SummonCreature(NPC_JAINA_PART1, outroPos[2], TEMPSUMMON_MANUAL_DESPAWN);
                            else
                                jainaOrSylvanas = me->SummonCreature(NPC_SYLVANAS_PART1, outroPos[2], TEMPSUMMON_MANUAL_DESPAWN);

                            if (jainaOrSylvanas)
                            {
                                jainaOrSylvanas->GetMotionMaster()->MovePoint(0, outroPos[3]);
                                _outroNpcGUID = jainaOrSylvanas->GetGUID();
                            }
                            _events.ScheduleEvent(EVENT_OUTRO_2, 6000);
                            break;
                        }
                        case EVENT_OUTRO_2:
                            if (Creature* jainaOrSylvanas = ObjectAccessor::GetCreature(*me, _outroNpcGUID))
                            {
                                jainaOrSylvanas->SetFacingToObject(me);
                                me->SetFacingToObject(jainaOrSylvanas);
                                if (_instanceScript->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                                    jainaOrSylvanas->AI()->Talk(SAY_JAYNA_OUTRO_2);
                                else
                                    jainaOrSylvanas->AI()->Talk(SAY_SYLVANAS_OUTRO_2);
                            }
                            _events.ScheduleEvent(EVENT_OUTRO_3, 5000);
                            break;
                        case EVENT_OUTRO_3:
                            Talk(SAY_KRICK_OUTRO_3);
                            _events.ScheduleEvent(EVENT_OUTRO_4, 18000);
                            break;
                        case EVENT_OUTRO_4:
                            if (Creature* jainaOrSylvanas = ObjectAccessor::GetCreature(*me, _outroNpcGUID))
                            {
                                if (_instanceScript->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                                    jainaOrSylvanas->AI()->Talk(SAY_JAYNA_OUTRO_4);
                                else
                                    jainaOrSylvanas->AI()->Talk(SAY_SYLVANAS_OUTRO_4);
                            }
                            _events.ScheduleEvent(EVENT_OUTRO_5, 5000);
                            break;
                        case EVENT_OUTRO_5:
                            Talk(SAY_KRICK_OUTRO_5);
                            _events.ScheduleEvent(EVENT_OUTRO_6, 1000);
                            break;
                        case EVENT_OUTRO_6:
                            if (Creature* tyrannus = me->GetCreature(*me, _instanceScript->GetGuidData(DATA_TYRANNUS_EVENT)))
                            {
                                tyrannus->SetSpeed(MOVE_RUN, 3.5f, true);
                                tyrannus->GetMotionMaster()->MovePoint(1, outroPos[4]);
                                _tyrannusGUID = tyrannus->GetGUID();
                            }
                            _events.ScheduleEvent(EVENT_OUTRO_7, 5000);
                            break;
                        case EVENT_OUTRO_7:
                            if (Creature* tyrannus = ObjectAccessor::GetCreature(*me, _tyrannusGUID))
                                tyrannus->AI()->Talk(SAY_TYRANNUS_OUTRO_7);
                            _events.ScheduleEvent(EVENT_OUTRO_8, 5000);
                            break;
                        case EVENT_OUTRO_8:
                            me->SetFlying(true);
                            me->GetMotionMaster()->MoveTakeoff(0, outroPos[5]);
                            DoCast(me, SPELL_STRANGULATING);
                            _events.ScheduleEvent(EVENT_OUTRO_9, 2000);
                            break;
                        case EVENT_OUTRO_9:
                            Talk(SAY_KRICK_OUTRO_8);
                            // TODO: Tyrannus starts killing Krick.
                            // there shall be some visual spell effect
                            if (Creature* tyrannus = ObjectAccessor::GetCreature(*me, _tyrannusGUID))
                                tyrannus->CastSpell(me, SPELL_NECROMANTIC_POWER, true);  // not sure if it's the right spell :/
                            _events.ScheduleEvent(EVENT_OUTRO_10, 1000);
                            break;
                        case EVENT_OUTRO_10:
                            me->SetFlying(false);
                            me->GetMotionMaster()->MoveFall(0);
                            _events.ScheduleEvent(EVENT_OUTRO_11, 2000);
                            break;
                        case EVENT_OUTRO_11:
                            DoCast(me, SPELL_KRICK_KILL_CREDIT); // don't really know if we need it
                            me->SetStandState(UNIT_STAND_STATE_DEAD);
                            me->SetHealth(0);
                            _events.ScheduleEvent(EVENT_OUTRO_12, 3000);
                            break;
                        case EVENT_OUTRO_12:
                            if (Creature* tyrannus = ObjectAccessor::GetCreature(*me, _tyrannusGUID))
                                tyrannus->AI()->Talk(SAY_TYRANNUS_OUTRO_9);
                            _events.ScheduleEvent(EVENT_OUTRO_13, 2000);
                            break;
                        case EVENT_OUTRO_13:
                            if (Creature* jainaOrSylvanas = ObjectAccessor::GetCreature(*me, _outroNpcGUID))
                            {
                                if (_instanceScript->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                                    jainaOrSylvanas->AI()->Talk(SAY_JAYNA_OUTRO_10);
                                else
                                    jainaOrSylvanas->AI()->Talk(SAY_SYLVANAS_OUTRO_10);
                            }
                            // End of OUTRO. for now...
                            _events.ScheduleEvent(EVENT_OUTRO_END, 3000);
                            if (Creature* tyrannus = ObjectAccessor::GetCreature(*me, _tyrannusGUID))
                                tyrannus->GetMotionMaster()->MovePoint(0, outroPos[7]);
                            break;
                        case EVENT_OUTRO_END:
                            if (Creature* tyrannus = ObjectAccessor::GetCreature(*me, _tyrannusGUID))
                                tyrannus->DespawnOrUnsummon()/*DisappearAndDie()*/;

                            me->DisappearAndDie();
                            break;
                        default:
                            break;
                    }
                }
            }

        private:
            InstanceScript* _instanceScript;
            SummonList _summons;
            EventMap _events;

            KrickPhase _phase;
            ObjectGuid _outroNpcGUID;
            ObjectGuid _tyrannusGUID;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_krickAI(creature);
        }
};

class spell_krick_explosive_barrage : public SpellScriptLoader
{
    public:
        spell_krick_explosive_barrage() : SpellScriptLoader("spell_krick_explosive_barrage") { }

        class spell_krick_explosive_barrage_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_krick_explosive_barrage_AuraScript);

            void HandlePeriodicTick(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();
                if (Unit* caster = GetCaster())
                    if (caster->GetTypeId() == TYPEID_UNIT)
                    {
                        for (auto&& ref : caster->GetMap()->GetPlayers())
                            if (Player* player = ref.GetSource())
                                if (player->IsWithinDist(caster, 60.0f)) // don't know correct range
                                    caster->CastSpell(player, SPELL_EXPLOSIVE_BARRAGE_SUMMON, true);
                    }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_krick_explosive_barrage_AuraScript::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_krick_explosive_barrage_AuraScript();
        }
};

class spell_ick_explosive_barrage : public SpellScriptLoader
{
    public:
        spell_ick_explosive_barrage() : SpellScriptLoader("spell_ick_explosive_barrage") { }

        class spell_ick_explosive_barrage_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_ick_explosive_barrage_AuraScript);

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (caster->GetTypeId() == TYPEID_UNIT)
                        caster->GetMotionMaster()->MoveIdle();
            }

            void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (caster->GetTypeId() == TYPEID_UNIT)
                    {
                        caster->GetMotionMaster()->Clear();
                        caster->GetMotionMaster()->MoveChase(caster->GetVictim());
                    }
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_ick_explosive_barrage_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_ick_explosive_barrage_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_ick_explosive_barrage_AuraScript();
        }
};

class spell_exploding_orb_hasty_grow : public SpellScriptLoader
{
    public:
        spell_exploding_orb_hasty_grow() : SpellScriptLoader("spell_exploding_orb_hasty_grow") { }

        class spell_exploding_orb_hasty_grow_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_exploding_orb_hasty_grow_AuraScript);

            void OnStackChange(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetStackAmount() == 15)
                {
                    Unit* target = GetTarget(); // store target because aura gets removed
                    //PreventDefaultAction();
                    target->CastSpell(target, SPELL_EXPLOSIVE_BARRAGE_DAMAGE, false);
                    target->RemoveAurasDueToSpell(SPELL_HASTY_GROW);
                    target->RemoveAurasDueToSpell(SPELL_AUTO_GROW);
                    target->RemoveAurasDueToSpell(SPELL_EXPLODING_ORB);
                    if (Creature* creature = target->ToCreature())
                        creature->DespawnOrUnsummon();
                }
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_exploding_orb_hasty_grow_AuraScript::OnStackChange, EFFECT_0, SPELL_AURA_MOD_SCALE, AURA_EFFECT_HANDLE_REAPPLY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_exploding_orb_hasty_grow_AuraScript();
        }
};

class spell_krick_pursuit : public SpellScriptLoader
{
    public:
        spell_krick_pursuit() : SpellScriptLoader("spell_krick_pursuit") { }

        class spell_krick_pursuit_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_krick_pursuit_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                Trinity::Containers::RandomResizeList(targets, 1);
                if (!targets.empty())
                    m_target = targets.front();
            }

            void SelectTarget(std::list<WorldObject*>& targets)
            {
                targets.clear();
                if (m_target)
                    targets.push_back(m_target);
            }

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                if (GetCaster()->GetTypeId() != TYPEID_UNIT)
                    return;

                Unit* caster = GetCaster();
                CreatureAI* ickAI = caster->ToCreature()->AI();
                if (Unit* target = GetHitUnit())
                {
                    ickAI->Talk(SAY_ICK_CHASE_1, target);
                    CAST_AI(boss_ick::boss_ickAI, ickAI)->SetTempThreat(caster->GetThreatManager().getThreat(target));
                    caster->AddThreat(target, float(GetEffectValue()));
                    target->AddThreat(caster, float(GetEffectValue()));
                }
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_krick_pursuit_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_krick_pursuit_SpellScript::SelectTarget, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_krick_pursuit_SpellScript::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }

        private:
            WorldObject* m_target = NULL;
        };

        class spell_krick_pursuit_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_krick_pursuit_AuraScript);

            void HandleExtraEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (Creature* creCaster = caster->ToCreature())
                        CAST_AI(boss_ick::boss_ickAI, creCaster->AI())->_ResetThreat(GetTarget());
            }

            void Register() override
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_krick_pursuit_AuraScript::HandleExtraEffect, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_krick_pursuit_SpellScript();
        }

        AuraScript* GetAuraScript() const override
        {
            return new spell_krick_pursuit_AuraScript();
        }
};

class spell_krick_pursuit_confusion : public SpellScriptLoader
{
    public:
        spell_krick_pursuit_confusion() : SpellScriptLoader("spell_krick_pursuit_confusion") { }

        class spell_krick_pursuit_confusion_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_krick_pursuit_confusion_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                GetTarget()->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                GetTarget()->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_krick_pursuit_confusion_AuraScript::OnApply, EFFECT_2, SPELL_AURA_LINKED, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_krick_pursuit_confusion_AuraScript::OnRemove, EFFECT_2, SPELL_AURA_LINKED, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_krick_pursuit_confusion_AuraScript();
        }
};

void AddSC_boss_ick()
{
    new boss_ick();
    new boss_krick();
    new spell_krick_explosive_barrage();
    new spell_ick_explosive_barrage();
    new spell_exploding_orb_hasty_grow();
    new spell_krick_pursuit();
    new spell_krick_pursuit_confusion();
}
