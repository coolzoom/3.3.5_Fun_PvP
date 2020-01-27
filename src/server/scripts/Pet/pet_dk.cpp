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
 * Scriptnames of files in this file should be prefixed with "npc_pet_dk_".
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "CombatAI.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

enum DeathKnightSpells
{
    SPELL_DK_SUMMON_GARGOYLE_1          = 49206,
    SPELL_DK_SUMMON_GARGOYLE_2          = 50514,
    SPELL_DK_DISMISS_GARGOYLE           = 50515,
    SPELL_DK_SANCTUARY                  = 54661
};

/*class npc_pet_dk_ebon_gargoyle : public CreatureScript
{
    public:
        npc_pet_dk_ebon_gargoyle() : CreatureScript("npc_pet_dk_ebon_gargoyle") { }

        struct npc_pet_dk_ebon_gargoyleAI : CasterAI
        {
            npc_pet_dk_ebon_gargoyleAI(Creature* creature) : CasterAI(creature)
            {
                Initialize();
            }

            void Initialize()
            {
                // Not needed to be despawned now
                _despawnTimer = 0;
            }

            void InitializeAI() override
            {
                Initialize();

                CasterAI::InitializeAI();
                ObjectGuid ownerGuid = me->GetOwnerGUID();
                if (!ownerGuid)
                    return;

                // Find victim of Summon Gargoyle spell
                std::list<Unit*> targets;
                Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(me, me, 45.0f);
                Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(me, targets, u_check);
                me->VisitNearbyObject(45.0f, searcher);
                for (std::list<Unit*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
                    if ((*iter)->GetAura(SPELL_DK_SUMMON_GARGOYLE_1, ownerGuid))
                    {
                        if (!(*iter)->HasBreakableByDamageCrowdControlAura())
                            if (GetOwner() && me->GetVictim() && GetOwner()->GetTypeId() == TYPEID_PLAYER && !GetOwner()->CanSeeOrDetect(me->GetVictim(), false, true))
                                me->Attack((*iter), false);
                        break;
                    }
            }

            void JustDied(Unit* killer) override
            {
                // Stop Feeding Gargoyle when it dies
                if (Unit* owner = me->GetOwner())
                    owner->RemoveAurasDueToSpell(SPELL_DK_SUMMON_GARGOYLE_2);
            }

            // Fly away when dismissed
            void SpellHit(Unit* source, SpellInfo const* spell) override
            {
                if (spell->Id != SPELL_DK_DISMISS_GARGOYLE || !me->IsAlive())
                    return;

                Unit* owner = me->GetOwner();
                if (!owner || owner != source)
                    return;

                // Stop Fighting
                me->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE, true);

                // Sanctuary
                me->CastSpell(me, SPELL_DK_SANCTUARY, true);
                me->SetReactState(REACT_PASSIVE);
                me->CastStop();

                //! HACK: Creature's can't have MOVEMENTFLAG_FLYING
                // Fly Away
                me->SetCanFly(true);

                me->SetSpeedRate(MOVE_FLIGHT, 0.75f);
                me->SetSpeedRate(MOVE_RUN, 0.75f);

                float x = me->GetPositionX() + 20 * std::cos(me->GetOrientation());
                float y = me->GetPositionY() + 20 * std::sin(me->GetOrientation());
                float z = me->GetPositionZ() + 50;
                me->GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MovePoint(0, x, y, z);

                // Despawn as soon as possible
                _despawnTimer = 4 * IN_MILLISECONDS;
            }

            void UpdateAI(uint32 diff) override
            {
                if (Unit* owner = me->GetOwner())
                    if (me->GetVictim() && owner->GetTypeId() == TYPEID_PLAYER && !owner->CanSeeOrDetect(me->GetVictim(), false, true))
                    {
                        me->AI()->EnterEvadeMode(CreatureAI::EVADE_REASON_OTHER);
                        me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle());
                    }
                    
                // if evade mode finding for next target && GARGOYLE CANT ATTACK ANOTHER TARGET THAN TAGGED (ON SUMMON)
                if (me->IsInEvadeMode())
                {
                    ObjectGuid ownerGuid = me->GetOwnerGUID();
                    if (!ownerGuid)
                        return;

                    std::list<Unit*> targets;
                    Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(me, me, 45.0f);
                    Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(me, targets, u_check);
                    me->VisitNearbyObject(45.0f, searcher);
                    for (std::list<Unit*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
                        if ((*iter)->GetAura(SPELL_DK_SUMMON_GARGOYLE_1, ownerGuid))
                        {
                            if (!(*iter)->HasBreakableByDamageCrowdControlAura())
                                if (me->GetVictim() && owner->GetTypeId() == TYPEID_PLAYER && !owner->CanSeeOrDetect(me->GetVictim(), false, true))
                                    me->Attack((*iter), false);
                            break;
                        }
                }

                if (_despawnTimer > 0)
                {
                    if (_despawnTimer > diff)
                        _despawnTimer -= diff;
                    else
                        me->DespawnOrUnsummon();
                    return;
                }

                CasterAI::UpdateAI(diff);
            }

        private:
           uint32 _despawnTimer;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_pet_dk_ebon_gargoyleAI(creature);
        }
};*/

class npc_pet_dk_ebon_gargoyle : public CreatureScript
{
    public:
        npc_pet_dk_ebon_gargoyle() : CreatureScript("npc_pet_dk_ebon_gargoyle") { }

        struct npc_pet_dk_ebon_gargoyleAI : CasterAI
        {
            npc_pet_dk_ebon_gargoyleAI(Creature* creature) : CasterAI(creature)
            {
                Initialize();
            }

            void Initialize()
            {
                // Not needed to be despawned now
                _despawnTimer = 0;
            }

        uint32 despawnTimer;
        bool targetFound;
            void InitializeAI() override
            {
                Initialize();

                CasterAI::InitializeAI();
                ObjectGuid ownerGuid = me->GetOwnerGUID();
                if (!ownerGuid)
                    return;
            // Not needed to be despawned now
            despawnTimer = 0;
            _TargetCheck = 0;
            targetFound = false;
                // Find victim of Summon Gargoyle spell
                std::list<Unit*> targets;
                Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(me, me, 45.0f);
                Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(me, targets, u_check);
                me->VisitNearbyObject(45.0f, searcher);
                for (std::list<Unit*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
                    if ((*iter)->HasAura(SPELL_DK_SUMMON_GARGOYLE_1, ownerGuid))
                    {
                        if (!(*iter)->HasBreakableByDamageCrowdControlAura())
                            if (me->GetOwner() && me->GetVictim() && me->GetOwner()->GetTypeId() == TYPEID_PLAYER && !me->GetOwner()->CanSeeOrDetect(me->GetVictim(), false, true))
                                me->Attack((*iter), false);
                    targetFound = true;
                        break;
                    }
            }

        void JustDied(Unit* /*killer*/) override
            {
                // Stop Feeding Gargoyle when it dies
                if (Unit* owner = me->GetOwner())
                    owner->RemoveAurasDueToSpell(SPELL_DK_SUMMON_GARGOYLE_2);
            }

            // Fly away when dismissed
            void SpellHit(Unit* source, SpellInfo const* spell) override
            {
                if (spell->Id != SPELL_DK_DISMISS_GARGOYLE || !me->IsAlive())
                    return;

                Unit* owner = me->GetOwner();
                if (!owner || owner != source)
                    return;

                // Stop Fighting
                me->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE, true);

                // Sanctuary
                me->CastSpell(me, SPELL_DK_SANCTUARY, true);
                me->SetReactState(REACT_PASSIVE);
                me->CastStop();

                //! HACK: Creature's can't have MOVEMENTFLAG_FLYING
                // Fly Away
            me->AddUnitMovementFlag(MOVEMENTFLAG_CAN_FLY|MOVEMENTFLAG_ASCENDING|MOVEMENTFLAG_FLYING);
                me->SetSpeedRate(MOVE_FLIGHT, 0.75f);
                me->SetSpeedRate(MOVE_RUN, 0.75f);

                float x = me->GetPositionX() + 20 * std::cos(me->GetOrientation());
                float y = me->GetPositionY() + 20 * std::sin(me->GetOrientation());
                float z = me->GetPositionZ() + 50;
                me->GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MovePoint(0, x, y, z);

                // Despawn as soon as possible
                _despawnTimer = 4 * IN_MILLISECONDS;
            }

            void UpdateAI(uint32 diff) override
            {
                if (Unit* owner = me->GetOwner())
                    if (me->GetVictim() && owner->GetTypeId() == TYPEID_PLAYER && !owner->CanSeeOrDetect(me->GetVictim(), false, true))
                    {
                        me->AI()->EnterEvadeMode(CreatureAI::EVADE_REASON_OTHER);
                        me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle());
                    }
                    
                // if evade mode finding for next target && GARGOYLE CANT ATTACK ANOTHER TARGET THAN TAGGED (ON SUMMON)
            if (!targetFound)
                {
                    ObjectGuid ownerGuid = me->GetOwnerGUID();
                    if (!ownerGuid)
                        return;

                    std::list<Unit*> targets;
                    Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(me, me, 45.0f);
                    Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(me, targets, u_check);
                    me->VisitNearbyObject(45.0f, searcher);
                    for (std::list<Unit*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
                        if ((*iter)->HasAura(SPELL_DK_SUMMON_GARGOYLE_1, ownerGuid))
                        {
                            if (!(*iter)->HasBreakableByDamageCrowdControlAura())
                                if (me->GetVictim() && me->GetOwner()->GetTypeId() == TYPEID_PLAYER && !me->GetOwner()->CanSeeOrDetect(me->GetVictim(), false, true))
                                    me->Attack((*iter), false);
                        targetFound = true;
                            break;
                        }
                }

                if (_despawnTimer > 0)
                {
                    if (_despawnTimer > diff)
                        _despawnTimer -= diff;
                    else
                        me->DespawnOrUnsummon();
                    return;
                }

                CasterAI::UpdateAI(diff);
				if (_TargetCheck <= diff)
                    if (me->IsValidAssistTarget(me->GetOwner()))
                        me->Attack(me->GetOwner()->GetVictim(),false);
            }

        private:
           uint32 _despawnTimer;
		   uint32 _TargetCheck;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_pet_dk_ebon_gargoyleAI(creature);
        }
};

void AddSC_deathknight_pet_scripts()
{
    new npc_pet_dk_ebon_gargoyle();
}
