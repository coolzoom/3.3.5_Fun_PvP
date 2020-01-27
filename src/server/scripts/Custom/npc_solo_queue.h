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
 
#include "Chat.h"

#ifndef SOLO_3V3_H
#define SOLO_3V3_H

const int8 PLAYERS_COUNT_PER_TEAM = 3;
const int8 REQUIRED_TOTAL_PLAYERS = 6;

// SOLO_3V3_TALENTS found in: TalentTab.dbc -> TalentTabID

// Warrior, Rogue, Deathknight etc.
const uint32 SOLO_2v2_TALENTS_MELEE[] =
{
    383, // PaladinProtection
    163, // WarriorProtection
    361,
    161,
    182,
    398,
    164,
    181,
    263,
    281,
    399,
    183,
    381,
    400,
    0 // End
};

enum SoloTrinityStrings // Language.h might have same entries, appears when executing SQL, change if needed
{
    LANG_SOLO_ARENA_DISABLED         = 11200, //Arena disabled
    LANG_SOLO_ARENA_TEAM_CREATED     = 11201, //Arena team successful created!
    LANG_SOLO_ARENA_PLAYED_TIME      = 11202, //You must have at least 15 minutes of played time before you may queue. Please spend a little more time setting up your character, and try again soon.    
    LANG_SOLO_ARENA_PLAYED_TIMER     = 11203, //Your current played time: %u minutes, but need 15.
    LANG_SOLO_ARENA_CHECK_IP         = 11204, //You cannot join Arena now because there is already someone queued with the same IP address.
    LANG_SOLO_ARENA_CHAR_EQUIP       = 11205, //Your character is not fully equipped.
    LANG_SOLO_ARENA_ERROR            = 11206, //You are in Arena Queue.
    LANG_SOLO_ARENA_ERROR_JOIN       = 11207, //Something went wrong while joining queue. Already in another queue?
    LANG_SOLO_ARENA_CURRENTLY_TALENT = 11208, //You have currently 
    LANG_SOLO_ARENA_CURRENTLY_POINT  = 11209, //free talent points.Please spend all your talent points before queueing arena.
    LANG_SOLO_ARENA_LVL_ERROR        = 11210, //You need level %u to create an arena team.
    LANG_SOLO_ARENA_CHECK_TALENTS    = 11211, //You can't join, because you have invested too many points in a forbidden talent. Please edit your talents.
	LANG_SOLO_ARENA_CHANGE_TALENTS   = 11212, //You can't change your talents while in queue for 1v1 or 3v3 Solo.
};

// Mage, Hunter, Warlock etc.
const uint32 SOLO_2v2_TALENTS_RANGE[] =
{
    81,
    261,
    283,
    302,
    41,
    303,
    363,
    61,
    203,
    301,
    362,
    0 // End
};
/*const uint32 SOLO_2v2_TALENTS_DPS[] =
{
	383, // PaladinProtection
	163, // WarriorProtection
	161,
	182,
	398,
	164,
	181,
	263,
	281,
	399,
	183,
	381,
	400,
	81,
	261,
	283,
	302,
	361,
	41,
	303,
	363,
	61,
	203,
	301,
	362,
	0 // End
};*/
const uint32 SOLO_2v2_TALENTS_HEAL[] =
{
    201, // PriestDiscipline
    202, // PriestHoly
    382, // PaladinHoly
    262, // ShamanRestoration
    282, // DruidRestoration
    0 // End
};

// TalentTab.dbc -> TalentTabID
const uint32 FORBIDDEN_TALENTS_IN_1V1_ARENA[] = 
{
	// Healer
	201, // PriestDiscipline
	202, // PriestHoly
	382, // PaladinHoly
	262, // ShamanRestoration
	282, // DruidRestoration

	// Tanks
	//383, // PaladinProtection
	//163, // WarriorProtection

	0 // End
};

// Returns MELEE, RANGE or HEALER (depends on talent builds)
static Solo2v2TalentCat GetTalentCatForSolo2v2(Player* player)
{
    if (!player || sWorld->getBoolConfig(CONFIG_SOLO_2V2_FILTER_TALENTS) == false)
        return MELEE;

    uint32 count[MAX_TALENT_CAT];
    for (int i = 0; i < MAX_TALENT_CAT; i++)
        count[i] = 0;

    for (uint32 talentId = 0; talentId < sTalentStore.GetNumRows(); ++talentId)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentId);

        if (!talentInfo)
            continue;

        for (int8 rank = MAX_TALENT_RANK-1; rank >= 0; --rank)
        {
            if (talentInfo->RankID[rank] == 0)
                continue;
                        
            if (player->HasTalent(talentInfo->RankID[rank], player->GetActiveSpec()))
            {
                for (int8 i = 0; SOLO_2v2_TALENTS_MELEE[i] != 0; i++)
                    if (SOLO_2v2_TALENTS_MELEE[i] == talentInfo->TalentTab)
                        count[MELEE] += rank + 1;

                for (int8 i = 0; SOLO_2v2_TALENTS_RANGE[i] != 0; i++)
                    if (SOLO_2v2_TALENTS_RANGE[i] == talentInfo->TalentTab)
                        count[RANGE] += rank + 1;

                for (int8 i = 0; SOLO_2v2_TALENTS_HEAL[i] != 0; i++)
                    if (SOLO_2v2_TALENTS_HEAL[i] == talentInfo->TalentTab)
                        count[HEALER] += rank + 1;
            }
        }
    }


    uint32 prevCount = 0;
    Solo2v2TalentCat talCat = MELEE; // Default MELEE (if no talent points set)
    for (int i = 0; i < MAX_TALENT_CAT; i++)
    {
        if (count[i] > prevCount)
        {
            talCat = (Solo2v2TalentCat)i;
            prevCount = count[i];
        }
    }

    return talCat;
}


// Return false, if player have invested more than 35 talentpoints in a forbidden talenttree.
static bool Arena1v1CheckTalents(Player* player)
{
	if(!player)
		return false;

	if(sWorld->getBoolConfig(CONFIG_ARENA_1V1_BLOCK_FORBIDDEN_TALENTS) == false)
		return true;

	uint32 count = 0;
	for (uint32 talentId = 0; talentId < sTalentStore.GetNumRows(); ++talentId)
	{
		TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentId);

		if (!talentInfo)
			continue;

		for (int8 rank = MAX_TALENT_RANK - 1; rank >= 0; --rank)
		{
			if (talentInfo->RankID[rank] == 0)
				continue;
						
			if (player->HasTalent(talentInfo->RankID[rank], player->GetActiveSpec()))
			{
				for(int8 i = 0; FORBIDDEN_TALENTS_IN_1V1_ARENA[i] != 0; ++i)
					if(FORBIDDEN_TALENTS_IN_1V1_ARENA[i] == talentInfo->TalentTab)
						count += rank + 1;
			}
		}
	}

	if(count >= 36)
	{
		ChatHandler(player->GetSession()).SendSysMessage("Вы не можете присоединиться, так как вложили много талантов в запрещенную ветку. Пожалуйста, поменяйте таланты.");
		return false;
	}
	else
		return true;
}

#endif