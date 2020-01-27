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
#include <cstring>
#include <string.h>
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ArenaTeam.h"
#include "ArenaTeamMgr.h"
#include "World.h"
#include "WorldSession.h"
#include "Player.h"
#include "Config.h"

enum enus
{
    ARENA_MAX_RESULTS = 15,
    ARENA_2V2_LADDER = GOSSIP_ACTION_INFO_DEF + 40,
    ARENA_3V3_LADDER = GOSSIP_ACTION_INFO_DEF + 41,
    ARENA_5V5_LADDER = GOSSIP_ACTION_INFO_DEF + 42,
    ARENA_GOODBYE = GOSSIP_ACTION_INFO_DEF + 43,
    ARENA_START_TEAM_LOOKUP = GOSSIP_ACTION_INFO_DEF + 44
};

class arena_top_teams : public CreatureScript
{
public:
    uint32 optionToTeamType(uint32 option)
    {
        uint32 teamType;
        switch (option)
        {
            case ARENA_2V2_LADDER:
                teamType = 2;
                break;

            case ARENA_3V3_LADDER:
                teamType = 3;
                break;

            case ARENA_5V5_LADDER:
                teamType = 5;
                break;
        }
        return teamType;
    }

    uint32 teamTypeToOption(uint32 teamType)
    {
        uint32 option;
        switch (teamType)
        {
            case 2:
                option = ARENA_2V2_LADDER;
                break;

            case 3:
                option = ARENA_3V3_LADDER;
                break;

            case 5:
            case 1:
                option = ARENA_5V5_LADDER;
                break;
        }
        return option;
    }

    std::string raceToString(uint8 race)
    {
        std::string race_s = "Unknown";
        switch (race)
        {
            case RACE_HUMAN:
                race_s = "Человек";
                break;

            case RACE_ORC:
                race_s = "Орк";
                break;

            case RACE_DWARF:
                race_s = "Дворф";
                break;

            case RACE_NIGHTELF:
                race_s = "Ночной эльф";
                break;

            case RACE_UNDEAD_PLAYER:
                race_s = "Нежить";
                break;

            case RACE_TAUREN:
                race_s = "Таурен";
                break;

            case RACE_GNOME:
                race_s = "Гном";
                break;

            case RACE_TROLL:
                race_s = "Троль";
                break;

            case RACE_BLOODELF:
                race_s = "Кровавый эльф";
                break;

            case RACE_DRAENEI:
                race_s = "Дреней";
                break;
        }
        return race_s;
    }

    std::string classToString(uint8 Class)
    {
        std::string Class_s = "Unknown";
        switch (Class)
        {
            case CLASS_WARRIOR:
                Class_s = "Воин";
                break;

            case CLASS_PALADIN:
                Class_s = "Паладин";
                break;

            case CLASS_HUNTER:
                Class_s = "Охотник";
                break;

            case CLASS_ROGUE:
                Class_s = "Разбойник";
                break;

            case CLASS_PRIEST:
                Class_s = "Прист";
                break;

            case CLASS_DEATH_KNIGHT:
                Class_s = "Рыцарь смерти";
                break;

            case CLASS_SHAMAN:
                Class_s = "Шаман";
                break;

            case CLASS_MAGE:
                Class_s = "Маг";
                break;

            case CLASS_WARLOCK:
                Class_s = "Чернокнижник";
                break;

            case CLASS_DRUID:
                Class_s = "Друид";
                break;
        }
        return Class_s;
    }

    std::string getPlayerStatus(uint64 guid)
    {
		Player *player = ObjectAccessor::FindPlayer(ObjectGuid(guid));
        if (!player)
            return "Вне игры";
        if (player->isAFK())
            return "В игре, <AFK> ";
		if (player->isDND())
			return "В игре, <Busy> ";
        return "В игре";
    }

    std::string getWinPercent(uint32 wins, uint32 losses)
    {
        uint32 totalGames = wins + losses;
        if (totalGames == 0)
            return "0%";

        std::stringstream buf;
        uint32 percentage = (wins * 100) / totalGames;
        buf << percentage << "%";
        return buf.str();
    }

public:
    arena_top_teams() : CreatureScript("arena_top_teams"){}

    bool OnGossipHello(Player *player, Creature *creature)
    {
        player->ADD_GOSSIP_ITEM(2, "|cff00ff00|TInterface\\icons\\Achievement_Arena_2v2_7:30:30:-18:0|t|r Топ 15 Тим: 2v2 Арена", GOSSIP_SENDER_MAIN, ARENA_2V2_LADDER);
        player->ADD_GOSSIP_ITEM(2, "|cff00ff00|TInterface\\icons\\Achievement_Arena_3v3_7:30:30:-18:0|t|r Топ 15 Тим: 3v3 Арена", GOSSIP_SENDER_MAIN, ARENA_3V3_LADDER);
        player->ADD_GOSSIP_ITEM(2, "|cff00ff00|TInterface\\icons\\Achievement_Arena_5v5_7:30:30:-18:0|t|r Топ 15 Тим: Соло арены", GOSSIP_SENDER_MAIN, ARENA_5V5_LADDER);
        player->ADD_GOSSIP_ITEM(2, "|cff00ff00|TInterface\\icons\\spell_chargenegative:30:30:-18:0|t|r Выйти", GOSSIP_SENDER_MAIN, ARENA_GOODBYE);
        player->SEND_GOSSIP_MENU(90085, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (uiAction)
        {
        case ARENA_GOODBYE:
            {
                player->PlayerTalkClass->SendCloseGossip();
                break;
            }

        case ARENA_2V2_LADDER:
        case ARENA_5V5_LADDER:
        case ARENA_3V3_LADDER:
            {
                uint32 teamType = optionToTeamType(uiAction);
                QueryResult result = CharacterDatabase.PQuery("SELECT arenaTeamId, name, rating, seasonGames, seasonWins FROM arena_team WHERE type = '%u' ORDER BY rating DESC LIMIT %u;", teamType, ARENA_MAX_RESULTS);

                if (!result)
                {
                    player->ADD_GOSSIP_ITEM(7, "Выйти", GOSSIP_SENDER_MAIN, ARENA_GOODBYE);
                    player->SEND_GOSSIP_MENU(1, creature->GetGUID());
                }
                else
                {
                    std::string name;
                    uint32 teamId, rating, rank = 1, seasonGames, seasonWins = 1;
                    player->ADD_GOSSIP_ITEM(0, "Топ 15 Арена Тим список", GOSSIP_SENDER_MAIN, ARENA_GOODBYE);
                    do
                    {
                        Field *fields = result->Fetch();
                        teamId = fields[0].GetUInt32();
                        name = fields[1].GetString();
                        rating = fields[2].GetUInt32();
                        seasonGames = fields[3].GetUInt32();
                        seasonWins = fields[4].GetUInt32();
                        
                        std::stringstream buffer;

                        if (rank == 1)
                            buffer << "|cff00ff00|TInterface\\PvPRankBadges\\PvPRank14.png:26|t|r";
                        else if (rank == 2)
                            buffer << "|cff00ff00|TInterface\\PvPRankBadges\\PvPRank11.png:26|t|r";
                        else if (rank == 3)
                            buffer << "|cff00ff00|TInterface\\PvPRankBadges\\PvPRank07.png:26|t|r";
                        else 
                            buffer << "|cff00ff00|TInterface\\PvPRankBadges\\PvPRank03.png:26|t|r";

                        buffer << rank << ". " << name;
                        buffer << ": " << "|cFF1E90FF" << rating << "|r (W:" << seasonWins << " | L:" << (seasonGames - seasonWins) << ")";
                        player->ADD_GOSSIP_ITEM(4, buffer.str(), GOSSIP_SENDER_MAIN, ARENA_START_TEAM_LOOKUP + teamId);

                        rank++;
                    } while (result->NextRow());
                    player->ADD_GOSSIP_ITEM(7, "Выйти", GOSSIP_SENDER_MAIN, ARENA_GOODBYE);
                    player->SEND_GOSSIP_MENU(90086, creature->GetGUID());
                }
                break;
            }

        default:
            {
                if (uiAction > ARENA_START_TEAM_LOOKUP)
                {
                    uint32 teamId = uiAction - ARENA_START_TEAM_LOOKUP;
                    QueryResult result = CharacterDatabase.PQuery("SELECT name, rating, seasonWins, seasonGames - seasonWins, weekWins, weekGames - weekWins, rank, captainGuid , type FROM arena_team WHERE arenaTeamId = '%u'", teamId);

                    if (!result)
                    {
                        player->GetSession()->SendNotification("Такой команды нет...");
                        player->PlayerTalkClass->SendCloseGossip();
                        return true;
                    }

                    Field *fields = result->Fetch();
                    std::string name = fields[0].GetString();
                    uint32 rating = fields[1].GetUInt32();
                    uint32 seasonWins = fields[2].GetUInt32();
                    uint32 seasonLosses = fields[3].GetUInt32();
                    uint32 weekWins = fields[4].GetUInt32();
                    uint32 weekLosses = fields[5].GetUInt32();
                    uint32 rank = fields[6].GetUInt32();
                    uint32 captainGuid = fields[7].GetUInt32();
                    uint32 type = fields[8].GetUInt32();
                    uint32 parentOption = teamTypeToOption(type);

                    std::string seasonWinPercentage = getWinPercent(seasonWins, seasonLosses);
                    std::string weekWinPercentage = getWinPercent(weekWins, weekLosses);

                    std::stringstream buf;
                    std::string rankIco;
                    if (rank == 1)
                        rankIco = "|cff00ff00|TInterface\\PvPRankBadges\\PvPRank14.png:26|t|r";
                    else if (rank == 2)
                        rankIco = "|cff00ff00|TInterface\\PvPRankBadges\\PvPRank11.png:26|t|r";
                    else if (rank == 3)
                        rankIco = "|cff00ff00|TInterface\\PvPRankBadges\\PvPRank07.png:26|t|r";
                    else 
                        rankIco = "|cff00ff00|TInterface\\PvPRankBadges\\PvPRank03.png:26|t|r";
                        
                    buf << "Team Name: " << "|cFF1E90FF" << name << "|r\n"
                        << "Rating: " << "|cFF1E90FF" << rating << "|r" << " (Ранг: " << "|cFF1E90FF" << rank << rankIco <<"|r" << ", Тип: " << "|cFF1E90FF" << type << "v" << type << "|r"")\n"
                        << "Total Week: " << "|cFF1E90FF" << weekWins << "-" << weekLosses << "|r"" (" << "|cFF1E90FF" << weekWinPercentage << "|r" << "win)\n"
                        << "Total Season: " << "|cFF1E90FF" << seasonWins << "-" << seasonLosses << "|r" << " (" << "|cFF1E90FF" << seasonWinPercentage << "|r" << " win) - Total Games: |cFF1E90FF" << (seasonWins+seasonLosses) << "|r played";
                    player->ADD_GOSSIP_ITEM(4, buf.str(), GOSSIP_SENDER_MAIN, parentOption);

                    QueryResult members = CharacterDatabase.PQuery("SELECT  a.guid, a.personalRating, a.weekWins, a.weekGames - a.weekWins, a.seasonWins, a.seasonGames - a.seasonWins, c.name, c.race, c.class, c.level FROM arena_team_member a LEFT JOIN characters c ON c.guid = a.guid WHERE arenaTeamId = '%u' ORDER BY a.guid = '%u' DESC, a.seasonGames DESC, c.name ASC", teamId, captainGuid);
                    if (!members)
                    {
                        player->ADD_GOSSIP_ITEM(7, "No team members found...?", GOSSIP_SENDER_MAIN, parentOption);
                    }
                    else
                    {
                        uint32 memberPos = 1;
                        uint32 memberCount = members->GetRowCount();
                        uint32 guid, personalRating, level;
                        std::string name, race, Class;

                        buf.str("");
                        buf << "      --- " << memberCount << " team" << ((memberCount == 1) ? " member" : " members") << " found" << " ---";
                        player->ADD_GOSSIP_ITEM(0, buf.str(), GOSSIP_SENDER_MAIN, parentOption);
                        do
                        {
                            fields = members->Fetch();
                            guid = fields[0].GetUInt32();
                            personalRating = fields[1].GetUInt32();
                            weekWins = fields[2].GetUInt32();
                            weekLosses = fields[3].GetUInt32();
                            seasonWins = fields[4].GetUInt32();
                            seasonLosses = fields[5].GetUInt32();
                            name = fields[6].GetString();
                            race = raceToString(fields[7].GetUInt8());
                            Class = classToString(fields[8].GetUInt8());
                            level = fields[9].GetUInt32();

                            seasonWinPercentage = getWinPercent(seasonWins, seasonLosses);
                            weekWinPercentage = getWinPercent(weekWins, weekLosses);

                            buf.str("");
                            buf << memberPos << ". ";
                            if (guid == captainGuid)
                                buf << "Team Captain ";

							buf << rankIco << name << ", " << getPlayerStatus(uint64(guid)) << "\n"
                                << race << " " << Class << ", " << "|cFF1E90FF" << personalRating << "|r" << " personal rating!\n"
                                << "Week: " << "|cFF1E90FF" << weekWins << "-" << weekLosses << "|r" << " (" << "|cFF1E90FF" << weekWinPercentage << "|r" << " win), " << "|cFF1E90FF" << (weekWins + weekLosses) << "|r" << " played!\n"
                                << "Season: " << "|cFF1E90FF" << seasonWins << "-" << seasonLosses << "|r" << " (" << "|cFF1E90FF" << seasonWinPercentage << "|r" << " win), " << "|cFF1E90FF" << (seasonWins + seasonLosses) << "|r" << " played!";
                            player->ADD_GOSSIP_ITEM(4, buf.str(), GOSSIP_SENDER_MAIN, parentOption);
                            memberPos++;
                        } while (members->NextRow());
                    }
                    player->SEND_GOSSIP_MENU(90087, creature->GetGUID());
                }
            }
        }
        return true;
    }
};

void AddSC_arena_top_teams()
{
    new arena_top_teams();
}