/*
* Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
* Copyright (C) 2016-2017 RustEmu <https://gitlab.com/healthstone/>
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
#include "ArenaTeamMgr.h"
#include "Common.h"
#include "DisableMgr.h"
#include "BattlegroundMgr.h"
#include "Battleground.h"
#include "ArenaTeam.h"
#include "Language.h"
#include "npc_solo_queue.h"
#include "BattlegroundQueue.h"
#include "Player.h"


class npc_solo_queue : public CreatureScript
{
public:
    npc_solo_queue() : CreatureScript("npc_solo_queue")
    {
        for (int i = 0; i < MAX_TALENT_CAT; ++i)
        cache2v2Queue[i] = 0;

        lastFetchQueueList = 0;
    }

    bool OnGossipHello(Player* player, Creature* me)
    {
        if(!player || !me)
        return true;

        if (sWorld->getBoolConfig(CONFIG_SOLO_2V2_ENABLE) == false)
        {
            ChatHandler(player->GetSession()).SendSysMessage("ERROR: Arena disabled!");
            return true;
        }

        fetchQueueList();
        std::stringstream infoQueue;
        infoQueue << "Solo 3vs3 Arena\n";
        infoQueue << "Queued Players: " << (cache2v2Queue[MELEE] + cache2v2Queue[RANGE] + cache2v2Queue[HEALER]);
        //infoQueue << "Queued Players: " << (cache2v2Queue[DPS] + cache2v2Queue[HEALER]);

        if (sWorld->getBoolConfig(CONFIG_SOLO_2V2_FILTER_TALENTS))
        {
            infoQueue << "\n\n";
            infoQueue << "Зарегестрированно дд: " << cache2v2Queue[MELEE] << " (Длинная очередь!)\n";
            infoQueue << "Зарегестрированно рдд: " << cache2v2Queue[RANGE] << " (Длинная очередь!)\n";
            infoQueue << "Зарегестрированно хилов: " << cache2v2Queue[HEALER] << " (Бонусные награды!)\n";
            //infoQueue << "Queued DPS: " << cache2v2Queue[DPS] << "\n";
            //infoQueue << "Queued Healers: " << cache2v2Queue[HEALER] << "\n";
        }

        if (player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_5v5) 
                || player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_2v2_SOLO))
        player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_TALK, "Покинуть арену", GOSSIP_SENDER_MAIN, 3, "Вы уверены?", 0, false);

        if (player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_5v5) == false && 
                player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_2v2_SOLO) == false)
        {
            //player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "Sign up 1v1 Skirmish\n", GOSSIP_SENDER_MAIN, 21);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "Зарегестрировать арену 3х3 соло\n", GOSSIP_SENDER_MAIN, 2);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "Зарегестрировать арену 1х1 соло\n", GOSSIP_SENDER_MAIN, 20);
            /*if (player->GetArenaTeamId(ArenaTeam::GetSlotByType(ARENA_TEAM_5v5)) == NULL)
                player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_MONEY_BAG, "Create new arena team | Создать новую команду арены", GOSSIP_SENDER_MAIN, 1, "Create arena team? | Создать команду арены?", sWorld->getIntConfig(CONFIG_SOLO_2V2_COSTS), false);
            else
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "3v3 Solo Queue\n", GOSSIP_SENDER_MAIN, 2);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "1v1 Queue\n", GOSSIP_SENDER_MAIN, 20);
            }*/
        }

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, infoQueue.str().c_str(), GOSSIP_SENDER_MAIN, 0);
        //player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, "Script Info", GOSSIP_SENDER_MAIN, 8);
        player->SEND_GOSSIP_MENU(70, me->GetGUID());
        return true;
    }



    bool OnGossipSelect(Player* player, Creature* me, uint32 /*uiSender*/, uint32 uiAction)
    {
        if(!player || !me)
        return true;

        player->PlayerTalkClass->ClearMenus();

        switch (uiAction)
        {
        case 2: // 2v2 Join Queue Arena (rated)
            {
                int8 slot = ARENA_TEAM_5v5;
                if (player->InBattlegroundQueue())
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("ERROR: You are in Arena Queue.");
                    player->CLOSE_GOSSIP_MENU();
                    return true;
                }
                if (sWorld->getIntConfig(CONFIG_SOLO_2V2_MIN_LEVEL) <= player->getLevel())
                {
                    if (player->GetMoney() >= sWorld->getIntConfig(CONFIG_SOLO_2V2_COSTS) && CreateArenateam(player, me))
                    player->ModifyMoney(sWorld->getIntConfig(CONFIG_SOLO_2V2_COSTS) * -1);
                }
                else if (!player->GetArenaTeamId(slot))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("ERROR: You need level %u to create an arena team.", sWorld->getIntConfig(CONFIG_SOLO_2V2_MIN_LEVEL));
                    player->CLOSE_GOSSIP_MENU();
                    return true;
                }

                // check Deserter debuff
                if (player->HasAura(26013) && 
                        (sWorld->getBoolConfig(CONFIG_SOLO_2V2_CAST_DESERTER_ON_AFK) || sWorld->getBoolConfig(CONFIG_SOLO_2V2_CAST_DESERTER_ON_LEAVE)))
                {
                    WorldPacket data;
                    sBattlegroundMgr->BuildGroupJoinedBattlegroundPacket(&data, ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS);
                    player->GetSession()->SendPacket(&data);
                }
                else
                if (ArenaCheckFullEquipAndTalents(player) 
                        && JoinQueueArena(player, me, true, ARENA_TYPE_2v2_SOLO) == false)
                ChatHandler(player->GetSession()).SendSysMessage("ERROR: Something went wrong while joining queue. Already in another queue? [Проверьте на мини-карте наличие регистрации на БГ или Арену, или попробуйте зайти в режим спектатора, и выйти из него]");
                
                player->CLOSE_GOSSIP_MENU();
                return true;
            }

        case 20: // 1v1 Join Queue Arena (rated)
            {
                int8 slot = ARENA_TEAM_5v5;
                if (player->InBattlegroundQueue())
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("ERROR: You are in Arena Queue.");
                    player->CLOSE_GOSSIP_MENU();
                    return true;
                }
                if (!player->GetArenaTeamId(slot) && sWorld->getIntConfig(CONFIG_SOLO_2V2_MIN_LEVEL) <= player->getLevel())
                {
                    if (player->GetMoney() >= sWorld->getIntConfig(CONFIG_SOLO_2V2_COSTS) && CreateArenateam(player, me))
                    player->ModifyMoney(sWorld->getIntConfig(CONFIG_SOLO_2V2_COSTS) * -1);
                }
                else if (!player->GetArenaTeamId(slot))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("ERROR: You need level %u to create an arena team.", sWorld->getIntConfig(CONFIG_SOLO_2V2_MIN_LEVEL));
                    player->CLOSE_GOSSIP_MENU();
                    return true;
                }
                if (ArenaCheckFullEquipAndTalents(player) && Arena1v1CheckTalents(player) 
                        && JoinQueueArena(player, me, true, ARENA_TYPE_5v5) == false)
                ChatHandler(player->GetSession()).SendSysMessage("ERROR: Something went wrong while joining queue. Already in another queue?");

                player->CLOSE_GOSSIP_MENU();
                return true;
            }

        case 21: // 1v1 Join Queue Skirmish
            {
                if (ArenaCheckFullEquipAndTalents(player) && Arena1v1CheckTalents(player) 
                        && JoinQueueArena(player, me, false, ARENA_TYPE_5v5) == false)
                ChatHandler(player->GetSession()).SendSysMessage("Something went wrong while joining queue. Already in another queue?");

                player->CLOSE_GOSSIP_MENU();
                return true;
            }

        case 3: // Leave Queue
            {
                uint8 arenaType = ARENA_TYPE_5v5;
                if (player->InBattlegroundQueueForBattlegroundQueueType(BATTLEGROUND_QUEUE_2v2_SOLO))
                arenaType = ARENA_TYPE_2v2_SOLO;

                WorldPacket Data;
                Data << arenaType << (uint8)0x0 << (uint32)BATTLEGROUND_AA << (uint16)0x0 << (uint8)0x0;
                player->GetSession()->HandleBattleFieldPortOpcode(Data);
                player->CLOSE_GOSSIP_MENU();
                return true;
            }

        }

        OnGossipHello(player, me);
        return true;
    }

private:
    int16 cache2v2Queue[MAX_TALENT_CAT];
    uint32 lastFetchQueueList;

    bool ArenaCheckFullEquipAndTalents(Player* player)
    {
        if (!player)
        return false;

        if (sWorld->getBoolConfig(CONFIG_ARENA_CHECK_EQUIP_AND_TALENTS) == false)
        return true;

        std::stringstream err;

        if (player->GetFreeTalentPoints() > 0)
        err << "ERROR: You have currently " << player->GetFreeTalentPoints() << " free talent points. Please spend all your talent points before queueing arena.\n";

        Item* newItem = NULL;
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            if (slot == EQUIPMENT_SLOT_OFFHAND || slot == EQUIPMENT_SLOT_RANGED || slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
            continue;

            newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (newItem == NULL)
            {
                err << "ERROR: Your character is not fully equipped.\n";
                break;
            }
        }

        if (err.str().length() > 0)
        {
            ChatHandler(player->GetSession()).SendSysMessage(err.str().c_str());
            return false;
        }
        return true;
    }


    bool JoinQueueArena(Player* player, Creature* me, bool isRated, uint8 arenatype)
    {
        if (!player || !me)
        return false;

        if (sWorld->getIntConfig(CONFIG_SOLO_2V2_MIN_LEVEL) > player->getLevel())
        return false;

        uint8 arenaslot = ArenaTeam::GetSlotByType(ARENA_TEAM_5v5);
        uint32 arenaRating = 0;
        uint32 matchmakerRating = 0;

        // ignore if we already in BG or BG queue
        if (player->InBattleground() || player->InBattlegroundQueue())
        return false;

        //check existance
        Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA);
        if (!bg)
        return false;

        if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, BATTLEGROUND_AA, NULL))
        {
            ChatHandler(player->GetSession()).PSendSysMessage(LANG_ARENA_DISABLED);
            return false;
        }

        BattlegroundTypeId bgTypeId = bg->GetTypeID();
        BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, arenatype);
        PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), player->getLevel());
        if (!bracketEntry)
        return false;

        // check if already in queue
        if (player->GetBattlegroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
        //player is already in this queue
        return false;
        // check if has free queue slots
        if (!player->HasFreeBattlegroundQueueId())
        return false;

        if (sWorld->getBoolConfig(CONFIG_SOLO_3V3_TIME_CHECK))
        {
            uint32 reqPlayedTime = 15 * MINUTE; // 15 minutes

            if (player->GetTotalPlayedTime() <= reqPlayedTime)
            {
                uint32 remainingTime = ((15 * MINUTE) - player->GetTotalPlayedTime()) / MINUTE;
                uint32 currentTime = 15 - remainingTime;

                ChatHandler(player->GetSession()).SendSysMessage(LANG_SOLO_ARENA_PLAYED_TIME);
                ChatHandler(player->GetSession()).PSendSysMessage(LANG_SOLO_ARENA_PLAYED_TIMER, currentTime);
                return false;
            }
        }

        uint32 ateamId = 0;

        if (isRated)
        {
            ateamId = player->GetArenaTeamId(arenaslot);
            ArenaTeam* at = sArenaTeamMgr->GetArenaTeamById(ateamId);
            if (!at)
            {
                player->GetSession()->SendNotInArenaTeamPacket(arenatype);
                return false;
            }

            // get the team rating for queueing
            arenaRating = at->GetRating();
            matchmakerRating = at->GetAverageMMR();
            // the arenateam id must match for everyone in the group

            if (arenaRating <= 0)
            arenaRating = 1;
        }
        
        BattlegroundQueue &bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);
        if (sWorld->getBoolConfig(CONFIG_SOLO_3V3_IP_CHECK))
        {
            if (bgQueue.IPExistsInQueue(player, NULL, bracketEntry, ARENA_TYPE_2v2_SOLO, isRated, false))
            {
                ChatHandler(player->GetSession()).SendSysMessage(LANG_SOLO_ARENA_CHECK_IP);
                return false;
            }

            if (bgQueue.IPExistsInQueue(player, NULL, bracketEntry, ARENA_TYPE_5v5, isRated, false))
            {
                ChatHandler(player->GetSession()).SendSysMessage(LANG_SOLO_ARENA_CHECK_IP);
                return false;
            }
        }
        
        bg->SetRated(isRated);

        GroupQueueInfo* ginfo = bgQueue.AddGroup(player, NULL, bgTypeId, bracketEntry, arenatype, isRated, false, arenaRating, matchmakerRating, ateamId);
        uint32 avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());
        uint32 queueSlot = player->AddBattlegroundQueueId(bgQueueTypeId);

        WorldPacket data;
        // send status packet (in queue)
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, queueSlot, STATUS_WAIT_QUEUE, avgTime, 0, arenatype, 0);
        player->GetSession()->SendPacket(&data);

        sBattlegroundMgr->ScheduleQueueUpdate(matchmakerRating, arenatype, bgQueueTypeId, bgTypeId, bracketEntry->GetBracketId());

        return true;
    }


    bool CreateArenateam(Player* player, Creature* me)
    {
        if (!player || !me)
        return false;

        uint8 slot = ArenaTeam::GetSlotByType(ARENA_TEAM_5v5);
        if (slot >= MAX_ARENA_SLOT)
        return false;

        // Check if player is already in an arena team
        if (player->GetArenaTeamId(slot))
        return false;

        // Teamname = playername
        // if team name exist, we have to choose another name (playername + number)
        int i = 1;
        std::stringstream teamName;
        teamName << player->GetName();
        do
        {
            if (sArenaTeamMgr->GetArenaTeamByName(teamName.str()) != NULL) // teamname exist, so choose another name
            {
                teamName.str(std::string());
                teamName << player->GetName() << i;
            }
            else
            break;
        } while (i < 100); // should never happen

        // Create arena team
        ArenaTeam* arenaTeam = new ArenaTeam();

        if (!arenaTeam->Create(player->GetGUID(), ARENA_TEAM_5v5, teamName.str(), 4283124816, 45, 4294242303, 5, 4294705149))
        {
            delete arenaTeam;
            return false;
        }

        // Register arena team
        sArenaTeamMgr->AddArenaTeam(arenaTeam);
        arenaTeam->AddMember(player->GetGUID());

        ChatHandler(player->GetSession()).SendSysMessage("Arena team successful created!");

        return true;
    }

    void fetchQueueList()
    {
        if (GetMSTimeDiffToNow(lastFetchQueueList) < 1000)
        return; // prevent spamming
        lastFetchQueueList = getMSTime();

        BattlegroundQueue* queue = &sBattlegroundMgr->GetBattlegroundQueue(BATTLEGROUND_QUEUE_2v2_SOLO);

        for (int i = 0; i < MAX_TALENT_CAT; ++i)
        cache2v2Queue[i] = 0;
        for (int i = BG_BRACKET_ID_FIRST; i <= BG_BRACKET_ID_LAST; ++i)
        for (int j = 0; j < 2; ++j)
        {
            for (BattlegroundQueue::GroupsQueueType::iterator itr = queue->m_QueuedGroups[i][j].begin(); itr != queue->m_QueuedGroups[i][j].end(); ++itr)
            {
                if ((*itr)->IsInvitedToBGInstanceGUID) // Skip when invited
                continue;

                std::map<ObjectGuid, PlayerQueueInfo*> *grpPlr = &(*itr)->Players;
                for (std::map<ObjectGuid, PlayerQueueInfo*>::iterator grpPlrItr = grpPlr->begin(); grpPlrItr != grpPlr->end(); ++grpPlrItr)
                {
                    Player* plr = ObjectAccessor::FindPlayer(grpPlrItr->first);
                    if (!plr)
                    continue;

                    Solo2v2TalentCat plrCat = GetTalentCatForSolo2v2(plr); // get talent cat
                    cache2v2Queue[plrCat]++;
                }
            }
        }
    }
};


void AddSC_npc_solo_queue()
{
    new npc_solo_queue();
}