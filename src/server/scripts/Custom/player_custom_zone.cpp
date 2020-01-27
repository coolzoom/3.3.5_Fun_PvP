/*
* Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "Chat.h"
#include "Language.h"
#include "Group.h"
#include "GroupMgr.h"
#include "GameEventMgr.h"
#include "GameObjectAI.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "MapManager.h"
#include "ScriptMgr.h"
#include "ChannelMgr.h"
#include "WorldSession.h"
#include "Player.h"
#include "ObjectMgr.h"

#define valuta_kill 38186 
#define MENU_INFO_LEVEL_HELP "|TInterface\\icons\\Inv_misc_coin_01:16:16:-19:0|t Как это работает? Вы убиваете игроков в пвп зоне хиджал и получаете за это (Хиджал Пойнт) который нужно обменять на виртуальные Хиджал монеты. Вы должны выбрать меню (Повысить уровень скидки) и ввести количество Хиджал Пойнтов которые хотите вложить. Это повысит ваш уровень скидки."
#define MENU_INFO_LEVEL_ZERO "|TInterface\\icons\\Inv_misc_coin_01:16:16:-19:0|t 0 Уровень: 0 до 150."
#define MENU_INFO_LEVEL_ONE "|TInterface\\icons\\Inv_misc_coin_01:16:16:-19:0|t 1 Уровень: 150 до 300."
#define MENU_INFO_LEVEL_TWO "|TInterface\\icons\\Inv_misc_coin_01:16:16:-19:0|t 2 Уровень: 300 до 600."
#define MENU_INFO_LEVEL_THREE "|TInterface\\icons\\Inv_misc_coin_01:16:16:-19:0|t 3 Уровень: 600 до 1200."
#define MENU_INFO_LEVEL_FOUR "|TInterface\\icons\\Inv_misc_coin_01:16:16:-19:0|t 4 Уровень: 1200 до 2100."
#define MENU_INFO_LEVEL_FIVE "|TInterface\\icons\\Inv_misc_coin_01:16:16:-19:0|t MAX Уровень: с 2100."
#define MENU_INFO_LEVEL_BACK "|TInterface\\icons\\Spell_chargenegative:16:16:-19:0|t Назад"

enum Misc
{
    /* ENTRY ПРОДАВЦОВ БРОНИ */
    ENTRY_ARMOR_VENDOR_LEVEL_ONE     = 250002,
    ENTRY_ARMOR_VENDOR_LEVEL_TWO     = 250003,
    ENTRY_ARMOR_VENDOR_LEVEL_THREE   = 250004,
    ENTRY_ARMOR_VENDOR_LEVEL_FOUR    = 250005,
    ENTRY_ARMOR_VENDOR_LEVEL_FIVE    = 250006,
    /* ENTRY ПРОДАВЦОВ ОРУЖИЯ */
    ENTRY_WEAPON_VENDOR_LEVEL_ONE    = 250007,
    ENTRY_WEAPON_VENDOR_LEVEL_TWO    = 250008,
    ENTRY_WEAPON_VENDOR_LEVEL_THREE  = 250009,
    ENTRY_WEAPON_VENDOR_LEVEL_FOUR   = 250010,
    ENTRY_WEAPON_VENDOR_LEVEL_FIVE   = 250011,
    /* УРОВНИ СКИДОК */
    LEVEL_ZERO                       = 0,
    LEVEL_ONE                        = 150,
    LEVEL_TWO                        = 300,
    LEVEL_THREE                      = 600,
    LEVEL_FOUR                       = 1200,
    LEVEL_FIVE                       = 2100,
    /* ID ВАЛЮТЫ ХИДЖАЛА */
    HYJAL_POINT_ID                   = 43514,
    /* ID ЗОНЫ И ОБЛАСТИ ХИДЖАЛА */
    HYJAL_ZONE_ID                    = 616,
    HYJAL_AREA_ID                    = 616
};

class pvp_hijal : public CreatureScript
{
public:
    pvp_hijal() : CreatureScript("pvp_hijal") { }

    struct pvp_hijalAI : public ScriptedAI
    {
        pvp_hijalAI(Creature* creature) : ScriptedAI(creature) { }

        void MoveInLineOfSight(Unit* who)
        {
            if (me->IsWithinDistInMap(who, 30.0f) && who->GetTypeId() == TYPEID_PLAYER)
            who->ToPlayer()->TeleportTo(1, 5257.938965f, -1920.361816f, 1355.876953f, 3.578476f);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new pvp_hijalAI(creature);
    }
}; 

using namespace std;

class world_pvp : public PlayerScript
{
public:
    world_pvp() : PlayerScript("world_pvp") { }

    void PlayerIsInGroup(Player* killer)
    {
        if (killer->GetZoneId() == HYJAL_ZONE_ID && killer->GetAreaId() == HYJAL_AREA_ID)
        {
            /* Group * group = killer->GetGroup();
            Group::MemberSlotList const &members = group->GetMemberSlots();

            for (Group::MemberSlotList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
            {
                Group::MemberSlot const &slot = *itr;
                Player* killer = ObjectAccessor::FindPlayer((*itr).guid);
                */

            if (killer && killer->GetSession())
            killer->AddItem(HYJAL_POINT_ID, 1);                
            //}
        }			
    }

    void OnPVPKill(Player * killer, Player * victim)
    {
        
        if (victim->GetMapId() == 1 && victim->GetZoneId() == HYJAL_ZONE_ID && victim->GetAreaId() == HYJAL_AREA_ID)
        {		
            if (killer == victim)
            return;

            if (killer->GetGroup())
            {
                PlayerIsInGroup(killer);
            }
            else
            {
                killer->AddItem(HYJAL_POINT_ID, 1);
            }

            uint32 spawnpoints[]={1,2,3,4,5};
            int random= urand(1,5);

            if (spawnpoints[random] == 1)
            {
                victim->TeleportTo(1, 5257.938965f, -1920.361816f, 1355.876953f, 3.578476f);
                victim->ResurrectPlayer(1.0f, false);
            }					
            else if (spawnpoints[random] == 2)
            {
                victim->TeleportTo(1, 5218.667969f, -2082.810303f, 1280.192505f, 0.6970f);
                victim->ResurrectPlayer(1.0f, false);
            }
            else if (spawnpoints[random] == 3)
            { 
                victim->TeleportTo(1, 5372.805664f, -2187.191406f, 1297.666748f, 3.0713f);
                victim->ResurrectPlayer(1.0f, false);
            }					
            else if (spawnpoints[random] == 4)
            { 
                victim->TeleportTo(1, 5365.471680f, -2121.912354f, 1295.847412f, 3.989835f);
                victim->ResurrectPlayer(1.0f, false);
            }					
            else if (spawnpoints[random] == 5)
            { 
                victim->TeleportTo(1, 5248.508789f, -2067.746582f, 1276.984375f, 6.2506f);
                victim->ResurrectPlayer(1.0f, false);
            }
            killer->SaveToDB();
        }
    }

    void OnMapChanged(Player* plr)
    {
        if (plr->GetZoneId() == HYJAL_ZONE_ID && plr->GetAreaId() == HYJAL_AREA_ID)
        {
            plr->SaveRecallPosition();
            std::ostringstream battle_for_the_hyjal_announce_enter;
            battle_for_the_hyjal_announce_enter << "Игрок |cff00FF00" << plr->GetName() << "|r вступает в битву за |cffFF0000[Хиджал]|r";
            sWorld->SendZoneText(HYJAL_ZONE_ID, battle_for_the_hyjal_announce_enter.str().c_str(), NULL);
        }
        		
        if (plr->GetZoneId() != HYJAL_ZONE_ID && plr->GetAreaId() != HYJAL_AREA_ID)
        return;

        Player* player;
        uint32 numPlayers = 0;
        Map::PlayerList const &players = plr->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        if (player = itr->GetSource()->ToPlayer())
        if (player->GetZoneId()== HYJAL_ZONE_ID && player->GetAreaId() == HYJAL_AREA_ID && !player->IsGameMaster())
        numPlayers++;

        std::ostringstream private_message_on_hyjal_1;
        std::ostringstream private_message_on_hyjal_2;
        private_message_on_hyjal_1 << "|cffFFFFFFВы вступили в битву за |cffFF0000[Хиджал]|r";
        private_message_on_hyjal_2 << "|cff45fe00Игроков на поле боя:|r |cffff0000" + std::to_string(numPlayers) + "|r ";
        ChatHandler(plr->GetSession()).PSendSysMessage(private_message_on_hyjal_1.str().c_str());
        ChatHandler(plr->GetSession()).PSendSysMessage(private_message_on_hyjal_2.str().c_str());
    }

    void OnUpdateZone(Player* player, uint32 newZone, uint32 newArea) override
    {
        if (newZone == HYJAL_ZONE_ID && newArea == HYJAL_AREA_ID && player->GetGroup())
        {
            if (player->GetGroup()->GetMembersCount() > 1)
            {
                player->GetGroup()->Disband();
                return;
            }
        }		
    }
};


#include "MapManager.h"
struct BroadcastData { uint32 time; };

struct BroadcastData Broadcasts[] = {300000};

#define BROADCAST_COUNT  sizeof Broadcasts/sizeof(*Broadcasts)

class pvp_zone_hyjal_announces : public WorldScript
{
public:
    pvp_zone_hyjal_announces(): WorldScript("pvp_zone_hyjal_announces") { }
    
    std::string GetZoneOnline()
    {
        SessionMap const& sessions = sWorld->GetAllSessions();
        uint32 count = 0;
        for (SessionMap::const_iterator it = sessions.begin(); it != sessions.end(); ++it)
        {
            if (Player* player = it->second->GetPlayer())
            {
                if (player->IsInWorld() && player->GetZoneId() == HYJAL_ZONE_ID && player->GetAreaId() == HYJAL_AREA_ID)
                ++count;
            }
        }

        if (count < 1 || count >= 5)
        {
            std::string str = "|cFF008000"+std::to_string(count) + "|cff00e600 игроков.|r";
            return str;
        }

        if (count == 1)
        {
            std::string str = "|cFF008000"+std::to_string(count) + "|cff00e600 игрок.|r";
            return str;
        }

        if (count == 2 || count == 3 || count == 4)
        {
            std::string str = "|cFF008000"+std::to_string(count) + "|cff00e600 игрока.|r";
            return str;
        }
    }
    
    void OnStartup()
    {
        for(uint32 i = 0; i < BROADCAST_COUNT; i++)
        events.ScheduleEvent(i+1, Broadcasts[i].time);
    }

    void OnUpdate(uint32 diff)
    {
        events.Update(diff);
        while (uint32 id = events.ExecuteEvent())
        {
            if (id <= BROADCAST_COUNT)
            {
                std::ostringstream ss;
                ss << "|cffff6060[ПВП|cff00e600 Зона|cffff6060 Хиджал]:|cff00e600 В данный момент "+ GetZoneOnline();
                sWorld->SendServerMessage(SERVER_MSG_STRING, ss.str().c_str());
                events.ScheduleEvent(id, Broadcasts[id-1].time);
            }    
        }
    }
private:
    EventMap events;
};

class npc_pvp_zone_hyjal_vendor : public CreatureScript
{
public:
    npc_pvp_zone_hyjal_vendor() : CreatureScript("npc_pvp_zone_hyjal_vendor") { }
    
    bool OnGossipHello(Player* pPlayer, Creature* pCreature) override
    {

        pPlayer->PlayerTalkClass->ClearMenus();
        QueryResult result = CharacterDatabase.PQuery("SELECT hidjal FROM characters WHERE guid = %u", pPlayer->GetGUID());
        if (result)
        {
            
            Field * fields = NULL;
            fields = result->Fetch();	
            uint32 hidjal = fields[0].GetUInt32();
            
            uint32 ostatok1 = LEVEL_ONE - hidjal;
            uint32 ostatok2 = LEVEL_TWO - hidjal;
            uint32 ostatok3 = LEVEL_THREE - hidjal;
            uint32 ostatok4 = LEVEL_FOUR - hidjal;
            uint32 ostatok5 = LEVEL_FIVE - hidjal;		
            pPlayer->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Inv_weapon_rifle_21:25:25:-19:0|t|cffff0000[Вступить в бой]|r", GOSSIP_SENDER_MAIN, 9);
            if (hidjal >= LEVEL_ZERO && hidjal < LEVEL_ONE)
            {
                std::ostringstream femb;
                femb << "|TInterface\\icons\\inv_jewelry_talisman_08:25:25:-19:0|tУровень вашей скидки: [|cffE80000 0 |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femb.str(), GOSSIP_SENDER_MAIN, 1);
                std::ostringstream femt;
                femt << "|TInterface\\icons\\Spell_holy_dizzy:25:25:-19:0|tВложено: [|cffE80000 " <<  hidjal << " |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femt.str(), GOSSIP_SENDER_MAIN, 1);			
                std::ostringstream femg;
                femg << "|TInterface\\icons\\Spell_holy_blessingofstrength:25:25:-19:0|tДо след уровня осталось: [|cffE80000 " << ostatok1 << " |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femg.str(), GOSSIP_SENDER_MAIN, 1);			
            }
            if (hidjal >= LEVEL_ONE && hidjal < LEVEL_TWO)
            {
                std::ostringstream femb;
                femb << "|TInterface\\icons\\inv_jewelry_talisman_08:25:25:-19:0|tУровень вашей скидки: [|cffE80000 1 |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femb.str(), GOSSIP_SENDER_MAIN, 1);
                std::ostringstream femt;
                femt << "|TInterface\\icons\\Spell_holy_dizzy:25:25:-19:0|tВложено: [|cffE80000 " <<  hidjal << " |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femt.str(), GOSSIP_SENDER_MAIN, 1);			
                std::ostringstream femg;
                femg << "|TInterface\\icons\\Spell_holy_blessingofstrength:25:25:-19:0|tДо след уровня осталось: [|cffE80000 " << ostatok2 << " |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femg.str(), GOSSIP_SENDER_MAIN, 1);			
            }
            if (hidjal >= LEVEL_TWO && hidjal < LEVEL_THREE)
            {
                std::ostringstream femb;
                femb << "|TInterface\\icons\\inv_jewelry_talisman_08:25:25:-19:0|tУровень вашей скидки: [|cffE80000 2 |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femb.str(), GOSSIP_SENDER_MAIN, 1);
                std::ostringstream femt;
                femt << "|TInterface\\icons\\Spell_holy_dizzy:25:25:-19:0|tВложено: [|cffE80000 " <<  hidjal << " |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femt.str(), GOSSIP_SENDER_MAIN, 1);			
                std::ostringstream femg;
                femg << "|TInterface\\icons\\Spell_holy_blessingofstrength:25:25:-19:0|tДо след уровня осталось: [|cffE80000 " << ostatok3 << " |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femg.str(), GOSSIP_SENDER_MAIN, 1);			
            }
            if (hidjal >= LEVEL_THREE && hidjal < LEVEL_FOUR)
            {
                std::ostringstream femb;
                femb << "|TInterface\\icons\\inv_jewelry_talisman_08:25:25:-19:0|tУровень вашей скидки: [|cffE80000 3 |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femb.str(), GOSSIP_SENDER_MAIN, 1);
                std::ostringstream femt;
                femt << "|TInterface\\icons\\Spell_holy_dizzy:25:25:-19:0|tВложено: [|cffE80000 " <<  hidjal << " |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femt.str(), GOSSIP_SENDER_MAIN, 1);			
                std::ostringstream femg;
                femg << "|TInterface\\icons\\Spell_holy_blessingofstrength:25:25:-19:0|tДо след уровня осталось: [|cffE80000 " << ostatok4 << " |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femg.str(), GOSSIP_SENDER_MAIN, 1);			
            }
            if (hidjal >= LEVEL_FOUR && hidjal < LEVEL_FIVE)
            {
                std::ostringstream femb;
                femb << "|TInterface\\icons\\inv_jewelry_talisman_08:25:25:-19:0|tУровень вашей скидки: [|cffE80000 4 |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femb.str(), GOSSIP_SENDER_MAIN, 1);
                std::ostringstream femt;
                femt << "|TInterface\\icons\\Spell_holy_dizzy:25:25:-19:0|tВложено: [|cffE80000 " <<  hidjal << " |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femt.str(), GOSSIP_SENDER_MAIN, 1);			
                std::ostringstream femg;
                femg << "|TInterface\\icons\\Spell_holy_blessingofstrength:25:25:-19:0|tДо след уровня осталось: [|cffE80000 " << ostatok5 << " |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femg.str(), GOSSIP_SENDER_MAIN, 1);			
            }			
            if (hidjal >= LEVEL_FIVE)
            {
                std::ostringstream femb;
                femb << "|TInterface\\icons\\inv_jewelry_talisman_08:25:25:-19:0|tУровень вашей скидки: [|cffE80000 MAX |r]";
                pPlayer->ADD_GOSSIP_ITEM(0, femb.str(), GOSSIP_SENDER_MAIN, 1);
            }		
            
            pPlayer->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_magic_polymorphrabbit:25:25:-19:0|t|cffff0000[Помощь]|r Как работает эта система?", GOSSIP_SENDER_MAIN, 6);
            
            if (hidjal < LEVEL_FIVE)
            { 
                pPlayer->ADD_GOSSIP_ITEM_EXTENDED(9, "|TInterface\\icons\\Spell_chargepositive:25:25:-19:0|tПовысить уровень скидки", GOSSIP_SENDER_MAIN, 5, "Введите количество Хиджал Пойнтов которое вы хотите вложить:", 0, true);
            }
        }
        pPlayer->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Inv_enchant_shardnexuslarge:25:25:-19:0|tТопы Хиджала", GOSSIP_SENDER_MAIN, 7);
        pPlayer->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Achievement_arena_2v2_1:25:25:-19:0|tPvP броня", GOSSIP_SENDER_MAIN, 2);
        pPlayer->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Achievement_arena_5v5_1:25:25:-19:0|tPvP оружие", GOSSIP_SENDER_MAIN, 4);
        pPlayer->SEND_GOSSIP_MENU(1, pCreature->GetGUID());
        return true;
    }
    
    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction) override
    {
        pPlayer->PlayerTalkClass->ClearMenus();

        switch (uiAction)
        {
        case 1:
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        case 9:
            if (pPlayer->GetGroup())
            {
                pPlayer->GetSession()->SendAreaTriggerMessage("Вы не сможете попасть в PvP зону Хиджал, пока не покинете группу!");
                pPlayer->CLOSE_GOSSIP_MENU();
                return false;
            }
            
            if (!pPlayer && pPlayer->IsBeingTeleported() && !pPlayer->IsAlive() && pPlayer->IsInCombat())
            {
                pPlayer->GetSession()->SendAreaTriggerMessage("Вы не сможете попасть в PvP зону Хиджал, пока не выйдите из состояния боя!");
                pPlayer->CLOSE_GOSSIP_MENU();
                return false;
            }

            if (sGameEventMgr->IsActiveEvent(70))
            {
                pPlayer->GetSession()->SendAreaTriggerMessage("Вы не сможете попасть в PvP зону Хиджал, пока не повержен мировой босс 'Бессмертые души'!");
                pPlayer->CLOSE_GOSSIP_MENU();
                return false;
            }
            pPlayer->TeleportTo(1, 5260.493652f, -2166.809326f, 1259.482056f, 1.484607f);
            break;
        case 2:
            {
                QueryResult result = CharacterDatabase.PQuery("SELECT hidjal FROM characters WHERE guid = %u", pPlayer->GetGUID());
                if (result)
                {
                    Field * fields = NULL;
                    fields = result->Fetch();	
                    uint32 hidjal = fields[0].GetUInt32();
                    
                    if (hidjal >= LEVEL_ZERO && hidjal < LEVEL_ONE)
                    {
                        ChatHandler(pPlayer->GetSession()).PSendSysMessage("|cff006699У вас недостаточно высокий уровень скидки!|r");  
                    }
                    if (hidjal >= LEVEL_ONE && hidjal < LEVEL_TWO)
                    {
                        pPlayer->GetSession()->SendListInventory(pCreature->GetGUID(), ENTRY_ARMOR_VENDOR_LEVEL_ONE);  
                    }
                    if (hidjal >= LEVEL_TWO && hidjal < LEVEL_THREE)
                    {
                        pPlayer->GetSession()->SendListInventory(pCreature->GetGUID(), ENTRY_ARMOR_VENDOR_LEVEL_TWO);  
                    }
                    if (hidjal >= LEVEL_THREE && hidjal < LEVEL_FOUR)
                    {
                        pPlayer->GetSession()->SendListInventory(pCreature->GetGUID(), ENTRY_ARMOR_VENDOR_LEVEL_THREE);  
                    }
                    if (hidjal >= LEVEL_FOUR && hidjal < LEVEL_FIVE)
                    {
                        pPlayer->GetSession()->SendListInventory(pCreature->GetGUID(), ENTRY_ARMOR_VENDOR_LEVEL_FOUR);  
                    }	
                    if (hidjal >= LEVEL_FIVE)
                    {
                        pPlayer->GetSession()->SendListInventory(pCreature->GetGUID(), ENTRY_ARMOR_VENDOR_LEVEL_FIVE);  
                    }						  
                }					  
            }
            break;				
        case 4:
            {
                QueryResult result = CharacterDatabase.PQuery("SELECT hidjal FROM characters WHERE guid = %u", pPlayer->GetGUID());
                if (result)
                {
                    Field * fields = NULL;
                    fields = result->Fetch();	
                    uint32 hidjal = fields[0].GetUInt32();
                    
                    if (hidjal >= LEVEL_ZERO && hidjal < LEVEL_ONE)
                    {
                        ChatHandler(pPlayer->GetSession()).PSendSysMessage("|cff006699У вас недостаточно высокий уровень скидки!|r");  
                    }
                    if (hidjal >= LEVEL_ONE && hidjal < LEVEL_TWO)
                    {
                        pPlayer->GetSession()->SendListInventory(pCreature->GetGUID(), ENTRY_WEAPON_VENDOR_LEVEL_ONE);  
                    }
                    if (hidjal >= LEVEL_TWO && hidjal < LEVEL_THREE)
                    {
                        pPlayer->GetSession()->SendListInventory(pCreature->GetGUID(), ENTRY_WEAPON_VENDOR_LEVEL_TWO);  
                    }
                    if (hidjal >= LEVEL_THREE && hidjal < LEVEL_FOUR)
                    {
                        pPlayer->GetSession()->SendListInventory(pCreature->GetGUID(), ENTRY_WEAPON_VENDOR_LEVEL_THREE);  
                    }
                    if (hidjal >= LEVEL_FOUR && hidjal < LEVEL_FIVE)
                    {
                        pPlayer->GetSession()->SendListInventory(pCreature->GetGUID(), ENTRY_WEAPON_VENDOR_LEVEL_FOUR);  
                    }	
                    if (hidjal >= LEVEL_FIVE)
                    {
                        pPlayer->GetSession()->SendListInventory(pCreature->GetGUID(), ENTRY_WEAPON_VENDOR_LEVEL_FIVE);  
                    }						  
                }					  
            }
            break;
        case 6:
            {		
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, MENU_INFO_LEVEL_HELP, GOSSIP_SENDER_MAIN, 1);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, MENU_INFO_LEVEL_ZERO, GOSSIP_SENDER_MAIN, 1);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, MENU_INFO_LEVEL_ONE, GOSSIP_SENDER_MAIN, 1);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, MENU_INFO_LEVEL_TWO, GOSSIP_SENDER_MAIN, 1);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, MENU_INFO_LEVEL_THREE, GOSSIP_SENDER_MAIN, 1);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, MENU_INFO_LEVEL_FOUR, GOSSIP_SENDER_MAIN, 1);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, MENU_INFO_LEVEL_FIVE, GOSSIP_SENDER_MAIN, 1);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, MENU_INFO_LEVEL_BACK, GOSSIP_SENDER_MAIN, 1);
                pPlayer->PlayerTalkClass->SendGossipMenu(1, pCreature->GetGUID());
            }
            break;
            
        case 7:
            {
                QueryResult result = CharacterDatabase.Query("SELECT name,hidjal FROM characters ORDER BY hidjal DESC LIMIT 10");
                if(result)
                {

                    std::string name;
                    uint32 rank = 1;
                    Field * fields = NULL;
                    do
                    {
                        fields        = result->Fetch();
                        name          = fields[0].GetString();
                        uint32 hidjal = fields[1].GetUInt32();
                        
                        std::stringstream buffer;
                        buffer << "|TInterface\\icons\\Inv_enchant_shardnexuslarge:20:20:-19:0|t"<< rank << " Ник: |cff0033CC" << name << "|r очков: |cff00CCCC" << hidjal << "|r";
                        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, buffer.str(), GOSSIP_SENDER_MAIN, 1);
                        rank++;
                    }
                    while(result->NextRow());
                    pPlayer->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetGUID());
                    pPlayer->SaveToDB();
                }
            }					
            break;
        }
        return true;
    }
    bool OnGossipSelectCode(Player* player, Creature* pCreature, uint32 sender, uint32 action, const char* code)
    {
        switch(action)
        {
        case 5:
            ByeHidjal(player, pCreature, code);
            break;
        }
        return true;
    }

    void ByeHidjal(Player* player, Creature* pCreature, char const* code)	
    {

        uint32 token = atoi(code);
        
        if (player->HasItemCount(HYJAL_POINT_ID, token))
        {
            player->DestroyItemCount(HYJAL_POINT_ID, token, true);
            CharacterDatabase.PQuery("UPDATE characters SET hidjal = hidjal+%u WHERE guid = %u", token, player->GetGUID());
            player->SaveToDB();

            player->GetSession()->SendAreaTriggerMessage("Обмен успешно произошел!");
            player->CLOSE_GOSSIP_MENU();
        }
        else
        {
            player->GetSession()->SendNotification("У вас недостаточно Хиджал Пойнтов!");
            player->CLOSE_GOSSIP_MENU();
        }		
    }		
};

class gob_event : public GameObjectScript
{
public:
    gob_event() : GameObjectScript("gob_event") { }

    bool OnGossipHello(Player * player, GameObject * go)
    {
        
        uint32 pvptoken = urand(5, 50);
        uint32 transvol = urand(1, 2);
        uint32 honor = urand(1000, 10000);	
        uint32 arena = urand(20, 60);
        
        player->AddItem(38186, pvptoken);
        player->AddItem(34628, transvol);
        player->ModifyHonorPoints(+honor);
        player->ModifyArenaPoints(+arena);
        
        std::ostringstream ss;
        ss << "|cff99CC66[Поиск Сокровищ]:|cFFF3E2A9 Игрок |cff99CC66"<< player->GetName().c_str() << "|cFFF3E2A9 нашел кристал и получил [|cff99CC66 " << pvptoken << "|cFFF3E2A9 PvP Token, |cff99CC66" << transvol << "|cFFF3E2A9 PVP XaviUs, |cff99CC66" << honor << "|cFFF3E2A9 очков чести, |cff99CC66" << arena << "|cFFF3E2A9 арена пойнтов ].|r";
        sWorld->SendServerMessage(SERVER_MSG_STRING, ss.str().c_str());
        go->SetRespawnTime(0);
        go->Delete();
        go->DeleteFromDB();
        player->CLOSE_GOSSIP_MENU();
    }
};

struct BroadcastData Broadcastss[] =
{
    {1800000},
};

const int MAX_POWERUP_SPAWN_POINTS = 10;
static const Position powerSpawnPoint[MAX_POWERUP_SPAWN_POINTS] = 
{
    {5401.054199, -2228.073242, 1328.340210, 0},  
    {5371.348633, -2117.359863, 1296.060913, 0},  
    {5263.796387, -2163.597900, 1259.471069, 0},  
    {5299.274414, -2268.433105, 1312.693237, 0},  
    {5193.908203, -2330.928955, 1318.296753, 0},  
    {5148.775391, -2298.490234, 1291.271484, 0},  
    {5213.022461, -2126.643799, 1271.223389, 0},  
    {5212.991211, -2028.673584, 1307.308716, 0},  
    {5257.389648, -1961.742798, 1304.248169, 0},  
    {5337.941406, -1806.320312, 1375.338989, 0},
};

class event_announce : public WorldScript
{
public:
    event_announce(): WorldScript("event_announce") { }
    
    void OnStartup()
    {
        for(uint32 i = 0; i < BROADCAST_COUNT; i++)
        events.ScheduleEvent(i+1, Broadcastss[i].time);
    }

    void OnUpdate(uint32 diff)
    {
        events.Update(diff);
        while (uint32 id = events.ExecuteEvent())
        {
            if (id <= BROADCAST_COUNT)
            {	
                std::ostringstream ss;
                ss << "|cffff0000[Анонс|cff00CD00Ивентов|cffff0000]: |cff00ff00Стартовал|cFFF3E2A9 Ивент |cff0000ffПоиск Сокровищ|cFFF3E2A9. Найдите кристал [В Пвп зоне Хиджал]. Повторный ивент будет через 30 минут.";
                sWorld->SendServerMessage(SERVER_MSG_STRING, ss.str().c_str());
                events.ScheduleEvent(id, Broadcastss[id-1].time);

                uint32 objectId = 300041;

                uint32 spawntimeSecs = 30*60;

                const GameObjectTemplate* objectInfo = sObjectMgr->GetGameObjectTemplate(objectId);

                if (!objectInfo)
                {
                    return;
                }

                if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
                {
                    return;
                }
                
                int i = urand(0, MAX_POWERUP_SPAWN_POINTS - 1);
                float x = powerSpawnPoint[i].GetPositionX();
                float y = powerSpawnPoint[i].GetPositionY();
                float z = powerSpawnPoint[i].GetPositionZ();
                float o = powerSpawnPoint[i].GetOrientation();
                Map* map = sMapMgr->FindMap(1, 0);
                
                if (!map)
                return;

                GameObject* object = new GameObject;
                ObjectGuid::LowType guidLow = map->GenerateLowGuid<HighGuid::GameObject>();

                if (!object->Create(guidLow, objectInfo->entry, map, 1, x, y, z, o, 0.0f, 0.0f, 0.0f, 0.0f, 0, GO_STATE_READY))
                {
                    delete object;
                    return;
                }

                if (spawntimeSecs)
                {
                    uint32 value = spawntimeSecs;
                    object->SetRespawnTime(value);
                }

                object->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), 1);
                guidLow = object->GetSpawnId();
                delete object;
                object = new GameObject();

                if (!object->LoadGameObjectFromDB(guidLow, map))
                {
                    delete object;
                    return;
                }

                sObjectMgr->AddGameobjectToGrid(guidLow, sObjectMgr->GetGOData(guidLow));
                
            }    
        }
    }
private:
    EventMap events;
};


enum Spells
{
    //===================FireSpells==========================
    SPELL_SUNBEAM_FIRE                               = 62872,
    SPELL_SCORCH_FIRE                                = 63474,
    SPELL_JETS_FIRE                                  = 63472,
    SPELL_FIREBALL_FIRE                              = 68926,
    SPELL_FLAMING_CINDER_FIRE                        = 67332,
    SPELL_BURNING_BITE_FIRE                          = 67626,
    //===================IceSpells===========================
    SPELL_ICEBOLT_ICE                                = 31249,
    SPELL_ICECHAINS_ICE                              = 22744,
    SPELL_FROST_BLAST_ICE                            = 72123,
    SPELL_FROST_BLAST_1_ICE                          = 27808,
    SPELL_FROST_SHIELD_ICE                           = 62650,
    SPELL_ICEBLAST_ICE                               = 28522,
    //===================EarthSpells=========================
    SPELL_NATURE_EARTH                               = 62519, //Stacks
    SPELL_GROUND_EARTH                               = 62859,
    SPELL_LIFEBLOOM_EARTH                            = 67959, // x3 for more powerful healing
    SPELL_WRATH_EARTH                                = 67953,
    SPELL_STUN_EARTH                                 = 52402,
    SPELL_POISON_NOVA_EARTH                          = 68989,
    SPELL_PETRIFYING_BREATH_EARTH                    = 62030,
    //===================DarkSpells==========================
    SPELL_SOULS_SMALL_DARK                           = 72521,
    SPELL_SOULS_LARGE_DARK                           = 72523,
    SPELL_SOULS_SUMMON_DARK                          = 68967,
    SPELL_MIRRORED_DARK                              = 69051,
    SPELL_DEATH_AND_DECAY_DARK                       = 72110,
    SPELL_DARK_VOLLEY_DARK                           = 63038,
    //___________________NPCSpells___________________________
    //===================FireElemental=======================
    SPELL_CURSE_OF_FLAMES_ELEMENTAL                  = 38010,
    SPELL_FLAME_BREATH_ELEMENTAL                     = 64021,
    //===================IceMage=============================
    SPELL_BLIZZARD                                   = 71118,
    SPELL_CONE_OF_COLD                               = 64655,
    SPELL_COUNTERSPELL                               = 59111,
    //===================FireMage============================
    SPELL_HEAL                                       = 71120,
    SPELL_FEL_FIREBALL                               = 66532,
    SPELL_FIRE_NOVA                                  = 68958,
    SPELL_SHIP                                       = 118,
    //===================DeathKnight=========================
    SPELL_FROST_STRIKE                               = 67937,
    SPELL_DEATH_COIL                                 = 67929,
    SPELL_GLACIAL_STRIKE                             = 71317,
    SPELL_ANTIMAGIC_ZONE                             = 51052,
    //===================Warrior=============================
    SPELL_SHOCKWAVE                                  = 75418,
    SPELL_BLOODTHRIST                                = 33964,
    SPELL_DISARM                                     = 15752,
    //===================Common==============================
    SPELL_FRENZY                                     = 47774,  // 15% AS for 2 min. Stacks.
    SPELL_FURY                                       = 66721,  // 5% damage. Stacks

    KNOCKBACK = 69293, 
    FLAME = 68970, 
    METEOR = 45915,
};

enum eEnums
{
    NPC_FIRE_ELEMENTAL                               = 99007,
    GO_ICE_DOOR_1                                    =201910,
    GO_ICE_DOOR_2                                    =201911,
};

#define FIRE_SAY_AGGRO                                "Вам нечего здесь делать. Убирайтесь, пока живы!"
#define FIRE_SAY_FRENZY                               "Ярость наполняет меня!"
#define FIRE_SAY_SUMMON_TITAN                         "Слуги мои, помогите!"
#define FIRE_SAY_KILL                                 "Этим всё и закончится!"
#define FIRE_SAY_DIE                                  "На этот раз... вам повезло..."

class event_npc_firelord : public CreatureScript
{
public:

    event_npc_firelord()
    : CreatureScript("event_npc_firelord")
    {
    }

    struct event_npc_firelordAI : public ScriptedAI
    {
        event_npc_firelordAI(Creature *c) : ScriptedAI(c),  summons(c) {}
        
        SummonList summons;

        uint32 m_uiSunbeamTimer;
        uint32 m_uiScorchTimer;
        uint32 m_uiJetsTimer;
        uint32 m_uiFireballTimer;
        uint32 m_uiFrenzyTimer;
        uint32 m_uiSummonTimer;
        uint32 m_uiFlamingCinderTimer;
        uint32 m_uiPhase;
        uint32 m_uiSummonCheck;
        uint32 m_uiFuryTimer;
        uint32 m_uiBurningBiteTimer;

        void Reset() override
        {
            m_uiSunbeamTimer       =  urand(10000, 30000);
            m_uiScorchTimer        =  urand(10000, 35000);
            m_uiJetsTimer          =  urand(10000, 35000);
            m_uiFireballTimer      =  urand(10000, 28000);
            m_uiFrenzyTimer        =  70000;              //70 seconds
            m_uiSummonTimer        =  60000;
            m_uiFlamingCinderTimer =  urand(10000, 15000);
            m_uiPhase              =  1;
            m_uiSummonCheck        =  0;
            m_uiFuryTimer          =  urand(20000, 30000);
            m_uiBurningBiteTimer   =  urand(12000, 25000);
            me->SetReactState(REACT_DEFENSIVE);
        }

        void JustSummoned(Creature *summon) override
        {
            if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
            {
                summon->CastSpell(pTarget, SPELL_CURSE_OF_FLAMES_ELEMENTAL, true);
                summon->CastSpell(pTarget, SPELL_FLAME_BREATH_ELEMENTAL, true);
            }

            if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
            summon->AI()->AttackStart(pTarget);
            summons.Summon(summon);
        }

        void EnterCombat(Unit* pWho) override
        {
            me->Yell(FIRE_SAY_AGGRO, LANG_UNIVERSAL, 0);
            summons.DespawnAll();
        }
        
        void KilledUnit(Unit *victim) override
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
            me->Yell(FIRE_SAY_KILL, LANG_UNIVERSAL, 0);
        }
        
        void JustDied(Unit* /*killer*/) override
        {
            me->Yell(FIRE_SAY_DIE, LANG_UNIVERSAL, 0);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
            return;
            
            if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

            if (!me->GetVictim())
            {
                me->CombatStop(true);
                EnterEvadeMode();
            }

            if (m_uiPhase > 1)
            {
                //Jets Timer
                if (m_uiJetsTimer <= uiDiff)
                {
                    DoCast(me->GetVictim(), SPELL_JETS_FIRE);

                    m_uiJetsTimer = urand(10000, 30000);
                }
                else
                m_uiJetsTimer -= uiDiff;

                //Sunbeam Timer
                if (m_uiSunbeamTimer <= uiDiff)
                {
                    if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                    DoCast(pTarget, SPELL_SUNBEAM_FIRE);

                    m_uiSunbeamTimer = urand(10000, 20000);
                }
                else
                m_uiSunbeamTimer -= uiDiff;

                //Scorch Timer
                if (m_uiScorchTimer <= uiDiff)
                {
                    DoCast(me->GetVictim(), SPELL_SCORCH_FIRE);
                    m_uiScorchTimer = urand(10000, 30000);
                }
                else
                m_uiScorchTimer -= uiDiff;

                //Fireball Timer
                if (m_uiFireballTimer <= uiDiff)
                {
                    DoCast(me->GetVictim(), SPELL_FIREBALL_FIRE);
                    m_uiFireballTimer = urand(10000, 25000);
                }
                else
                m_uiFireballTimer -= uiDiff;

                if (m_uiFrenzyTimer <= uiDiff)
                {
                    me->Yell(FIRE_SAY_FRENZY, LANG_UNIVERSAL, 0);
                    DoCast(me, SPELL_FRENZY);
                    m_uiFrenzyTimer = urand(20000, 40000);
                }
                else
                m_uiFrenzyTimer -= uiDiff;

                //Flaming Cinder Timer
                if (m_uiFlamingCinderTimer <= uiDiff)
                {
                    if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(pTarget, SPELL_FLAMING_CINDER_FIRE, true);

                    m_uiFlamingCinderTimer = urand(10000, 15000);
                }
                else
                m_uiFlamingCinderTimer -= uiDiff;

            }
            else if (m_uiPhase == 1)                            //Phase timer
            {
                //Frenzy Timer
                if (m_uiFrenzyTimer <= uiDiff)
                {
                    ++m_uiPhase;
                    me->Yell(FIRE_SAY_FRENZY, LANG_UNIVERSAL, 0);
                    DoCast(me, SPELL_FRENZY);
                    m_uiFrenzyTimer = urand(30000, 50000);
                }
                else
                m_uiFrenzyTimer -= uiDiff;

                //Sunbeam Timer
                if (m_uiSunbeamTimer <= uiDiff)
                {
                    DoCast(me->GetVictim(), SPELL_SUNBEAM_FIRE);

                    m_uiSunbeamTimer = urand(10000, 30000);
                }
                else
                m_uiSunbeamTimer -= uiDiff;

                //Scorch Timer
                if (m_uiScorchTimer <= uiDiff)
                {
                    DoCast(me->GetVictim(), SPELL_SCORCH_FIRE);
                    m_uiScorchTimer = urand(10000, 35000);
                }
                else
                m_uiScorchTimer -= uiDiff;
            }
            
            //Summon Timer
            if (m_uiSummonTimer <= uiDiff)
            {
                me->Yell(FIRE_SAY_SUMMON_TITAN, LANG_TITAN, 0);
                me->SummonCreature(NPC_FIRE_ELEMENTAL, me->GetPositionX()+2, me->GetPositionY()+2, me->GetPositionZ()+1, 0, TEMPSUMMON_TIMED_DESPAWN, 50000);
                m_uiSummonTimer = 60000;
                m_uiSummonCheck = 50000;
            }
            else
            m_uiSummonTimer -= uiDiff;
            
            //Summon Heal
            if (m_uiSummonCheck <= uiDiff)
            {
                if (Creature *pElemental = GetClosestCreatureWithEntry(me, NPC_FIRE_ELEMENTAL, INTERACTION_DISTANCE*100))
                {
                    for (uint8 i = 0; i < 4; ++i)
                    me->CastSpell(me, 71783, true);
                    me->CastSpell(me, SPELL_FURY, true);
                }

                m_uiSummonCheck = 1000000;    //Hack
            }
            else
            m_uiSummonCheck -= uiDiff;

            //Burning Bite Timer
            if (m_uiBurningBiteTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                me->CastSpell(pTarget, SPELL_BURNING_BITE_FIRE, true);

                m_uiBurningBiteTimer = urand(12000, 25000);
            }
            else
            m_uiBurningBiteTimer -= uiDiff;

            //Fury Timer
            if (m_uiFuryTimer <= uiDiff)
            {
                me->CastSpell(me, SPELL_FURY, true);

                m_uiFuryTimer = urand(25000, 40000);
            }
            else
            m_uiFuryTimer -= uiDiff;


            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new event_npc_firelordAI(creature);
    }

};

#define ICE_SAY_AGGRO                                "Я проморожу вас насквозь!"
#define ICE_SAY_KILL                                 "Ещё одна бесполезная ледышка!"
#define ICE_SAY_DIE                                  "Но! Я же был бессмертен..."

#define EMOTE_ICE_SHIELD_ICE                         "Кожа Стража льда Алкида покрывается коркой льда"

class event_npc_icelord : public CreatureScript
{
public:

    event_npc_icelord()
    : CreatureScript("event_npc_icelord")
    {
    }

    struct event_npc_icelordAI : public ScriptedAI
    {
        event_npc_icelordAI(Creature *c) : ScriptedAI(c) {}


        uint32 m_uiIceBoltTimer;
        uint32 m_uiIceChainsTimer;
        uint32 m_uiFrostBlastTimer;
        uint32 m_uiFrostShieldDuration;
        uint32 m_uiFrostShieldCooldown;
        uint32 m_uiFrostBlast;
        uint32 m_uiIceBlastTimer;

        void Reset() override
        {
            m_uiIceBoltTimer          = urand(5000, 20000);
            m_uiIceChainsTimer        = urand(10000,25000);
            m_uiFrostBlastTimer       = urand(10000,20000);
            m_uiFrostShieldCooldown   = urand(20000,22000);
            m_uiFrostBlast            = urand(40000,80000);
            m_uiIceBlastTimer         = urand(20000,40000);
            me->SetReactState(REACT_DEFENSIVE);
        }

        void JustSummoned(Creature *summon) override
        {
        }

        void EnterCombat(Unit* pWho) override
        {
            me->Yell(ICE_SAY_AGGRO, LANG_UNIVERSAL, 0);
        }
                
        void KilledUnit(Unit *victim) override
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
            me->Yell(ICE_SAY_KILL, LANG_UNIVERSAL, 0);
        }
        
        void JustDied(Unit* /*killer*/) override
        {
            if (GameObject *pDoor1 = GetClosestGameObjectWithEntry(me, GO_ICE_DOOR_1, INTERACTION_DISTANCE*1000))
            if (pDoor1->GetGoState() != GO_STATE_ACTIVE)
            pDoor1->UseDoorOrButton();
            if (GameObject *pDoor2 = GetClosestGameObjectWithEntry(me, GO_ICE_DOOR_2, INTERACTION_DISTANCE*1000))
            if (pDoor2->GetGoState() != GO_STATE_ACTIVE)
            pDoor2->UseDoorOrButton();
            me->Yell(ICE_SAY_DIE, LANG_UNIVERSAL, 0);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
            return;
            
            if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

            if (!me->GetVictim())
            {
                me->CombatStop(true);
                EnterEvadeMode();
            }

            //Icebolt Timer
            if (m_uiIceBoltTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                me->CastSpell(pTarget, SPELL_ICEBOLT_ICE, true);

                m_uiIceBoltTimer = urand(5000, 15000);
            }
            else
            m_uiIceBoltTimer -= uiDiff;

            //IceBlast Timer
            if (m_uiIceBlastTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                me->CastSpell(pTarget, SPELL_ICEBLAST_ICE, true);

                m_uiIceBlastTimer = urand(20000,40000);
            }
            else
            m_uiIceBlastTimer -= uiDiff;

            //Ice Chains Timer
            if (m_uiIceChainsTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                DoCast(pTarget, SPELL_ICECHAINS_ICE);

                m_uiIceChainsTimer = urand(10000, 17000);
            }
            else
            m_uiIceChainsTimer -= uiDiff;

            //Frost Blast (AOE) Timer
            if (m_uiFrostBlast <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                DoCast(pTarget, SPELL_FROST_BLAST_1_ICE);

                m_uiFrostBlast = urand(40000,80000);
            }
            else
            m_uiFrostBlast -= uiDiff;

            //Frost Blast Timer
            if (m_uiFrostBlastTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                DoCast(pTarget, SPELL_FROST_BLAST_ICE);

                m_uiFrostBlastTimer = urand(10000,20000);
            }
            else
            m_uiFrostBlastTimer -= uiDiff;

            //Ice Shield duration
            if (m_uiFrostShieldDuration <= uiDiff && me->HasAura(SPELL_FROST_SHIELD_ICE))
            {
                me->RemoveAurasDueToSpell(SPELL_FROST_SHIELD_ICE);
            }
            else
            m_uiFrostShieldDuration -= uiDiff;

            //Ice Shield cooldown
            if (m_uiFrostShieldCooldown <= uiDiff)
            {
                DoCast(me, SPELL_FROST_SHIELD_ICE);
                me->TextEmote(EMOTE_ICE_SHIELD_ICE, 0);
                m_uiFrostShieldCooldown = urand(20000, 22000);
                m_uiFrostShieldDuration = urand(10000, 15000);
            }
            else
            m_uiFrostShieldCooldown -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new event_npc_icelordAI(creature);
    }

};

#define EARTH_SAY_AGGRO                               "Вам не следовало сюда приходить! Это место станет вашей могилой!"
#define EARTH_EMOTE_NATURE                            "Стража замли Акрилия наполняют силы земли"
#define EARTH_SAY_KILL                                "Отправляйся в землю!"
#define EARTH_SAY_DIE                                 "Мать земля, помо... ох..."

class event_npc_earthlord : public CreatureScript
{
public:

    event_npc_earthlord()
    : CreatureScript("event_npc_earthlord")
    {
    }

    struct event_npc_earthlordAI : public ScriptedAI
    {
        event_npc_earthlordAI(Creature *c) : ScriptedAI(c) {}

        uint32 m_uiNatureTimer;
        uint32 m_uiGroundTimer;
        uint32 m_uiLifebloomTimer;
        uint32 m_uiWrathTimer;
        uint32 m_uiStunTimer;
        uint32 m_uiPoisonNovaTimer;
        uint32 m_uiFuryTimer;
        uint32 m_uiPetrifyingBreathTimer;

        void Reset() override
        {
            m_uiNatureTimer           = urand(20000, 30000);
            m_uiGroundTimer           = urand(10000, 30000);
            m_uiLifebloomTimer        = urand(10000, 30000);
            m_uiWrathTimer            = urand(5000,  10000);
            m_uiStunTimer             = urand(10000, 20000);
            m_uiPoisonNovaTimer       = urand(5000,  20000);
            m_uiFuryTimer             = urand(25000, 40000);
            m_uiPetrifyingBreathTimer = urand(10000, 20000);
            me->SetReactState(REACT_DEFENSIVE);
        }

        void JustSummoned(Creature *summon) override
        {
        }

        void EnterCombat(Unit* pWho) override
        {
            me->Yell(EARTH_SAY_AGGRO, LANG_UNIVERSAL, 0);
            me->RemoveAurasDueToSpell(SPELL_FURY);
        }

        void KilledUnit(Unit *victim) override
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
            me->Yell(EARTH_SAY_KILL, LANG_UNIVERSAL, 0);
        }
        
        void JustDied(Unit* /*killer*/) override
        {
            me->Yell(EARTH_SAY_DIE, LANG_UNIVERSAL, 0);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
            return;
            
            if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

            if (!me->GetVictim())
            {
                me->CombatStop(true);
                EnterEvadeMode();
            }
            
            //Attuned to Nature Timer
            if (m_uiNatureTimer <= uiDiff)
            {
                me->CastSpell(me, SPELL_NATURE_EARTH, true);
                me->TextEmote(EARTH_EMOTE_NATURE, 0);

                m_uiNatureTimer = urand(20000, 30000);
            }
            else
            m_uiNatureTimer -= uiDiff;
            
            //Fury Timer
            if (m_uiFuryTimer <= uiDiff)
            {
                me->CastSpell(me, SPELL_FURY, true);

                m_uiFuryTimer = urand(25000, 40000);
            }
            else
            m_uiFuryTimer -= uiDiff;

            //Ground Tremor Timer
            if (m_uiGroundTimer <= uiDiff)
            {
                me->CastSpell(me, SPELL_GROUND_EARTH, true);

                m_uiGroundTimer = urand(15000, 30000);
            }
            else
            m_uiGroundTimer -= uiDiff;

            //Poison Nova Timer
            if (m_uiPoisonNovaTimer <= uiDiff)
            {
                me->CastSpell(me, SPELL_POISON_NOVA_EARTH, true);

                m_uiPoisonNovaTimer = urand(5000, 20000);
            }
            else
            m_uiPoisonNovaTimer -= uiDiff;

            //Lifebloom Timer
            if (m_uiLifebloomTimer <= uiDiff)
            {
                for (uint8 i = 0; i < 4; ++i)
                me->CastSpell(me, SPELL_LIFEBLOOM_EARTH, true);

                m_uiLifebloomTimer = urand(10000, 30000);
            }
            else
            m_uiLifebloomTimer -= uiDiff;

            //Wrath Timer
            if (m_uiWrathTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                me->CastSpell(pTarget, SPELL_WRATH_EARTH, true);

                m_uiWrathTimer = urand(5000,  10000);
            }
            else
            m_uiWrathTimer -= uiDiff;

            //Petrifying Breath Timer
            if (m_uiPetrifyingBreathTimer <= uiDiff)
            {
                me->CastSpell(me->GetVictim(), SPELL_PETRIFYING_BREATH_EARTH, true);

                m_uiPetrifyingBreathTimer = urand(10000,  20000);
            }
            else
            m_uiPetrifyingBreathTimer -= uiDiff;
            
            //Stun Timer
            if (m_uiStunTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                me->CastSpell(pTarget, SPELL_STUN_EARTH, true);

                m_uiStunTimer = urand(10000, 20000);
            }
            else
            m_uiStunTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new event_npc_earthlordAI(creature);
    }

};

#define DARK_SAY_AGGRO                                "Гости? Неожиданно..."
#define DARK_SAY_FRENZY                               "О да..! Я чувствую как ваша ярость наполняет меня!"
#define DARK_SAY_SUMMON                               "Посмотрите на души тех, кто был здесь до вас."
#define DARK_SAY_KILL                                 "Твоя душа пополнит мою армию!"
#define DARK_SAY_DIE                                  "Я слишком стар для таких битв..."

class event_npc_darklord : public CreatureScript
{
public:

    event_npc_darklord()
    : CreatureScript("event_npc_darklord")
    {
    }

    struct event_npc_darklordAI : public ScriptedAI
    {
        event_npc_darklordAI(Creature *c) : ScriptedAI(c) {}

        uint32 m_uiRandom1Timer;
        uint32 m_uiRandom2Timer;
        uint32 m_uiRandom3Timer;
        uint32 m_uiSummonSoulsTimer;
        uint32 m_uiMirroredTimer;
        uint32 m_uiDecayTimer;
        uint32 m_uiVolleyTimer;
        uint32 m_uiFrenzy;
        uint32 m_uiFuryTimer;
        uint32 dPhase;

        void Reset() override
        {
            m_uiRandom1Timer        = urand(10000,12000);
            m_uiRandom2Timer        = urand(7000, 12000);
            m_uiRandom3Timer        = urand(11000,15000);
            m_uiFrenzy              = urand(40000,55000);
            m_uiSummonSoulsTimer    = urand(12000,20000);
            m_uiMirroredTimer       = urand(20000,27000);
            m_uiDecayTimer          = urand(15000,45000);
            m_uiVolleyTimer         = urand(15000,37000);
            m_uiFuryTimer           = urand(25000,40000);
            dPhase = 1;
            me->SetReactState(REACT_DEFENSIVE);
        }

        void JustSummoned(Creature *summon) override
        {
            summon->CastSpell(summon, SPELL_FRENZY, true);
            summon->CastSpell(summon, SPELL_FURY, true);
            if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
            summon->AI()->AttackStart(pTarget);
        }

        void EnterCombat(Unit* pWho) override
        {
            me->Yell(DARK_SAY_AGGRO, LANG_UNIVERSAL, 0);
            me->RemoveAurasDueToSpell(SPELL_FURY);
        }

        void KilledUnit(Unit *victim) override
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
            me->Yell(DARK_SAY_KILL, LANG_UNIVERSAL, 0);
        }

        void JustDied(Unit* /*killer*/) override
        {
            me->Yell(DARK_SAY_DIE, LANG_UNIVERSAL, 0);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
            return;
            
            if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

            if (!me->GetVictim())
            {
                me->CombatStop(true);
                EnterEvadeMode();
            }

            //Random 1 Timer
            if (m_uiRandom1Timer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                me->CastSpell(pTarget, urand(0, 1) ? SPELL_SUNBEAM_FIRE : SPELL_FROST_BLAST_ICE, true);

                m_uiRandom1Timer = urand(10000, 30000);
            }
            else
            m_uiRandom1Timer -= uiDiff;

            //Random 2 Timer
            if (m_uiRandom2Timer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                me->CastSpell(pTarget, urand(0, 1) ? SPELL_STUN_EARTH : SPELL_GROUND_EARTH, true);

                m_uiRandom2Timer = urand(7000, 12000);
            }
            else
            m_uiRandom2Timer -= uiDiff;

            //Random 3 Timer
            if (m_uiRandom3Timer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                me->CastSpell(pTarget, urand(0, 1) ? SPELL_SCORCH_FIRE : SPELL_ICECHAINS_ICE, true);

                m_uiRandom3Timer = urand(11000,15000);
            }
            else
            m_uiRandom3Timer -= uiDiff;

            //Mirrored Timer
            if (m_uiMirroredTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                me->CastSpell(pTarget, SPELL_MIRRORED_DARK, true);

                m_uiMirroredTimer = urand(20000,27000);
            }
            else
            m_uiMirroredTimer -= uiDiff;

            //Death and Decay Timer
            if (m_uiDecayTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                me->CastSpell(pTarget, SPELL_DEATH_AND_DECAY_DARK, true);

                m_uiDecayTimer = urand(15000,45000);
            }
            else
            m_uiDecayTimer -= uiDiff;

            //Dark Volley Timer
            if (m_uiVolleyTimer <= uiDiff)
            {
                me->CastSpell(me, SPELL_DARK_VOLLEY_DARK, true);

                m_uiVolleyTimer = urand(15000,45000);
            }
            else
            m_uiVolleyTimer -= uiDiff;

            //Summon Timer
            if (m_uiSummonSoulsTimer <= uiDiff)
            {
                for (uint8 n = 0; n < dPhase*4; ++n)
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                me->CastSpell(pTarget, SPELL_SOULS_SUMMON_DARK, true);

                me->Yell(DARK_SAY_SUMMON, LANG_UNIVERSAL, 0);
                m_uiSummonSoulsTimer = urand(12000,20000);
            }
            else
            m_uiSummonSoulsTimer -= uiDiff;

            //Fury Timer
            if (m_uiFuryTimer <= uiDiff)
            {
                me->CastSpell(me, SPELL_FURY, true);

                m_uiFuryTimer = urand(25000, 40000);
            }
            else
            m_uiFuryTimer -= uiDiff;

            if (dPhase == 1)
            {
                //Frenzy & Phase Timer
                if (m_uiFrenzy <= uiDiff)
                {
                    ++dPhase;
                    me->Yell(DARK_SAY_FRENZY, LANG_UNIVERSAL, 0);
                    me->CastSpell(me, SPELL_FRENZY, true);
                    me->CastSpell(me, SPELL_SOULS_SMALL_DARK, true);
                    m_uiFrenzy = urand(40000,55000);
                }
                else
                m_uiFrenzy -= uiDiff;

            } else if (dPhase < 5 && dPhase !=1) {
                //Frenzy & Phase Timer
                if (m_uiFrenzy <= uiDiff)
                {
                    ++dPhase;
                    me->Yell(DARK_SAY_FRENZY, LANG_UNIVERSAL, 0);
                    me->CastSpell(me, SPELL_FRENZY, true);
                    m_uiFrenzy = urand(40000,50000);
                }
                else
                m_uiFrenzy -= uiDiff;

            } else if (dPhase == 5) {
                //Frenzy & Phase Timer
                if (m_uiFrenzy <= uiDiff)
                {
                    ++dPhase;
                    me->Yell(DARK_SAY_FRENZY, LANG_UNIVERSAL, 0);
                    me->CastSpell(me, SPELL_FRENZY, true);
                    me->CastSpell(me, SPELL_SOULS_LARGE_DARK, true);
                    m_uiFrenzy = urand(40000,55000);
                }
                else
                m_uiFrenzy -= uiDiff;
            } else if (dPhase > 5) {
                //Frenzy & Phase Timer
                if (m_uiFrenzy <= uiDiff)
                {
                    ++dPhase;
                    me->Yell(DARK_SAY_FRENZY, LANG_UNIVERSAL, 0);
                    me->CastSpell(me, SPELL_FRENZY, true);
                    m_uiFrenzy = urand(35000,45000);
                }
                else
                m_uiFrenzy -= uiDiff;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new event_npc_darklordAI(creature);
    }

};

class event_mage_ice : public CreatureScript
{
public:

    event_mage_ice()
    : CreatureScript("event_mage_ice")
    {
    }

    struct event_mage_iceAI : public ScriptedAI
    {
        event_mage_iceAI(Creature *c) : ScriptedAI(c) {}

        uint32 m_uiBlizzardTimer;
        uint32 m_uiConeTimer;
        uint32 m_uiCounterspellTimer;

        void Reset() override
        {
            m_uiBlizzardTimer      = urand(15000, 30000);
            m_uiConeTimer          = urand(10000, 15000);
            m_uiCounterspellTimer  = urand(10000, 20000);
        }

        void JustSummoned(Creature *summon) override
        {
        }

        void EnterCombat(Unit* pWho) override
        {
        }
                
        void KilledUnit(Unit *victim) override
        {
        }
        
        void JustDied(Unit* /*killer*/) override
        {
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
            return;
            
            if (me->HasUnitState(UNIT_STATE_CASTING))
            return;
            
            //Blizzard Timer
            if (m_uiBlizzardTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                DoCast(pTarget, SPELL_BLIZZARD);

                m_uiBlizzardTimer = urand(15000, 30000);
            }
            else
            m_uiBlizzardTimer -= uiDiff;

            //Cone of Cold Timer
            if (m_uiConeTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                DoCast(pTarget, SPELL_CONE_OF_COLD);

                m_uiConeTimer = urand(10000, 15000);
            }
            else
            m_uiConeTimer -= uiDiff;

            //Counterspell Timer
            if (m_uiCounterspellTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                DoCast(pTarget, SPELL_COUNTERSPELL);

                m_uiCounterspellTimer = urand(10000, 20000);
            }
            else
            m_uiCounterspellTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new event_mage_iceAI(creature);
    }

};

class event_mage_fire : public CreatureScript
{
public:

    event_mage_fire()
    : CreatureScript("event_mage_fire")
    {
    }

    struct event_mage_fireAI : public ScriptedAI
    {
        event_mage_fireAI(Creature *c) : ScriptedAI(c) {}

        uint32 m_uiHealTimer;
        uint32 m_uiFFireballTimer;
        uint32 m_uiFireNovaTimer;
        uint32 m_uiShipTimer;

        void Reset() override
        {
            m_uiHealTimer      = urand(15000, 20000);
            m_uiFFireballTimer = urand(10000, 15000);
            m_uiFireNovaTimer  = urand(10000, 20000);
            m_uiShipTimer      = urand(30000, 35000);
        }

        void JustSummoned(Creature *summon) override
        {
        }

        void EnterCombat(Unit* pWho) override
        {
        }
        
        void KilledUnit(Unit *victim) override
        {
        }
        
        void JustDied(Unit* /*killer*/) override
        {
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
            return;
            
            if (me->HasUnitState(UNIT_STATE_CASTING))
            return;
            
            //Chain heal Timer
            if (m_uiHealTimer <= uiDiff)
            {
                if (Unit *pAlly = DoSelectLowestHpFriendly(INTERACTION_DISTANCE*100))
                DoCast(pAlly, SPELL_HEAL);

                m_uiHealTimer = urand(15000, 20000);
            }
            else
            m_uiHealTimer -= uiDiff;

            //Fel Fireball Timer
            if (m_uiFFireballTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                DoCast(pTarget, SPELL_FEL_FIREBALL);

                m_uiFFireballTimer = urand(10000, 15000);
            }
            else
            m_uiShipTimer -= uiDiff;
            
            // polymorph				
            if (m_uiShipTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                DoCast(pTarget, SPELL_SHIP);

                m_uiShipTimer = urand(30000, 35000);
            }
            else
            m_uiShipTimer -= uiDiff;			

            //Fire Nova timer
            if (m_uiFireNovaTimer <= uiDiff)
            {
                DoCast(me, SPELL_FIRE_NOVA);

                m_uiFireNovaTimer = urand(8000,  15000);
            }
            else
            m_uiFireNovaTimer -= uiDiff;


            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new event_mage_fireAI(creature);
    }

};

#define PANDA_SAY_KILL                                 "Ну что ? Кто следущий ? Ахахахаха"
#define PANDA_SAY_DIE                                  "Неееет вы не могли сделать это со мной..."

class event_dk : public CreatureScript
{
public:

    event_dk()
    : CreatureScript("event_dk")
    {
    }

    struct event_dkAI : public ScriptedAI
    {
        event_dkAI(Creature *c) : ScriptedAI(c) {}
        uint32 m_uiFrostStrikeTimer;
        uint32 m_uiDeathCoilTimer;
        uint32 m_uiGlacialStrikeTimer;
        uint32 m_uiTapTimer;
        void Reset() override
        {
            m_uiFrostStrikeTimer      = urand(6000, 10000);
            m_uiDeathCoilTimer        = urand(5000, 15000);
            m_uiGlacialStrikeTimer    = urand(10000,17000);
            m_uiTapTimer              = urand(12000,17000);
        }
        void JustSummoned(Creature *summon) override
        {
        }

        void EnterCombat(Unit* pWho) override
        {
        }
                
        void KilledUnit(Unit *victim) override
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
            me->Yell(PANDA_SAY_KILL, LANG_UNIVERSAL, 0);
        }

        void JustDied(Unit* /*killer*/) override
        {
            me->Yell(PANDA_SAY_DIE, LANG_UNIVERSAL, 0);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
            return;
            
            if (me->HasUnitState(UNIT_STATE_CASTING))
            return;
            
            //Death Coil Timer
            if (m_uiDeathCoilTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                DoCast(pTarget, SPELL_DEATH_COIL);

                m_uiDeathCoilTimer = urand(5000, 15000);
            }
            else
            m_uiDeathCoilTimer -= uiDiff;
            //Frost Strike Timer
            if (m_uiFrostStrikeTimer <= uiDiff)
            {
                DoCast(me->GetVictim(), SPELL_FROST_STRIKE);
                m_uiFrostStrikeTimer = urand(7000, 10000);
            }
            else
            m_uiFrostStrikeTimer -= uiDiff;

            //Glacial Strike Timer
            if (m_uiGlacialStrikeTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                DoCast(pTarget, SPELL_GLACIAL_STRIKE);
                m_uiGlacialStrikeTimer = urand(7000, 17000);
            }
            else
            m_uiGlacialStrikeTimer -= uiDiff;
            
            // Rune Tap Timer
            if (m_uiTapTimer <= uiDiff && !HealthAbovePct(35))
            {
                me->CastSpell(me, SPELL_ANTIMAGIC_ZONE, true);
                m_uiTapTimer = urand(12000, 17000);
            }
            else
            m_uiTapTimer -= uiDiff;
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new event_dkAI(creature);
    }

};

class event_warrior : public CreatureScript
{
public:

    event_warrior()
    : CreatureScript("event_warrior")
    {
    }
    struct event_warriorAI : public ScriptedAI
    {
        event_warriorAI(Creature *c) : ScriptedAI(c) {}

        uint32 m_uiShockwaveTimer;
        uint32 m_uiBloodthristTimer;
        uint32 m_uiDisarmTimer;

        void Reset() override
        {
            m_uiShockwaveTimer      = urand(8000,12000);
            m_uiBloodthristTimer    = urand(5000, 9000);
            m_uiDisarmTimer         = urand(8000,20000);
        }
        void JustSummoned(Creature *summon) override
        {
        }
        void EnterCombat(Unit* pWho) override
        {
        }
        void KilledUnit(Unit *victim) override
        {
        }        
        void JustDied(Unit* /*killer*/) override
        {
        }
        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
            return;
            if (me->HasUnitState(UNIT_STATE_CASTING))
            return;
            
            //Shockwave Timer
            if (m_uiShockwaveTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                DoCast(pTarget, SPELL_SHOCKWAVE);
                m_uiShockwaveTimer = urand(8000, 10000);
            }
            else
            m_uiShockwaveTimer -= uiDiff;

            //Bloddthrist Timer
            if (m_uiBloodthristTimer <= uiDiff)
            {
                DoCast(me->GetVictim(), SPELL_BLOODTHRIST);
                m_uiBloodthristTimer = urand(5000, 9000);
            }
            else
            m_uiBloodthristTimer -= uiDiff;
            if (m_uiDisarmTimer <= uiDiff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                DoCast(pTarget, SPELL_DISARM);

                m_uiDisarmTimer = urand(8000, 10000);
            }
            else
            m_uiDisarmTimer -= uiDiff;
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new event_warriorAI(creature);
    }

};

class IceMan : public CreatureScript 
{ 
public: 
    IceMan() : CreatureScript("IceMan"){} 
    
    CreatureAI* GetAI(Creature* pCreature) const override
    { 
        return new IceManAI(pCreature); 
    } 
    
    struct IceManAI : public ScriptedAI 
    { 
        IceManAI(Creature *c) : ScriptedAI(c) {}   
        uint32 Knockback_timer; 
        uint32 Flame_timer; 
        uint32 Meteor_timer; 

        void Reset()  override
        { 
            Knockback_timer = 40000; 
            Flame_timer = 10000; 
            Meteor_timer = 3500; 
        }        
        void JustDied(Unit * victim) override
        { 
            me->Yell("НЕЕЕЕЕЕЕЕЕЕТ!!!",LANG_UNIVERSAL, 0); 
        }         
        void KilledUnit(Unit * victim)  override
        { 
            me->Yell("Уверены?!",LANG_UNIVERSAL, 0);    
        }         
        void EnterCombat(Unit * /*who*/)  override
        { 
            me->Yell("Вы думаете, что вы можете убить меня?!",LANG_UNIVERSAL, 0); 
        } 
        void UpdateAI(uint32 uiDiff)  override
        { 
            if(!UpdateVictim()) 
            return; 
            if(!UpdateVictim()) 
            return; 
            if (Knockback_timer <= uiDiff) 
            { 
                DoCast(me->GetVictim(), KNOCKBACK);  
                Knockback_timer = 20000;  
            } 
            else 
            Knockback_timer -= uiDiff;        
            if (!UpdateVictim()) 
            return; 

            if (Flame_timer <= uiDiff) 
            { 
                DoCast(me->GetVictim(), FLAME); 
                Flame_timer = 15000;  
            } 
            else 
            Flame_timer -= uiDiff; 

            if(!UpdateVictim()) 
            return; 

            if (Meteor_timer <= uiDiff) 
            { 
                DoCast(me->GetVictim(), METEOR);  
                Meteor_timer = 20000;  
            } 
            else 
            Meteor_timer -=uiDiff; 

            DoMeleeAttackIfReady(); 
        } 
    }; 
}; 

/*#####################*\
## boss_immortal_souls ##
\*#####################*/

enum ImmortalSoulsSpells
{
    // Печаль (Sadness)
    SPELL_SHADOW_BOLT = 70043, // Стрела Тьмы
    SPELL_SOUL_FLAY = 45442, // Свежевание души
    SPELL_SHADOW_WORD_PAIN = 72319, // Слово Тьмы: Боль
    SPELL_FINGER_OF_DEATH = 31984, // Палец смерти
    SPELL_BLACK_PLAGUE = 64153, // Черная чума
    SPELL_HARVEST_SOUL = 74325, // Жатва душ
    // Тоска (Anguish)
    SPELL_SHROUD_OF_THE_OCCULT = 70768, // Оккультная защита 
    SPELL_PAIN_AND_SUFFERING = 73790, // Боль и страдание
    SPELL_SOUL_SICKNESS = 69131, // Боль души
    SPELL_WAIL_OF_SOULS = 70210, // Плач душ
    SPELL_SOUL_SHRIEK = 73802, // Визг души
    SPELL_STOP_TIME = 60077, // Остановка времени
    // Скорбь (Affliction)
    SPELL_DARK_VOLLEY = 63038, // Темный залп
    SPELL_DEADEN = 41410, // Нечувствительность
    SPELL_SHADOW_BLAST = 70866, // Теневой разряд
    SPELL_SHADOW_SPIKE = 46589, // Теневой шип
    SPELL_AURA_OF_DESIRE = 41350, // Аура желания
    SPELL_CLONE_PLAYER = 57507, // Копия игрока
    SPELL_CLONE_WEAPON = 41054, // Копия оружия
    // Грусть (Sorrow)
    SPELL_DRAIN_LIFE = 70213, // Похищение жизни
    SPELL_RUNE_SHIELD = 41431, // Рунический щит
    SPELL_SHADOW_LANCE = 72806, // Теневое копье
    SPELL_AURA_OF_SUFFERING = 42017, // Аура страдания
    SPELL_SINFUL_BEAM = 40827, // Греховный луч
    SPELL_FEAR = 65809, // Страх
    SPELL_SOULSTORM = 68872,  // Буря душ
    SPELL_SOULSTORM_CHANNEL = 69008,
    SPELL_SOULSTORM_VISUAL = 68870,
    // Ауры (Auras)
    SPELL_SHADOWFORM                         = 16592, // Облик Тьмы
    SPELL_SHADOW_CRASH                       = 62660, // Темное сокрушение
    SPELL_REFLECTION_FIELD                   = 10831, // Отражающее поле
    SPELL_SHADOW_FISSURE                     = 59127, // Расщелина тьмы
    SPELL_SOUL_FRAGMENT                      = 71905, // Фрагмент души
    SPELL_SOUL_FRAGMENT_TIMER                = 6000, // Таймер (Фрагмент души)
    SPELL_GHOSTLY                            = 16713, // Призрачность
    //SPELL_CORRUPTION_OF_TIME                 = 60451, // Проклятие времени
    //SPELL_AURA_OF_DARKNESS                   = 71111, // Аура Тьмы
    //SPELL_SHROUD_OF_SORROW                   = 72983, // Покров скорби 51019 
    SPELL_MENDING                            = 2147, // Лечение
    SPELL_ARCANE_BEAM_PERIODIC               = 51019, // Периодический луч тайной магии 
    SPELL_PERIODIC_MANA_BURN                 = 812, // Периодическое сожжение маны
    SPELL_DEATH_AND_DECAY                    = 71001, // Лужа
    SPELL_MORTAL_STRIKE                      = 65926, // Мортал
    
    // Визуальные эффекты (Visual Effects)
    SPELL_VISUAL_1                           = 72523, // Визуальный эффект №1
    SPELL_VISUAL_2                           = 46228, // Визуальный эффект №2
    SPELL_VISUAL_3                           = 60484 // Визуальный эффект №3
};

enum Events
{
    // Sadness
    EVENT_SHADOW_BOLT = 1,
    EVENT_SOUL_FLAY = 2,
    EVENT_SHADOW_WORD_PAIN = 3,
    EVENT_FINGER_OF_DEATH = 4,
    EVENT_BLACK_PLAGUE = 5,
    EVENT_HARVEST_SOUL = 6,
    // Anguish
    EVENT_SHROUD_OF_THE_OCCULT = 7,
    EVENT_PAIN_AND_SUFFERING = 8,
    EVENT_SOUL_SICKNESS = 9,
    EVENT_WAIL_OF_SOULS = 10,
    EVENT_SOUL_SHRIEK = 11,
    EVENT_STOP_TIME = 12,
    // Affliction
    EVENT_DARK_VOLLEY = 13,
    EVENT_DEADEN = 14,
    EVENT_SHADOW_BLAST = 15,
    EVENT_SHADOW_SPIKE = 16,
    EVENT_AURA_OF_DESIRE = 17,
    EVENT_SEVERED_ESSENCE = 18,
    // Sorrow
    EVENT_DRAIN_LIFE = 19,
    EVENT_RUNE_SHIELD = 20,
    EVENT_SHADOW_LANCE = 21,
    EVENT_AURA_OF_SUFFERING = 22,
    EVENT_SINFUL_BEAM = 23,
    EVENT_FEAR = 24,
    EVENT_SOULSTORM = 25,
    EVENT_REFLECTION_FIELD = 26,
    EVENT_SHADOW_FISSURE = 27,

    EVENT_SOUL_FRAGMENT = 28,
    EVENT_GHOSTLY = 29,

    EVENT_SADNESS = 32,
    EVENT_ANGUISH = 33,
    EVENT_AFFLICTION = 34,
    EVENT_SORROW = 35,
    EVENT_DEATH = 36,
    EVENT_DEATH_AND = 37,
    EVENT_DEATH_AND_DECAY = 38,
    EVENT_MORTAL_STRIKE = 39,
    EVENT_MORTAL = 40,
    EVENT_MORTAL_STRIK = 41
};

#define EMOTE_AGGRO_SADNESS "|TInterface/icons/spell_misc_emotionsad:26|t На вас нападает 'Печаль' |TInterface/icons/spell_misc_emotionsad:26|t"
#define EMOTE_AGGRO_ANGUISH "|TInterface/icons/spell_misc_emotionsad:26|t Появляется 'Тоска' |TInterface/icons/spell_misc_emotionsad:26|t"
#define EMOTE_AGGRO_AFFLICTION "|TInterface/icons/spell_misc_emotionsad:26|t Появляется 'Скорбь' |TInterface/icons/spell_misc_emotionsad:26|t"
#define EMOTE_AGGRO_SORROW "|TInterface/icons/spell_misc_emotionsad:26|t Появляется 'Грусть' |TInterface/icons/spell_misc_emotionsad:26|t"
#define EMOTE_SOULSTORM "|TInterface/icons/spell_nature_astralrecal:26|t Грусть применяет заклинание 'Буря душ' |TInterface/icons/spell_nature_astralrecal:26|t"

#define TEXT_AGGRO_SADNESS "Очередные безумцы! Мы ждали вас... Наконец-то вы познаете покой смерти..."
#define TEXT_AGGRO_ANGUISH "Мы - это сон наяву. Чудовища из ваших кошмаров. Мы есть тьма! Трепещите, ничтожные, перед посланницами смерти!"
#define TEXT_AGGRO_AFFLICTION "Вы боитесь того, чем не можете управлять? Можете ли вы управлять своим страхом?"
#define TEXT_AGGRO_SORROW "Вы, жалкие смертные! Вы не способны предотвратить предначертанное!"
#define TEXT_ON_SLAY "Придите же, о жалкие порождения жизни... Навеки обретите покой на этой земле..."
#define TEXT_ON_DEATH "Это еще не конец... Вы ничего не сможете сделать..."
#define TEXT_ON_SOULSTORM "Конец неминуем, смертные! Этот мир обречён на уничтожение!"
#define TEXT_ON_HARVEST_SOUL "Настал час расплаты... Ваши тела и души будут уничтожены..."
#define TEXT_ON_STOP_TIME "Задумайтесь о своей никчемности, пока есть время..."
#define TEXT_ON_SHADOW_BLAST "Ваша слабость делает нас сильнее..."
#define TEXT_ON_FEAR "Бегите в страхе от нас, жалкие смертные..."

Creature *severed_essence;

enum Phases
{
    SADNESS_PHASE       = 1,
    ANGUISH_PHASE       = 2,
    AFFLICTION_PHASE    = 3,
    SORROW_PHASE        = 4
};

uint32 MORPH_IMMORTAL_SOULS;

class boss_immortal_souls : public CreatureScript
{
public:
    boss_immortal_souls() : CreatureScript("boss_immortal_souls") { }


    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_immortal_soulsAI(creature);
    }

    struct boss_immortal_soulsAI : public ScriptedAI
    {
        boss_immortal_soulsAI(Creature* creature) : ScriptedAI(creature) { }

        void SetMorph(uint32 morph)
        {
            if (morph == MORPH_IMMORTAL_SOULS)
            return;
            MORPH_IMMORTAL_SOULS = morph;
            me->SetDisplayId(morph);
            return;
        }

        EventMap events;

        bool _sadness;
        bool _anguish;
        bool _affliction;
        bool _sorrow;
        bool _soulstormed;

        void Reset() override
        {
            events.Reset();
            _sadness = true;
            _anguish = false;
            _affliction = false;
            _sorrow = false;
            _soulstormed = false;
            events.SetPhase(SADNESS_PHASE);
            SetMorph(28191);
            me->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.8f);
            SetEquipmentSlots(false, 38304);
            me->RemoveAllAuras();
            me->SetReactState(REACT_AGGRESSIVE);
            me->AddAura(SPELL_GHOSTLY, me);
            me->AddAura(SPELL_VISUAL_1, me);
        }

        void EnterCombat(Unit* who) override
        {
            me->SetWalk(true);
            events.CancelEvent(EVENT_GHOSTLY);
            me->RemoveAura(SPELL_GHOSTLY);
            me->AddAura(SPELL_SHADOWFORM, me);
            events.SetPhase(SADNESS_PHASE);
            events.ScheduleEvent(EVENT_SOUL_FRAGMENT, 100);
            events.ScheduleEvent(EVENT_SADNESS, 100);
        }

        void DamageTaken(Unit* attacker, uint32& damage) override
        {
            if (!_anguish && !_affliction && !_sorrow && !HealthAbovePct(75))
            {
                _anguish = true;
                events.ScheduleEvent(EVENT_ANGUISH, 100);
            }

            if (!_affliction && !_sorrow && !HealthAbovePct(50))
            {
                _affliction = true;
                events.ScheduleEvent(EVENT_AFFLICTION, 100);
            }

            if (!_sorrow && !HealthAbovePct(25))
            {
                events.ScheduleEvent(EVENT_SORROW, 100);
                _sorrow = true;
            }

            if (!_soulstormed && !HealthAbovePct(15))
            {
                _soulstormed = true;
                events.ScheduleEvent(EVENT_SOULSTORM, 100);
            }
        }

        void JustSummoned(Creature* summon) override
        {
            switch (summon->GetEntry())
            {
            case 100021:
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                    {
                        summon->CastSpell(target, 69091, false);
                    }
                    else
                    summon->DespawnOrUnsummon();
                    break;
                }
            default:
                break;
            }
        }

        void KilledUnit(Unit* /*who*/) override
        {
            me->Yell(TEXT_ON_SLAY, LANG_UNIVERSAL, me);
        }

        void JustDied(Unit* /*killer*/) override
        {
            events.Reset();
            me->RemoveAllAuras();
            me->Yell(TEXT_ON_DEATH, LANG_UNIVERSAL, me);
        }

        void UpdateAI(uint32 diff) override
        {
            bool live = me && me->IsAlive();
            if (!live)
            {
                events.Reset();
                return;
            }

            if (me->GetAuraCount(SPELL_GHOSTLY) < 1)
            {
                if (!UpdateVictim()) { me->AddAura(SPELL_GHOSTLY, me); }
            }

            if (me->GetAuraCount(SPELL_SOULSTORM_CHANNEL) < 1)
            {
                if (!UpdateVictim()) 
                    me->CastSpell(me, SPELL_SOULSTORM_CHANNEL, live); 
            }

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SOUL_FRAGMENT:
                    if (me->GetAuraCount(SPELL_SOUL_FRAGMENT) < 9)
                    {
                        events.CancelEvent(EVENT_SOUL_FRAGMENT);
                        DoCast(me, SPELL_SOUL_FRAGMENT);
                    }
                    events.ScheduleEvent(EVENT_SOUL_FRAGMENT, SPELL_SOUL_FRAGMENT_TIMER);
                    break;

                case EVENT_SHADOW_BOLT: // Случайная цель
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    DoCast(target, SPELL_SHADOW_BOLT, true);
                    events.ScheduleEvent(EVENT_SHADOW_BOLT, urand(15400, 16100), 0, SADNESS_PHASE);
                    break;
                    
                case EVENT_DEATH:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    DoCast(target, SPELL_DEATH_AND_DECAY, true);
                    events.ScheduleEvent(EVENT_DEATH, urand(200, 54100), 0, SADNESS_PHASE);
                    break;

                case EVENT_MORTAL:
                    DoCast(me->GetVictim(), SPELL_MORTAL_STRIKE, true);
                    events.ScheduleEvent(EVENT_MORTAL, urand(20, 22900), 0, SADNESS_PHASE);
                    break;
                    
                case EVENT_SOUL_FLAY:
                    DoCast(me->GetVictim(), SPELL_SOUL_FLAY, true);
                    events.ScheduleEvent(EVENT_SOUL_FLAY, urand(22000, 22900), 0, SADNESS_PHASE);
                    break;

                case EVENT_SHADOW_WORD_PAIN: // Случайная цель
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    DoCast(target, SPELL_SHADOW_WORD_PAIN, true);
                    events.ScheduleEvent(EVENT_SHADOW_WORD_PAIN, urand(13400, 14200), 0, SADNESS_PHASE);
                    break;

                case EVENT_FINGER_OF_DEATH:
                    DoCast(me->GetVictim(), SPELL_FINGER_OF_DEATH);
                    events.ScheduleEvent(EVENT_FINGER_OF_DEATH, urand(17000, 17700), 0, SADNESS_PHASE);
                    break;

                case EVENT_BLACK_PLAGUE: // АОЕ
                    DoCastAOE(SPELL_BLACK_PLAGUE);
                    events.ScheduleEvent(EVENT_BLACK_PLAGUE, urand(35400, 36300), 0, SADNESS_PHASE);
                    break;

                case EVENT_HARVEST_SOUL:
                    DoCast(me->GetVictim(), SPELL_HARVEST_SOUL);
                    me->Yell(TEXT_ON_HARVEST_SOUL, LANG_UNIVERSAL, me);
                    events.ScheduleEvent(EVENT_HARVEST_SOUL, urand(41200, 43100), 0, SADNESS_PHASE);
                    break;

                case EVENT_SHROUD_OF_THE_OCCULT:
                    me->AddAura(SPELL_SHROUD_OF_THE_OCCULT, me);
                    events.ScheduleEvent(EVENT_SHROUD_OF_THE_OCCULT, 80000, 0, ANGUISH_PHASE);
                    break;

                case EVENT_PAIN_AND_SUFFERING: // Случайная цель
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    DoCast(target, SPELL_PAIN_AND_SUFFERING);
                    events.ScheduleEvent(EVENT_PAIN_AND_SUFFERING, urand(15600, 16200), 0, ANGUISH_PHASE);
                    break;

                case EVENT_SOUL_SICKNESS:
                    DoCast(me->GetVictim(), SPELL_SOUL_SICKNESS);
                    events.ScheduleEvent(EVENT_SOUL_SICKNESS, urand(6200, 6800), 0, ANGUISH_PHASE);
                    break;

                case EVENT_WAIL_OF_SOULS: // Случайная цель
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    DoCast(target, SPELL_WAIL_OF_SOULS);
                    events.ScheduleEvent(EVENT_WAIL_OF_SOULS, urand(11400, 12300), 0, ANGUISH_PHASE);
                    break;

                case EVENT_SOUL_SHRIEK: // АОЕ
                    DoCastAOE(SPELL_SOUL_SHRIEK);
                    events.ScheduleEvent(EVENT_SOUL_SHRIEK, urand(22400, 23300), 0, ANGUISH_PHASE);
                    break;

                case EVENT_STOP_TIME: // АОЕ
                    DoCastAOE(SPELL_STOP_TIME);
                    me->Yell(TEXT_ON_STOP_TIME, LANG_UNIVERSAL, me);
                    events.ScheduleEvent(EVENT_STOP_TIME, urand(48200, 54100), 0, ANGUISH_PHASE);
                    break;
                    
                case EVENT_DEATH_AND:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    DoCast(target, SPELL_DEATH_AND_DECAY, true);
                    events.ScheduleEvent(EVENT_DEATH_AND, urand(200, 54100), 0, ANGUISH_PHASE);
                    break;

                case EVENT_DARK_VOLLEY:
                    DoCast(me->GetVictim(), SPELL_DARK_VOLLEY);
                    events.ScheduleEvent(EVENT_DARK_VOLLEY, urand(16100, 16900), 0, AFFLICTION_PHASE);
                    break;

                case EVENT_DEADEN:
                    DoCast(me->GetVictim(), SPELL_DEADEN);
                    events.ScheduleEvent(EVENT_DEADEN, urand(9200, 9800), 0, AFFLICTION_PHASE);
                    break;

                case EVENT_SHADOW_BLAST: // АОЕ
                    DoCastAOE(SPELL_SHADOW_BLAST);
                    me->Yell(TEXT_ON_SHADOW_BLAST, LANG_UNIVERSAL, me);
                    events.ScheduleEvent(EVENT_SHADOW_BLAST, urand(49200, 55100), 0, AFFLICTION_PHASE);
                    break;

                case EVENT_SHADOW_SPIKE: // Случайная цель
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    DoCast(target, SPELL_SHADOW_SPIKE);
                    events.ScheduleEvent(EVENT_SHADOW_SPIKE, urand(25400, 27300), 0, AFFLICTION_PHASE);
                    break;

                case EVENT_MORTAL_STRIK:
                    DoCast(me->GetVictim(), SPELL_MORTAL_STRIKE , true);
                    events.ScheduleEvent(EVENT_MORTAL_STRIK, urand(20, 22900), 0, AFFLICTION_PHASE);
                    break;
                    
                case EVENT_SEVERED_ESSENCE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 20.0f, true))
                    {
                        severed_essence = me->SummonCreature(100021, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000);
                        target->CastSpell(severed_essence, SPELL_CLONE_PLAYER, true); // 63793
                        target->CastSpell(severed_essence, SPELL_CLONE_WEAPON, true);
                        me->CastSpell(severed_essence, 70499, true); // Visual
                    }
                    events.ScheduleEvent(EVENT_SEVERED_ESSENCE, 42000, 0, AFFLICTION_PHASE);
                    break;
                    
                case EVENT_MORTAL_STRIKE:
                    DoCast(me->GetVictim(), SPELL_MORTAL_STRIKE);
                    events.ScheduleEvent(EVENT_MORTAL_STRIKE, urand(100, 13800), 0, SORROW_PHASE);
                    break;
                    
                case EVENT_DRAIN_LIFE:
                    DoCast(me->GetVictim(), SPELL_DRAIN_LIFE);
                    events.ScheduleEvent(EVENT_DRAIN_LIFE, urand(12200, 13800), 0, SORROW_PHASE);
                    break;

                case EVENT_RUNE_SHIELD:
                    me->AddAura(SPELL_RUNE_SHIELD, me);
                    events.ScheduleEvent(EVENT_RUNE_SHIELD, 70000, 0, SORROW_PHASE);
                    break;

                case EVENT_SHADOW_LANCE: // Случайная цель
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    DoCast(target, SPELL_SHADOW_LANCE);
                    events.ScheduleEvent(EVENT_SHADOW_LANCE, urand(22400, 23300), 0, SORROW_PHASE);
                    break;

                case EVENT_DEATH_AND_DECAY:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    DoCast(target, SPELL_DEATH_AND_DECAY, true);
                    events.ScheduleEvent(EVENT_DEATH_AND_DECAY, urand(200, 54100), 0, SORROW_PHASE);
                    break;
                    
                case EVENT_SINFUL_BEAM: // Случайная цель
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    DoCast(target, SPELL_SINFUL_BEAM);
                    events.ScheduleEvent(EVENT_SINFUL_BEAM, urand(14400, 15300), 0, SORROW_PHASE);
                    break;

                case EVENT_FEAR: // АОЕ
                    DoCastAOE(SPELL_FEAR);
                    me->Yell(TEXT_ON_FEAR, LANG_UNIVERSAL, me);
                    events.ScheduleEvent(EVENT_FEAR, urand(49200, 55100), 0, SORROW_PHASE);
                    break;

                case EVENT_SOULSTORM:
                    me->GetMotionMaster()->MovePoint(0, Position(2932.348633f, -4794.806641f, 234.688721f));
                    me->Yell(TEXT_ON_SOULSTORM, LANG_UNIVERSAL, me);
                    me->RemoveAllAuras();
                    DoCastAOE(SPELL_SHADOW_CRASH);
                    me->CastSpell(me, SPELL_SOULSTORM_VISUAL, true);
                    me->CastSpell(me, SPELL_SOULSTORM, false);
                    me->CastSpell(me, SPELL_ARCANE_BEAM_PERIODIC, true);
                    me->CastSpell(me, SPELL_PERIODIC_MANA_BURN, true);
                    events.ScheduleEvent(EVENT_REFLECTION_FIELD, urand(10100, 10700), 0, SORROW_PHASE);
                    events.ScheduleEvent(EVENT_SHADOW_FISSURE, urand(52400, 63300), 0, SORROW_PHASE);
                    break;

                case EVENT_REFLECTION_FIELD:
                    me->AddAura(SPELL_REFLECTION_FIELD, me);
                    events.ScheduleEvent(EVENT_REFLECTION_FIELD, urand(10100, 10700), 0, SORROW_PHASE);
                    break;

                case EVENT_SHADOW_FISSURE: // Случайная цель
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    DoCast(target, SPELL_SHADOW_FISSURE);
                    events.ScheduleEvent(EVENT_SHADOW_FISSURE, urand(52400, 63300), 0, SORROW_PHASE);
                    break;

                    // Phase Sadness
                case EVENT_SADNESS:
                    events.SetPhase(SADNESS_PHASE);
                    me->Yell(TEXT_AGGRO_SADNESS, LANG_UNIVERSAL, me);
                    me->CastSpell(me, SPELL_MENDING, live);
                    events.ScheduleEvent(EVENT_MORTAL, urand(20, 22900), 0, SADNESS_PHASE);
                    events.ScheduleEvent(EVENT_DEATH, urand(200, 54100), 0, SADNESS_PHASE);
                    events.ScheduleEvent(EVENT_SHADOW_BOLT, urand(15400, 16100), 0, SADNESS_PHASE);
                    events.ScheduleEvent(EVENT_SOUL_FLAY, urand(22000, 22900), 0, SADNESS_PHASE);
                    events.ScheduleEvent(EVENT_SHADOW_WORD_PAIN, urand(13400, 14200), 0, SADNESS_PHASE);
                    events.ScheduleEvent(EVENT_FINGER_OF_DEATH, urand(17000, 17700), 0, SADNESS_PHASE);
                    events.ScheduleEvent(EVENT_BLACK_PLAGUE, urand(35400, 36300), 0, SADNESS_PHASE);
                    events.ScheduleEvent(EVENT_HARVEST_SOUL, urand(41200, 43100), 0, SADNESS_PHASE);
                    break;
                    // Phase Anguish
                case EVENT_ANGUISH:
                    events.SetPhase(ANGUISH_PHASE);
                    me->Yell(TEXT_AGGRO_ANGUISH, LANG_UNIVERSAL, me);
                    me->RemoveAura(SPELL_MENDING);
                    me->CastSpell(me, SPELL_VISUAL_3, live);
                    SetMorph(30242);
                    me->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.2f);
                    SetEquipmentSlots(false, 49709);
                    events.ScheduleEvent(EVENT_SHROUD_OF_THE_OCCULT, 60000, 0, ANGUISH_PHASE);
                    events.ScheduleEvent(EVENT_DEATH_AND, urand(200, 54100), 0, ANGUISH_PHASE);
                    events.ScheduleEvent(EVENT_PAIN_AND_SUFFERING, urand(15600, 16200), 0, ANGUISH_PHASE);
                    events.ScheduleEvent(EVENT_SOUL_SICKNESS, urand(6200, 6800), 0, ANGUISH_PHASE);
                    events.ScheduleEvent(EVENT_WAIL_OF_SOULS, urand(11400, 12300), 0, ANGUISH_PHASE);
                    events.ScheduleEvent(EVENT_SOUL_SHRIEK, urand(22400, 23300), 0, ANGUISH_PHASE);
                    events.ScheduleEvent(EVENT_STOP_TIME, urand(48200, 54100), 0, ANGUISH_PHASE);
                    break;
                    // Phase Affliction
                case EVENT_AFFLICTION:
                    events.SetPhase(AFFLICTION_PHASE);
                    me->Yell(TEXT_AGGRO_AFFLICTION, LANG_UNIVERSAL, me);
                    me->CastSpell(me, SPELL_AURA_OF_DESIRE, live);
                    me->CastSpell(me, SPELL_VISUAL_3, live);
                    SetMorph(18239);
                    me->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.5f);
                    SetEquipmentSlots(false, 49653);
                    events.ScheduleEvent(EVENT_MORTAL_STRIK, urand(20, 22900), 0, AFFLICTION_PHASE);
                    events.ScheduleEvent(EVENT_DARK_VOLLEY, urand(16100, 16900), 0, AFFLICTION_PHASE);
                    events.ScheduleEvent(EVENT_DEADEN, urand(9200, 9800), 0, AFFLICTION_PHASE);
                    events.ScheduleEvent(EVENT_SHADOW_BLAST, urand(49200, 55100), 0, AFFLICTION_PHASE);
                    events.ScheduleEvent(EVENT_SHADOW_SPIKE, urand(25400, 27300), 0, AFFLICTION_PHASE);
                    events.ScheduleEvent(EVENT_SEVERED_ESSENCE, 42000, 0, AFFLICTION_PHASE);
                    break;
                    // Phase Sorrow
                case EVENT_SORROW:
                    events.SetPhase(SORROW_PHASE);
                    me->Yell(TEXT_AGGRO_SORROW, LANG_UNIVERSAL, me);
                    me->RemoveAura(SPELL_AURA_OF_DESIRE);
                    me->CastSpell(me, SPELL_AURA_OF_SUFFERING, live);
                    me->CastSpell(me, SPELL_VISUAL_3, live);
                    SetMorph(30169);
                    me->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.2f);
                    SetEquipmentSlots(false, 49654);
                    events.ScheduleEvent(EVENT_DRAIN_LIFE, urand(12200, 13800), 0, SORROW_PHASE);
                    events.ScheduleEvent(EVENT_MORTAL_STRIKE, urand(100, 13800), 0, SORROW_PHASE);
                    events.ScheduleEvent(EVENT_RUNE_SHIELD, 70000, 0, SORROW_PHASE);
                    events.ScheduleEvent(EVENT_DEATH_AND_DECAY, urand(200, 54100), 0, SORROW_PHASE);
                    events.ScheduleEvent(EVENT_SHADOW_LANCE, urand(22400, 23300), 0, SORROW_PHASE);
                    events.ScheduleEvent(EVENT_SINFUL_BEAM, urand(14400, 15300), 0, SORROW_PHASE);
                    events.ScheduleEvent(EVENT_FEAR, urand(49200, 55100), 0, SORROW_PHASE);
                    break;
                default:
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class RevivePlayer_AllMaps : public PlayerScript
{
   public:
    RevivePlayer_AllMaps() : PlayerScript("RevivePlayer_AllMaps") { }
   
    void OnPVPKill(Player* killer, Player* killed)
    {		
		if (killed->GetMap()->IsBattlegroundOrArena())
            return;
		
		if (killed == killer)
			return;
				
		uint32 money = killed->GetHonorPoints();			
		uint32 money_del = money;
		money_del *= killer->getLevel() == killed->getLevel() ? 0.10 : killer->getLevel() > killed->getLevel() ? 0.05 : 0.15; 
        killed->ModifyHonorPoints(-int32(money_del));
		killer->ModifyHonorPoints(money_del);
        ChatHandler(killer->GetSession()).PSendSysMessage("Вы получили: |cffff5050%u|r очков чести за убийство игрока |cffff5050%s|r", money_del, killed->GetName().c_str());
        ChatHandler(killed->GetSession()).PSendSysMessage("Вы потеряли: |cffff5050%u|r очков чести, вас убил |cffff5050%s|r", money_del, killer->GetName().c_str());		
	}	   
};

void AddSC_custom_zone()
{
    new pvp_hijal();
    new world_pvp;
    new pvp_zone_hyjal_announces();
    new npc_pvp_zone_hyjal_vendor();
    new gob_event();
    new event_announce();
    new boss_immortal_souls();
    new event_npc_firelord;
    new event_npc_icelord;
    new event_npc_earthlord;
    new event_npc_darklord;
    new event_mage_ice;
    new event_mage_fire;
    new event_dk;
    new event_warrior;
    new IceMan();
    new RevivePlayer_AllMaps();
}