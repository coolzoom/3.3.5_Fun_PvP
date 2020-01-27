/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
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

#include "ScriptMgr.h"
#include "Group.h"
#include "SocialMgr.h"

class challenge_commandscript : public CommandScript
{
public:
    challenge_commandscript() : CommandScript("challenge_commandscript") { }

    std::vector<ChatCommand> GetCommands() const
    {
        static std::vector<ChatCommand> challengeCommandTable =
        {            
            { "1x1",            SEC_PLAYER,      true, &HandleSkirmish1x1Command,       "" },
            { "2x2",            SEC_PLAYER,      true, &HandleSkirmish2x2Command,       "" },
            { "3x3",            SEC_PLAYER,      true, &HandleSkirmish3x3Command,       "" },
            { "player",         SEC_PLAYER,      true, &HandleSkirmishPlaCommand,       "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "challenge",      SEC_PLAYER,         true, NULL,                     "", challengeCommandTable },
        };
        return commandTable;
    }

    static bool CreateSkirmish(ChatHandler* handler, char* args, uint8 type, bool isGroup, bool isGM)
    {
        char* pParam1 = strtok(args, " ");
        Player *player1, *player2;
        
        if (!pParam1)
            return false;

        if (isGM)
        {
            char* pParam2 = strtok(NULL, " ");
            if (!pParam2)
                return false;

            if (!handler->extractPlayerTarget(pParam2, &player2))
                return false;
        }
        else
            player2 = handler->GetSession()->GetPlayer();

        if (!handler->extractPlayerTarget(pParam1, &player1))
            return false;
            
        if (player1->getLevel() < 80)
            return false;
            
        if (player2->getLevel() < 80)
            return false;
            
        if (type == 3)
        {
            uint8 grps1 = 1;
            uint8 grps2 = 1;
            if (Group* group = player1->GetGroup())
                if (group->GetMembersCount() <= 3)
                    grps1 = group->GetMembersCount();
                    
            if (Group* group = player2->GetGroup())
                if (group->GetMembersCount() <= 3)
                    grps2 = group->GetMembersCount();
 
            if (grps1 != grps2)
            {
                handler->PSendSysMessage("Players in groups is incorrect.");
                handler->SetSentErrorMessage(true);
                return false;
            }
            else if (grps1 > 1 && grps2 > 1)
                isGroup = true;

            switch (grps1)
            {
                case 3:  type = 1; break;
                case 2:  type = 0; break;
                case 1:  type = 2; break;
                default: type = 2; break;
            }            
        }

        if (isGroup) // 1v1
        {
            Group *gr1 = player1->GetGroup();
            Group *gr2 = player2->GetGroup();

            if (!gr1)
            {
                handler->PSendSysMessage("First player haven`t group.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (!gr2)
            {
                handler->PSendSysMessage("Second player haven`t group.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (!gr1->IsLeader(player1->GetGUID()))
                if (gr1->GetLeaderGUID())
                    if (Player* leader1 = ObjectAccessor::FindPlayer(gr1->GetLeaderGUID()))
                        player1 = leader1;

            if (!gr2->IsLeader(player2->GetGUID()))
                if (gr2->GetLeaderGUID())
                    if (Player* leader2 = ObjectAccessor::FindPlayer(gr2->GetLeaderGUID()))
                        player2 = leader2;
        }
        
        if (!player2->GetSocial() || player2->GetSocial()->HasIgnore(player1->GetGUID()))
        {
            handler->PSendSysMessage("ERROR: You are ignored by target.");
            handler->SetSentErrorMessage(true);
            return false;
        }
        
        if (player1->IsInCombat() || player2->IsInCombat())
        {
            handler->PSendSysMessage("ERROR: You or Oponnent is in Combat.");
            handler->SetSentErrorMessage(true);
            return false;
        }
        if (player1->GetMap()->IsBattlegroundOrArena() || player2->GetMap()->IsBattlegroundOrArena())
        {
            handler->PSendSysMessage("ERROR: You or Oponnent is in Battleground.");
            handler->SetSentErrorMessage(true);
            return false;
        }
        if (player1->InBattlegroundQueue() || player2->InBattlegroundQueue())
        {
            handler->PSendSysMessage("ERROR: You or Oponnent is in Arena Queue.");
            handler->SetSentErrorMessage(true);
            return false;
        }
        if (player1->GetGUID() == player2->GetGUID())
        {
            handler->PSendSysMessage("ERROR: You can't challenge self.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player1->IsGameMaster() || player2->IsGameMaster())
        {
            handler->PSendSysMessage("ERROR: Cant versus GMs.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player1->HasAura(9454) || player2->HasAura(9454))
        {
            handler->PSendSysMessage("ERROR: You are Frozen");
            handler->SetSentErrorMessage(true);
            return false;
        }

        WorldPacket data1(CMSG_BATTLEMASTER_JOIN_ARENA, 8+1+1+1);
        data1 << uint64(0);
        data1 << uint8(type);
        if (isGroup) 
            data1 << uint8(1);
        else
            data1 << uint8(0);
            
        data1 << uint8(0);

        WorldPacket data2(CMSG_BATTLEMASTER_JOIN_ARENA, 8+1+1+1);
        data2 << uint64(0);
        data2 << uint8(type);
        if (isGroup) 
            data2 << uint8(1);
        else
            data2 << uint8(0);
        data2 << uint8(0);

        int8 aType;

        switch (type)
        {
            case 0:
                aType = ARENA_TYPE_2v2;
                break;
            case 1:
                aType = ARENA_TYPE_3v3;
                break;
            case 2:
                aType = ARENA_TYPE_5v5;
                break;
            default:
                //sLog->outError("Unknown arena slot %u at CreateSkirmish", type);
                return false;
        }
        
        player1->setSkirmishStatus(aType, SKIRMISH_PREPEAR);
        player2->setSkirmishStatus(aType, SKIRMISH_PREPEAR);
        player1->GetSession()->HandleBattlemasterJoinArena(data1);
        player2->GetSession()->HandleBattlemasterJoinArena(data2);
        
        if (player1->getSkirmishStatus(aType) != SKIRMISH_JOINED)
        {
            player1->setSkirmishStatus(aType, SKIRMISH_NONE);
            handler->PSendSysMessage("ERROR: There are some problems when try to join first team.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player2->getSkirmishStatus(aType) != SKIRMISH_JOINED)
        {
            player2->setSkirmishStatus(aType, SKIRMISH_NONE);
            handler->PSendSysMessage("ERROR: There are some problems when try to join second team.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    static bool HandleSkirmish2x2Command(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        return CreateSkirmish(handler, (char*)args, 0, true, true);
    }

    static bool HandleSkirmish3x3Command(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        return CreateSkirmish(handler, (char*)args, 1,  true, true);
    }
    
    static bool HandleSkirmish1x1Command(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        return CreateSkirmish(handler, (char*)args, 2,  false, true);
    }

    static bool HandleSkirmishPlaCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        return CreateSkirmish(handler, (char*)args, 3,  false, false);
    }

};

void AddSC_challenge_commandscript()
{
    new challenge_commandscript();
}