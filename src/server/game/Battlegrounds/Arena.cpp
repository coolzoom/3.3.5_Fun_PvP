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

#include "Arena.h"
#include "ArenaScore.h"
#include "ArenaTeamMgr.h"
#include "Language.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "World.h"
#include "WorldSession.h"
#include "Battleground.h"

Arena::Arena()
{
	StartDelayTimes[BG_STARTING_EVENT_FIRST] = BG_START_DELAY_1M;
	StartDelayTimes[BG_STARTING_EVENT_SECOND] = BG_START_DELAY_30S;
	StartDelayTimes[BG_STARTING_EVENT_THIRD] = BG_START_DELAY_15S;
	StartDelayTimes[BG_STARTING_EVENT_FOURTH] = BG_START_DELAY_NONE;

	StartMessageIds[BG_STARTING_EVENT_FIRST] = LANG_ARENA_ONE_MINUTE;
	StartMessageIds[BG_STARTING_EVENT_SECOND] = LANG_ARENA_THIRTY_SECONDS;
	StartMessageIds[BG_STARTING_EVENT_THIRD] = LANG_ARENA_FIFTEEN_SECONDS;
	StartMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_ARENA_HAS_BEGUN;
}

void Arena::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    PlayerScores[player->GetGUID().GetCounter()] = new ArenaScore(player->GetGUID(), player->GetTeam());

    if (player->GetTeam() == ALLIANCE)        // gold
    {
        if (player->GetTeam() == HORDE)
            player->CastSpell(player, SPELL_HORDE_GOLD_FLAG, true);
        else
            player->CastSpell(player, SPELL_ALLIANCE_GOLD_FLAG, true);
    }
    else                                        // green
    {
        if (player->GetTeam() == HORDE)
            player->CastSpell(player, SPELL_HORDE_GREEN_FLAG, true);
        else
            player->CastSpell(player, SPELL_ALLIANCE_GREEN_FLAG, true);
    }
    player->RemoveArenaSpellCooldowns(true);
    player->RemovePet(NULL, PET_SAVE_NOT_IN_SLOT, true);
    player->RemoveAurasDueToSpell(27792); //Spirit of Redemption

    UpdateArenaWorldState();
}

void Arena::RemovePlayer(Player* player, ObjectGuid /*guid*/, uint32 /*team*/)
{
	if (GetStatus() == STATUS_WAIT_LEAVE)
		return;
        
    player->RemoveAurasDueToSpell(27792); //Spirit of Redemption
    player->RemoveAurasDueToSpell(29659);

	UpdateArenaWorldState();
	CheckWinConditions();
}

void Arena::FillInitialWorldStates(WorldPacket& data)
{
	data << uint32(ARENA_WORLD_STATE_ALIVE_PLAYERS_GREEN) << uint32(GetAlivePlayersCountByTeam(HORDE));
	data << uint32(ARENA_WORLD_STATE_ALIVE_PLAYERS_GOLD) << uint32(GetAlivePlayersCountByTeam(ALLIANCE));
}

void Arena::UpdateArenaWorldState()
{
	UpdateWorldState(ARENA_WORLD_STATE_ALIVE_PLAYERS_GREEN, GetAlivePlayersCountByTeam(HORDE));
	UpdateWorldState(ARENA_WORLD_STATE_ALIVE_PLAYERS_GOLD, GetAlivePlayersCountByTeam(ALLIANCE));
}

void Arena::HandleKillPlayer(Player* player, Player* killer)
{
	if (GetStatus() != STATUS_IN_PROGRESS)
		return;

	Battleground::HandleKillPlayer(player, killer);

	UpdateArenaWorldState();
	CheckWinConditions();
}

void Arena::RemovePlayerAtLeave(ObjectGuid guid, bool transport, bool sendPacket)
{
	if (isRated() && GetStatus() == STATUS_IN_PROGRESS)
	{
		BattlegroundPlayerMap::const_iterator itr = m_Players.find(guid);
		if (itr != m_Players.end()) // check if the player was a participant of the match, or only entered through gm command (appear)
		{
			// if the player was a match participant, calculate rating
			uint32 team = itr->second.Team;

			ArenaTeam* winnerArenaTeam = sArenaTeamMgr->GetArenaTeamById(GetArenaTeamIdForTeam(GetOtherTeam(team)));
			ArenaTeam* loserArenaTeam = sArenaTeamMgr->GetArenaTeamById(GetArenaTeamIdForTeam(team));

			// left a rated match while the encounter was in progress, consider as loser
			if (winnerArenaTeam && loserArenaTeam && winnerArenaTeam != loserArenaTeam)
			{
				if (Player* player = _GetPlayer(itr->first, itr->second.OfflineRemoveTime != 0, "Arena::RemovePlayerAtLeave"))
					loserArenaTeam->MemberLost(player, GetArenaMatchmakerRating(GetOtherTeam(team)));
				else
					loserArenaTeam->OfflineMemberLost(guid, GetArenaMatchmakerRating(GetOtherTeam(team)));
			}
		}
	}
	// remove player
	Battleground::RemovePlayerAtLeave(guid, transport, sendPacket);
}

void Arena::CheckWinConditions()
{
    if (!GetAlivePlayersCountByTeam(ALLIANCE) && GetPlayersCountByTeam(HORDE))
        EndBattleground(HORDE);
    else if (GetPlayersCountByTeam(ALLIANCE) && !GetAlivePlayersCountByTeam(HORDE))
        EndBattleground(ALLIANCE);
}

void Arena::EndBattleground(uint32 winner)
{
	// arena rating calculation
	if (isRated())
	{
		uint32 loserTeamRating = 0;
		uint32 loserMatchmakerRating = 0;
		int32  loserChange = 0;
		int32  loserMatchmakerChange = 0;
		uint32 winnerTeamRating = 0;
		uint32 winnerMatchmakerRating = 0;
		int32  winnerChange = 0;
		int32  winnerMatchmakerChange = 0;
        
		ArenaTeam* winnerArenaTeam = sArenaTeamMgr->GetArenaTeamById(GetArenaTeamIdForTeam(winner));
		ArenaTeam* loserArenaTeam = sArenaTeamMgr->GetArenaTeamById(GetArenaTeamIdForTeam(GetOtherTeam(winner)));

		uint32 Arena_Data[3][2]; // Two Teams 0 winner, 1 losser
		uint32 DamageDone, HealingDone, KillingBlows = 0;
        for (int y = 0; y < 2; y++)
            for (int x = 0; x < 3; x++)
                Arena_Data[x][y] = 0;

		int8 plrcount = 0;
		bool AntiWintrade = false;
        int8 lorand = irand(1,100);

		if (AntiWintrade == false)
		{
			//for (Battleground::BattlegroundScoreMap::const_iterator itr = GetPlayerScoresBegin(); itr != GetPlayerScoresEnd(); ++itr)
			for (Battleground::BattlegroundScoreMap::const_iterator itr = PlayerScores.begin(); itr != PlayerScores.end(); ++itr)
				if (Player* player = ObjectAccessor::FindPlayer(ObjectGuid(HighGuid::Player, itr->first)))
				{
					//if (player->GetBGTeam() == winner)
					//	continue;
					if (int32(player->GetMaxHealth()) <= int32(player->getLevel() * 150))
						continue;
					plrcount++;
				}
		}

		if (winnerArenaTeam && loserArenaTeam && winnerArenaTeam != loserArenaTeam)
		{
			loserTeamRating = loserArenaTeam->GetRating();
			loserMatchmakerRating = GetArenaMatchmakerRating(GetOtherTeam(winner));
			winnerTeamRating = winnerArenaTeam->GetRating();
			winnerMatchmakerRating = GetArenaMatchmakerRating(winner);

			/*if ((GetStartTime() <= 1 * IN_MILLISECONDS || (GetArenaType() == ARENA_TYPE_2v2 && plrcount < 4) || (GetArenaType() == ARENA_TYPE_3v3 && plrcount < 6) || (GetArenaType() == ARENA_TYPE_2v2_SOLO && plrcount < 6) || (GetArenaType() == ARENA_TYPE_5v5) && plrcount < 2))
				AntiWintrade = true;*/

			if (winner != 0)
			{
				if (AntiWintrade)
				{
					winnerChange = 0;
                    loserChange *= 2;
					winnerMatchmakerChange = 0;
				}
				else
                    winnerMatchmakerChange = winnerArenaTeam->WonAgainst(winnerMatchmakerRating, loserMatchmakerRating, winnerChange);

				loserMatchmakerChange = loserArenaTeam->LostAgainst(loserMatchmakerRating, winnerMatchmakerRating, loserChange);

				TC_LOG_DEBUG("bg.arena", "match Type: %u --- Winner: old rating: %u, rating gain: %d, old MMR: %u, MMR gain: %d --- Loser: old rating: %u, rating loss: %d, old MMR: %u, MMR loss: %d ---",
					GetArenaType(), winnerTeamRating, winnerChange, winnerMatchmakerRating, winnerMatchmakerChange,
					loserTeamRating, loserChange, loserMatchmakerRating, loserMatchmakerChange);
                    
                if (!AntiWintrade)
                    SetArenaMatchmakerRating(winner, winnerMatchmakerRating + winnerMatchmakerChange);
                    
				SetArenaMatchmakerRating(GetOtherTeam(winner), loserMatchmakerRating + loserMatchmakerChange);

				// bg team that the client expects is different to TeamId
				// alliance 1, horde 0
				uint8 winnerTeam = winner == ALLIANCE ? BG_TEAM_ALLIANCE : BG_TEAM_HORDE;
				uint8 loserTeam = winner == ALLIANCE ? BG_TEAM_HORDE : BG_TEAM_ALLIANCE;
				if (AntiWintrade)
				{
					winnerChange = 0;
                    loserChange *= 2;
					winnerMatchmakerChange = 0;
				}
				else
					_arenaTeamScores[winnerTeam].Assign(winnerChange, winnerMatchmakerRating, winnerArenaTeam->GetName());

				_arenaTeamScores[loserTeam].Assign(loserChange, loserMatchmakerRating, loserArenaTeam->GetName());

				TC_LOG_DEBUG("bg.arena", "Arena match Type: %u for Team1Id: %u - Team2Id: %u ended. WinnerTeamId: %u. Winner rating: +%d, Loser rating: %d",
					GetArenaType(), GetArenaTeamIdByIndex(TEAM_ALLIANCE), GetArenaTeamIdByIndex(TEAM_HORDE), winnerArenaTeam->GetId(), winnerChange, loserChange);

				//if (sWorld->getBoolConfig(CONFIG_ARENA_LOG_EXTENDED_INFO))
				for (auto const& score : PlayerScores)
					if (Player* player = ObjectAccessor::FindConnectedPlayer(ObjectGuid(HighGuid::Player, score.first)))
					{
						/*TC_LOG_DEBUG("bg.arena", "Statistics match Type: %u for %s (GUID: %u, Team: %d, IP: %s): %s",
						GetArenaType(), player->GetName().c_str(), score.first, player->GetArenaTeamId(GetArenaType() == 5 ? 2 : GetArenaType() == 3),
						player->GetSession()->GetRemoteAddress().c_str(), score.second->ToString().c_str());*/

						DamageDone = uint32(score.second->DamageDone);
						HealingDone = uint32(score.second->HealingDone);
						KillingBlows = uint32(score.second->KillingBlows);

						// Winner Data        
						if (player->GetArenaTeamId(GetArenaType()))
						{
							Arena_Data[0][0] += uint32(score.second->DamageDone);
							Arena_Data[1][0] += uint32(score.second->HealingDone);
							Arena_Data[2][0] += uint32(score.second->KillingBlows);
						}
						else if (player->GetArenaTeamId(GetArenaType()) == loserArenaTeam->GetId())
						{
							Arena_Data[0][1] += uint32(score.second->DamageDone);
							Arena_Data[1][1] += uint32(score.second->HealingDone);
							Arena_Data[2][1] += uint32(score.second->KillingBlows);
						}
					}

			}
			// Deduct 16 points from each teams arena-rating if there are no winners after 45+2 minutes
			else
			{
                _arenaTeamScores[BG_TEAM_ALLIANCE].Assign(ARENA_TIMELIMIT_POINTS_LOSS, winnerMatchmakerRating, winnerArenaTeam->GetName());
				_arenaTeamScores[BG_TEAM_HORDE].Assign(ARENA_TIMELIMIT_POINTS_LOSS, loserMatchmakerRating, loserArenaTeam->GetName());
                winnerArenaTeam->FinishGame(ARENA_TIMELIMIT_POINTS_LOSS);
				loserArenaTeam->FinishGame(ARENA_TIMELIMIT_POINTS_LOSS);
			}

			uint8 aliveWinners = GetAlivePlayersCountByTeam(winner);

			for (auto const& i : GetPlayers())
			{
				uint32 team = i.second.Team;

				if (i.second.OfflineRemoveTime)
				{
					// if rated arena match - make member lost!
					if (team == winner)
                    {
                        if (AntiWintrade)
                            winnerArenaTeam->OfflineMemberLost(i.first, loserMatchmakerRating, winnerMatchmakerChange);
                    }
					else
						loserArenaTeam->OfflineMemberLost(i.first, winnerMatchmakerRating, loserMatchmakerChange);
					continue;
				}

				Player* player = _GetPlayer(i.first, i.second.OfflineRemoveTime != 0, "Arena::EndBattleground");
				if (!player)
					continue;

				// per player calculation
				if (team == winner)
				{
					if (!AntiWintrade)
					{
						// update achievement BEFORE personal rating update
						uint32 rating = player->GetArenaPersonalRating(winnerArenaTeam->GetSlot());
                        if (winnerArenaTeam->GetSlot() == ARENA_TYPE_2v2_SOLO)
                            rating = player->GetArenaPersonalRating(ARENA_TEAM_5v5); // Try find 2v2 SoloQueue arena team ARENA_TEAM_5v5 == 1
                        if (rating == 0) // if can't find 2v2 SoloQueue, try find Type 5 arena team
                            rating = player->GetArenaPersonalRating(5);
						player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA, rating ? rating : 1);
						player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA, GetMapId());

						// Last standing - Rated 5v5 arena & be solely alive player
						if (GetArenaType() == ARENA_TYPE_2v2_SOLO && aliveWinners == 1 && player->IsAlive())
							player->CastSpell(player, SPELL_LAST_MAN_STANDING, true);
                            
                        /*if (GetStartTime() >= 10 * IN_MILLISECONDS)
                            if (lorand <= 30) // 35%
                                player->AddItem(37836, 1);*/

						winnerArenaTeam->MemberWon(player, loserMatchmakerRating, winnerMatchmakerChange);
					}
				}
				else
				{
					loserArenaTeam->MemberLost(player, winnerMatchmakerRating, loserMatchmakerChange);

					// Arena lost => reset the win_rated_arena having the "no_lose" condition
					player->ResetAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA, ACHIEVEMENT_CRITERIA_CONDITION_NO_LOSE);
				}
			}

			// save the stat changes
            winnerArenaTeam->SaveToDB();
			loserArenaTeam->SaveToDB();
			// send updated arena team stats to players
			// this way all arena team members will get notified, not only the ones who participated in this match
            winnerArenaTeam->NotifyStatsChanged();
			loserArenaTeam->NotifyStatsChanged();
		}
	}

	// end battleground
	Battleground::EndBattleground(winner);
}