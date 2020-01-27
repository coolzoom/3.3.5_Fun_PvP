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

/*
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "npc_pet_pri_".
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "PassiveAI.h"
#include "PetAI.h"

enum Priest_spells
{
    MANA_LEECH                       = 28305,
    GLYPH_OF_SHADOWFIEND_MANA        = 58227,
    GLYPH_OF_SHADOWFIEND             = 58228,
    SPELL_PRIEST_LIGHTWELL_CHARGES   = 59907
};

class npc_pet_pri_lightwell : public CreatureScript
{
    public:
        npc_pet_pri_lightwell() : CreatureScript("npc_pet_pri_lightwell") { }

        struct npc_pet_pri_lightwellAI : public PassiveAI
        {
            npc_pet_pri_lightwellAI(Creature* creature) : PassiveAI(creature)
            {
                DoCast(me, SPELL_PRIEST_LIGHTWELL_CHARGES, false);
            }

            void EnterEvadeMode(EvadeReason /*why*/) override
            {
                if (!me->IsAlive())
                    return;

                me->DeleteThreatList();
                me->CombatStop(true);
                me->ResetPlayerDamageReq();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_pet_pri_lightwellAI(creature);
        }
};

class npc_pet_pri_shadowfiend : public CreatureScript
{
    public:
        npc_pet_pri_shadowfiend() : CreatureScript("npc_pet_pri_shadowfiend") { }

        struct npc_pet_pri_shadowfiendAI : public PetAI
        {
            npc_pet_pri_shadowfiendAI(Creature* creature) : PetAI(creature) { }

            void Reset() override
            {
                    // Find victim of Shadowfiend
                    std::list<Unit*> targets;
                    Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(me, me, 30.0f);
                    Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(me, targets, u_check);
                    me->VisitNearbyObject(30.0f, searcher);
                    for (std::list<Unit*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
                        {
                            if ((*iter)->IsImmunedToDamage(SPELL_SCHOOL_MASK_NORMAL))
                                continue;
                            if (!me->IsValidAttackTarget((*iter)))
                                continue;
                            if (!(*iter)->HasBreakableByDamageCrowdControlAura())
                                me->Attack((*iter), false);
                            break;
                        }
                        
                if (me->IsSummon())
                    if (Unit* owner = me->ToTempSummon()->GetSummoner())
                        if (Unit* pet = owner->GetGuardianPet())
                            pet->CastSpell(pet, MANA_LEECH, true);
            }
            
            void DamageTaken(Unit* /*killer*/, uint32& damage) override
            {
                if (me->IsSummon())
                    if (Unit* owner = me->ToTempSummon()->GetSummoner())
                        if (owner->HasAura(GLYPH_OF_SHADOWFIEND) && damage >= me->GetHealth())
                            owner->CastSpell(owner, GLYPH_OF_SHADOWFIEND_MANA, true);
            }
            
            void UpdateAI(uint32 /*diff*/) override
            {
                if (Unit* owner = me->GetOwner())
                    if (me->GetVictim() && owner->GetTypeId() == TYPEID_PLAYER && !owner->CanSeeOrDetect(me->GetVictim(), false, true))
                    {
                        me->AI()->EnterEvadeMode(CreatureAI::EVADE_REASON_OTHER);
                        me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle());
                    }
                if (!UpdateVictim())
                    return;
                    
                if (me->HasUnitState(UNIT_STATE_FLEEING) || me->HasUnitState(UNIT_STATE_CONFUSED))
                {
                    me->AttackStop();
                    return;
                }
                
                if (me->GetVictim())
                {
                    if (me->GetVictim()->HasBreakableByDamageCrowdControlAura())
                    {
                        me->AttackStop();
                        return;
                    }

                    if (!me->IsValidAttackTarget(me->GetVictim()))
                    {
                        me->AttackStop();
                        return;
                    }
                        
                    if (me->GetVictim()->IsImmunedToDamage(SPELL_SCHOOL_MASK_NORMAL))
                    {
                        me->AttackStop();
                        return;
                    }
                        
                     DoMeleeAttackIfReady();
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_pet_pri_shadowfiendAI(creature);
        }
};

void AddSC_priest_pet_scripts()
{
    new npc_pet_pri_lightwell();
    new npc_pet_pri_shadowfiend();
}
