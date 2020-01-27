/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "zulgurub.h"

enum Says
{
    SAY_AGGRO                   = 0,
    SAY_RAIN_FIRE               = 1,
    SAY_DEATH                   = 2
};

enum Spells
{
    SPELL_CHARGE                = 22911,
    SPELL_SONICBURST            = 23918,
    SPELL_SCREECH               = 6605,
    SPELL_SHADOW_WORD_PAIN      = 23952,
    SPELL_MIND_FLAY             = 23953,
    SPELL_CHAIN_MIND_FLAY       = 26044, // Right ID unknown. So disabled
    SPELL_GREATERHEAL           = 23954,
    SPELL_BAT_FORM              = 23966,

    // Batriders Spell
    SPELL_BOMB                  = 40332 // Wrong ID but Magmadars bomb is not working...
};

enum BatIds
{
    NPC_BLOODSEEKER_BAT         = 11368,
    NPC_FRENZIED_BAT            = 14965
};

enum Events
{
    EVENT_CHARGE_JEKLIK         = 1,
    EVENT_SONIC_BURST,
    EVENT_SCREECH,
    EVENT_SPAWN_BATS,
    EVENT_SHADOW_WORD_PAIN,
    EVENT_MIND_FLAY,
    EVENT_CHAIN_MIND_FLAY,
    EVENT_GREATER_HEAL,
    EVENT_SPAWN_FLYING_BATS,
    EVENT_SPAWN_FLYING_BATS_LOW
};

enum Phase
{
    PHASE_ONE                   = 1,
    PHASE_TWO                   = 2
};

Position const SpawnBat[8] =
{
    { -12290.1600f, -1381.2400f, 145.0510f, 5.400f },
    { -12275.6220f, -1422.2640f, 136.8304f, 5.483f },
    { -12276.6220f, -1423.2640f, 137.8304f, 5.483f },
    { -12278.6220f, -1424.2640f, 139.8304f, 5.483f },
    { -12279.6220f, -1425.2640f, 138.8304f, 5.483f },
    { -12281.6220f, -1426.2640f, 136.8304f, 5.483f },
	{ -12292.2800f, -1381.4900f, 145.0510f, 5.391f },
	{ -12264.1900f, -1434.5700f, 135.8510f, 3.143f }
};

class boss_jeklik : public CreatureScript
{
    public: boss_jeklik() : CreatureScript("boss_jeklik") { }

        struct boss_jeklikAI : public BossAI
        {
            boss_jeklikAI(Creature* creature) : BossAI(creature, DATA_JEKLIK) { }

            void Reset() override
            {
                _Reset();
				me->GetMotionMaster()->MovePoint(1, SpawnBat[0]);
                me->UpdateOrientation(5.400f);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();
                Talk(SAY_DEATH);
            }

            void EnterCombat(Unit* /*who*/) override
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                events.SetPhase(PHASE_ONE);

                events.ScheduleEvent(EVENT_CHARGE_JEKLIK, 20000, 0, PHASE_ONE);
                events.ScheduleEvent(EVENT_SONIC_BURST, 8000, 0, PHASE_ONE);
                events.ScheduleEvent(EVENT_SCREECH, 13000, 0, PHASE_ONE);
                events.ScheduleEvent(EVENT_SPAWN_BATS, 60000, 0, PHASE_ONE);

                me->SetCanFly(true);
                me->SetDisableGravity(true);
                DoCast(me, SPELL_BAT_FORM);
                DoZoneInCombat();
            }

            void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/) override
            {
                if (events.IsInPhase(PHASE_ONE) && !HealthAbovePct(50))
                {
                    me->RemoveAurasDueToSpell(SPELL_BAT_FORM);
                    me->SetCanFly(false);
                    DoResetThreat();
                    events.SetPhase(PHASE_TWO);
                    events.ScheduleEvent(EVENT_SHADOW_WORD_PAIN, 6000, 0, PHASE_TWO);
                    events.ScheduleEvent(EVENT_MIND_FLAY, 11000, 0, PHASE_TWO);
                    events.ScheduleEvent(EVENT_CHAIN_MIND_FLAY, 20000, 0, PHASE_TWO);
                    events.ScheduleEvent(EVENT_GREATER_HEAL, 50000, 0, PHASE_TWO);
                    events.ScheduleEvent(EVENT_SPAWN_FLYING_BATS, 45000, 0, PHASE_TWO);
                    return;
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHARGE_JEKLIK:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            {
                                DoCast(target, SPELL_CHARGE);
                                AttackStart(target);
                            }
                            events.ScheduleEvent(EVENT_CHARGE_JEKLIK, urand(15000, 30000), 0, PHASE_ONE);
                            break;
                        case EVENT_SONIC_BURST:
                            DoCastVictim(SPELL_SONICBURST);
                            events.ScheduleEvent(EVENT_SONIC_BURST, urand(8000, 13000), 0, PHASE_ONE);
                            break;
                        case EVENT_SCREECH:
                            DoCastVictim(SPELL_SCREECH);
                            events.ScheduleEvent(EVENT_SCREECH, urand(12000, 24000), 0, PHASE_ONE);
                            break;
                        case EVENT_SPAWN_BATS:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                for (uint8 i = 0; i < 5; ++i)
                                    if (Creature* bat = me->SummonCreature(NPC_BLOODSEEKER_BAT, SpawnBat[i], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000))
                                        bat->AI()->AttackStart(target);
                            events.ScheduleEvent(EVENT_SPAWN_BATS, 60000, 0, PHASE_ONE);
                            break;
                        case EVENT_SHADOW_WORD_PAIN:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                DoCast(target, SPELL_SHADOW_WORD_PAIN);
                            events.ScheduleEvent(EVENT_SHADOW_WORD_PAIN, urand(9000, 12000), 0, PHASE_TWO);
                            break;
                        case EVENT_MIND_FLAY:
                            DoCastVictim(SPELL_MIND_FLAY);
                            events.ScheduleEvent(EVENT_MIND_FLAY, 16000, 0, PHASE_TWO);
                            break;
                        case EVENT_CHAIN_MIND_FLAY:
                            me->InterruptNonMeleeSpells(false);
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                DoCast(target, SPELL_CHAIN_MIND_FLAY);
                            events.ScheduleEvent(EVENT_CHAIN_MIND_FLAY, urand(15000, 30000), 0, PHASE_TWO);
                            break;
                        case EVENT_GREATER_HEAL:
                            me->InterruptNonMeleeSpells(false);
                            DoCast(me, SPELL_GREATERHEAL);
                            events.ScheduleEvent(EVENT_GREATER_HEAL, urand(25000, 35000), 0, PHASE_TWO);
                            break;
                        case EVENT_SPAWN_FLYING_BATS:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                if (Creature* flyingBat = me->SummonCreature(NPC_FRENZIED_BAT, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ() + 15.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000))
                                    flyingBat->AI()->AttackStart(target);
                            events.ScheduleEvent(EVENT_SPAWN_FLYING_BATS, urand(15000, 20000), 0, PHASE_TWO);
                            break;
                        case EVENT_SPAWN_FLYING_BATS_LOW:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                if (Creature* flyingBat = me->SummonCreature(NPC_FRENZIED_BAT, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ() + 15.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000))
                                    flyingBat->AI()->AttackStart(target);
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                if (Creature* flyingBat = me->SummonCreature(NPC_FRENZIED_BAT, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ() + 15.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000))
                                    flyingBat->AI()->AttackStart(target);
                            events.ScheduleEvent(EVENT_SPAWN_FLYING_BATS_LOW, urand(10000, 15000), 0, PHASE_TWO);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetZulGurubAI<boss_jeklikAI>(creature);
        }
};

// Flying Bat
class npc_batrider : public CreatureScript
{
    public:
        npc_batrider() : CreatureScript("npc_batrider") { }

        struct npc_batriderAI : public ScriptedAI
        {
            npc_batriderAI(Creature* creature) : ScriptedAI(creature) { }

            uint32 Bomb_Timer;

            void Reset() override
            {
                Bomb_Timer = 2000;
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    AttackStart(target);

            }

            void EnterCombat(Unit* /*who*/) override { }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;
                    
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    AttackStart(target);

                if (Bomb_Timer <= diff)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    {
                        DoCast(target, SPELL_BOMB);
                        Bomb_Timer = 5000;
                    }
                }
                else
                    Bomb_Timer -= diff;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_batriderAI(creature);
        }
};

void AddSC_boss_jeklik()
{
    new boss_jeklik();
    new npc_batrider();
}
