/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Deadmines
SD%Complete: 0
SDComment: Placeholder
SDCategory: Deadmines
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "deadmines.h"
#include "Spell.h"
#include "Player.h"
#include "WorldSession.h"

/*#####
# item_Defias_Gunpowder
#####*/

class item_defias_gunpowder : public ItemScript
{
public:
    item_defias_gunpowder() : ItemScript("item_defias_gunpowder") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        InstanceScript* instance = player->GetInstanceScript();

        if (!instance)
        {
            player->GetSession()->SendNotification("Instance script not initialized");
            return true;
        }

        if (instance->GetData(EVENT_STATE) != CANNON_NOT_USED)
            return false;

        if (targets.GetGOTarget() && targets.GetGOTarget()->GetEntry() == GO_DEFIAS_CANNON)
            instance->SetData(EVENT_STATE, CANNON_GUNPOWDER_USED);

        player->DestroyItemCount(item->GetEntry(), 1, true);
        return true;
    }
};
// THIS SYSTEM IS NOT COMPLETE SO MAKE IT UNUSED
class npc_dm_diff : public CreatureScript
{
public:
    npc_dm_diff() : CreatureScript("npc_dm_diff") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (Difficult > 0)
            return false;
 
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF+1)
            Difficult = 1;
        
        if (action == GOSSIP_ACTION_INFO_DEF+2)
            Difficult = 2;
        
        if (Difficult > 0)
        {
            std::list<Unit*> targets;
            Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(creature, creature, 999.0f);
            Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(creature, targets, u_check);
            creature->VisitNearbyObject(999.0f, searcher);
            for (std::list<Unit*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
            {
                if ((*iter)->GetTypeId() == TYPEID_PLAYER)
                    continue;
                    
                    if (Creature* me = (*iter)->ToCreature())
                    {
                        uint32 maxhealth = me->GetMaxHealth();
                        me->SetMaxHealth(maxhealth * (1.33f * Difficult));
                        me->SetHealth(maxhealth * (1.33f * Difficult));
                        me->CastSpell(me, 29660, true);
                        if (Aura* aura = me->GetAura(29660))
                            aura->SetStackAmount(Difficult*(3+Difficult));
                    }
                
            }
            creature->SetPhaseMask(8, true);
            player->GetMap()->SetCustomDifficulty(Difficult);
        }
        player->CLOSE_GOSSIP_MENU();
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        InstanceScript* instance = player->GetInstanceScript();

        if (!instance || player->GetMap()->GetCustomDifficulty() > 0)
        {
            player->GetSession()->SendNotification("Instance script not initialized");
            return true;
        }

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Set to heroic difficult? (308ItemLevel)", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Set to mythic difficult? (318ItemLevel)", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        player->SEND_GOSSIP_MENU(10000, creature->GetGUID());
        return true;
    }

};

void AddSC_deadmines()
{
    new item_defias_gunpowder();
    new npc_dm_diff();
}
