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

#include "Config.h"
#include "Unit.h"
#include "Chat.h"
#include "Creature.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "World.h"
#include "Pet.h"
#include "Map.h"
#include "ScriptMgr.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Language.h"
#include <vector>
#include <map>
#include "custom_check.h"

#define BOOL_TO_STRING(b) ((b)? "true":"false")

struct AutoBalanceCreatureInfo
{
    uint32 instancePlayerCount;
    float DamageMultiplier;
};

static std::map<uint32, AutoBalanceCreatureInfo> CreatureInfo; 
static std::map<int, int> forcedCreatureIds;                   

void LoadForcedCreatureIdsFromString(std::string creatureIds, int forcedPlayerCount) 
{
    std::string delimitedValue;
    std::stringstream creatureIdsStream;

    creatureIdsStream.str(creatureIds);
    while (std::getline(creatureIdsStream, delimitedValue, ',')) 
    {
        int creatureId = atoi(delimitedValue.c_str());
        if (creatureId >= 0)
        {
            forcedCreatureIds[creatureId] = forcedPlayerCount;
        }
    }
}

int GetForcedCreatureId(int creatureId)
{
    if(forcedCreatureIds.find(creatureId) == forcedCreatureIds.end())
    {
        return 0;
    }
    return forcedCreatureIds[creatureId];
}

class VAS_AutoBalance_WorldScript : public WorldScript
{
public:
    VAS_AutoBalance_WorldScript()
    : WorldScript("VAS_AutoBalance_WorldScript")
    {
    }

    void OnConfigLoad(bool /*reload*/)
    {
    }

    void OnStartup()
    {
    }

    void SetInitialWorldSettings()
    {
        forcedCreatureIds.clear();
        LoadForcedCreatureIdsFromString(sWorld->GetVAS40(), 40);
        LoadForcedCreatureIdsFromString(sWorld->GetVAS25(), 25);
        LoadForcedCreatureIdsFromString(sWorld->GetVAS10(), 10);
        LoadForcedCreatureIdsFromString(sWorld->GetVAS5(), 5);
        LoadForcedCreatureIdsFromString(sWorld->GetVAS2(), 2);
    }

};

class VAS_AutoBalance_PlayerScript : public PlayerScript
{
public:
    VAS_AutoBalance_PlayerScript()
    : PlayerScript("VAS_AutoBalance_PlayerScript")
    {
    }

    void OnLogin(Player *Player, bool /*firstLogin*/)
    {
    }
};

class VAS_AutoBalance_UnitScript : public UnitScript
{
public:
    VAS_AutoBalance_UnitScript()
    : UnitScript("VAS_AutoBalance_UnitScript")
    {
    }

    uint32 DealDamage(Unit* AttackerUnit, Unit *playerVictim, uint32 damage, DamageEffectType damagetype)
    {
        if (AttackerUnit->GetMap()->IsDungeon() && playerVictim->GetMap()->IsDungeon())
        if (AttackerUnit->GetTypeId() != TYPEID_PLAYER)
        {
            damage = VAS_Modifer_DealDamage(AttackerUnit,damage);
        }
        return damage;
    }

    void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage)
    {
        if (attacker->GetMap()->IsDungeon() && target->GetMap()->IsDungeon())
        {
            if (attacker->GetTypeId() != TYPEID_PLAYER)
            {
                if (!((attacker->IsHunterPet() || attacker->IsPet() || attacker->IsSummon()) && attacker->IsControlledByPlayer()))
                {
                    float multiplyer = (float)CreatureInfo[attacker->GetGUID()].DamageMultiplier;
                    damage = damage * multiplyer;
                }
            }
        }
    }
    
    void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage)
    {
        if ((attacker->GetMap()->IsDungeon() && target->GetMap()->IsDungeon()))
        {
            if (attacker->GetTypeId() != TYPEID_PLAYER)
            {
                if ((attacker->IsHunterPet() || attacker->IsPet() || attacker->IsSummon()) && attacker->IsControlledByPlayer())
                return;
                float multiplyer = (float)CreatureInfo[attacker->GetGUID()].DamageMultiplier;
                damage = damage * multiplyer;
            }
        }
        return;
    }

    void ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage)
    {
        if (((attacker->GetMap()->IsDungeon() && target->GetMap()->IsDungeon())) && (attacker->GetTypeId() != TYPEID_PLAYER))
        if (!((attacker->IsHunterPet() || attacker->IsPet() || attacker->IsSummon()) && attacker->IsControlledByPlayer()))
        {
            float multiplyer = (float)CreatureInfo[attacker->GetGUID()].DamageMultiplier;
            damage = damage * multiplyer;
        }
        return;
    }

    uint32 VAS_Modifer_DealDamage(Unit* AttackerUnit,uint32 damage)
    {
        if ((AttackerUnit->IsHunterPet() || AttackerUnit->IsPet() || AttackerUnit->IsSummon()) && AttackerUnit->IsControlledByPlayer())
        return damage;

        float damageMultiplier = CreatureInfo[AttackerUnit->GetGUID()].DamageMultiplier;

        return damage * damageMultiplier;

    }

};

class VAS_AutoBalance_AllMapScript : public AllMapScript
{
public:
    VAS_AutoBalance_AllMapScript()
    : AllMapScript("VAS_AutoBalance_AllMapScript")
    {
    }

    void OnPlayerEnterAll(Map* map, Player* player)
    {
        if (sWorld->getIntConfig(CONFIG_VAS_PLAYER_CHANGE_NOFITY) >= 1)
        {
            if ((map->GetEntry()->IsDungeon()) && !player->IsGameMaster() )
            {
                Map::PlayerList const &playerList = map->GetPlayers();
                if (!playerList.isEmpty())
                {
                    for (Map::PlayerList::const_iterator playerIteration = playerList.begin(); playerIteration != playerList.end(); ++playerIteration)
                    {
                        if (Player* playerHandle = playerIteration->GetSource())
                        {
                            ChatHandler chatHandle = ChatHandler(playerHandle->GetSession());
                            chatHandle.PSendSysMessage(LANG_VAS_BALANCE_JOIN,player->GetName().c_str(),map->GetMapName(),map->GetPlayersCountExceptGMs());
                        }
                    }
                }
            }
        }
    }

    void OnPlayerLeaveAll(Map* map, Player* player)
    {
        int instancePlayerCount = map->GetPlayersCountExceptGMs() - 1;

        if (instancePlayerCount >=1)
        {
            if (sWorld->getIntConfig(CONFIG_VAS_PLAYER_CHANGE_NOFITY) >= 1)
            {
                if ((map->GetEntry()->IsDungeon()) && !player->IsGameMaster())
                {
                    Map::PlayerList const &playerList = map->GetPlayers();
                    if (!playerList.isEmpty())
                    {
                        for (Map::PlayerList::const_iterator playerIteration = playerList.begin(); playerIteration != playerList.end(); ++playerIteration)
                        {
                            if (Player* playerHandle = playerIteration->GetSource())
                            {
                                ChatHandler chatHandle = ChatHandler(playerHandle->GetSession());
                                chatHandle.PSendSysMessage(LANG_VAS_BALANCE_LEFT,player->GetName().c_str(),map->GetMapName(),instancePlayerCount);
                            }
                        }
                    }
                }
            }
        }
    }
};

class VAS_AutoBalance_WorldMapScript : public WorldMapScript
{
public:
    VAS_AutoBalance_WorldMapScript()
    : WorldMapScript("VAS_AutoBalance_WorldMapScript",0)
    {
    }

    void OnPlayerEnter(Map* map, Player* player)
    {
    }

    void OnPlayerLeave(Map* map, Player* player)
    {
    }
};

class VAS_AutoBalance_AllCreatureScript : public AllCreatureScript
{
public:
    VAS_AutoBalance_AllCreatureScript()
    : AllCreatureScript("VAS_AutoBalance_AllCreatureScript")
    {
    }


    void Creature_SelectLevel(const CreatureTemplate *creatureTemplate, Creature* creature)
    {
        if (creature->GetMap()->IsDungeon())
        {
            ModifyCreatureAttributes(creature);
            CreatureInfo[creature->GetGUID()].instancePlayerCount = creature->GetMap()->GetPlayersCountExceptGMs();
        }
    }

    void OnAllCreatureUpdate(Creature* creature, uint32 diff)
    {
        if(!(CreatureInfo[creature->GetGUID()].instancePlayerCount == creature->GetMap()->GetPlayersCountExceptGMs()))
        {
            if (creature->GetMap()->IsDungeon() || !creature->GetVehicle())
            ModifyCreatureAttributes(creature);
            CreatureInfo[creature->GetGUID()].instancePlayerCount = creature->GetMap()->GetPlayersCountExceptGMs();
        }
    }

    void ModifyCreatureAttributes(Creature* creature)
    {
        if(((creature->IsHunterPet() || creature->IsPet() || creature->IsSummon()) && creature->IsControlledByPlayer()) || sWorld->getIntConfig(CONFIG_VAS_AUTOINSTANCE) < 1 || creature->GetMap()->GetPlayersCountExceptGMs() <= 0)
        {
            return;
        }

        CreatureTemplate const *creatureTemplate = creature->GetCreatureTemplate();
        CreatureBaseStats const* creatureStats = sObjectMgr->GetCreatureBaseStats(creature->getLevel(), creatureTemplate->unit_class);

        float damageMultiplier = 1.0f;
        float healthMultiplier = 1.0f;

        uint32 baseHealth = creatureStats->GenerateHealth(creatureTemplate);
        uint32 baseMana = creatureStats->GenerateMana(creatureTemplate);
        uint32 instancePlayerCount = creature->GetMap()->GetPlayersCountExceptGMs();
        uint32 maxNumberOfPlayers = ((InstanceMap*)sMapMgr->FindMap(creature->GetMapId(), creature->GetInstanceId()))->GetMaxPlayers();
        uint32 scaledHealth = 0;

        //   VAS SOLO  - By MobID
        if(GetForcedCreatureId(creatureTemplate->Entry) > 0)
        {
            maxNumberOfPlayers = GetForcedCreatureId(creatureTemplate->Entry);
        }

        switch (maxNumberOfPlayers)
        {
        case 40:
            healthMultiplier = (float)instancePlayerCount / (float)maxNumberOfPlayers; 
            break;
        case 25:
            healthMultiplier = (tanh((instancePlayerCount - 16.5f) / 1.5f) + 1.0f) / 2.0f;
            break;
        case 10:
            healthMultiplier = (tanh((instancePlayerCount - 4.5f) / 1.5f) + 1.0f) / 2.0f;
            break;
        case 2:
            healthMultiplier = (float)instancePlayerCount / (float)maxNumberOfPlayers;
            break;
        default:
            healthMultiplier = (tanh((instancePlayerCount - 2.2f) / 1.5f) + 1.0f) / 2.0f;
        }

        if(healthMultiplier <= sWorld->getFloatConfig(CONFIG_VAS_MIN_HP_MOD) )
        {
            healthMultiplier = sWorld->getFloatConfig(CONFIG_VAS_MIN_HP_MOD);
        }

        scaledHealth = uint32((baseHealth * healthMultiplier) + 1.0f);
        if (maxNumberOfPlayers==0)
        {
            damageMultiplier = healthMultiplier;
        }
        else
        {
            damageMultiplier = (float)instancePlayerCount / (float)maxNumberOfPlayers;
        }

        if(damageMultiplier <= sWorld->getFloatConfig(CONFIG_VAS_MIN_DAMAGE_MOD))
        {
            damageMultiplier = sWorld->getFloatConfig(CONFIG_VAS_MIN_DAMAGE_MOD);
        }

        creature->SetCreateHealth(scaledHealth);
        creature->SetMaxHealth(scaledHealth);
        creature->ResetPlayerDamageReq();
        creature->SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, (float)scaledHealth);
        CreatureInfo[creature->GetGUID()].DamageMultiplier = damageMultiplier;
    }
};

#include <map>

#include "Config.h"
#include "ScriptMgr.h"
#include "Unit.h"
#include "Player.h"
#include "Pet.h"
#include "Map.h"
#include "Group.h"
#include "InstanceScript.h"


namespace 
{

	class solocraft_player_instance_handler : public PlayerScript 
    {
	public:
		solocraft_player_instance_handler() : PlayerScript("solocraft_player_instance_handler") 
        {
			TC_LOG_INFO("scripts.solocraft.player.instance", "[Solocraft] solocraft_player_instance_handler Loaded");
		}

		void OnMapChanged(Player *player) override 
        {
			Map *map = player->GetMap();
			int difficulty = CalculateDifficulty(map, player);
			int numInGroup = GetNumInGroup(player);
			ApplyBuffs(player, map, difficulty, numInGroup);
		}
	private:
		std::map<ObjectGuid, int> _unitDifficulty;

        int CalculateDifficulty(Map *map, Player *player) 
        {
            int difficulty = 1;
            if (map)
            {
                if (map->IsBattleground())
                {
                    return false;
                }
                else if (map->Is25ManRaid()) 
                {
                    difficulty = 25;
                }
                else if (map->IsHeroic()) 
                {
                    difficulty = 10;
                }
                else if (map->IsRaid()) 
                {
                    difficulty = 40;
                } 
                else if (map->IsDungeon()) 
                {
                    difficulty = 10;
                }
            }
            return difficulty;
        }

		int GetNumInGroup(Player *player) 
        {
			int numInGroup = 1;
			Group *group = player->GetGroup();
			if (group) 
            {
				Group::MemberSlotList const& groupMembers = group->GetMemberSlots();
				numInGroup = groupMembers.size();
			}
			return numInGroup;
		}

		void ApplyBuffs(Player *player, Map *map, int difficulty, int numInGroup) 
        {
			ClearBuffs(player, map);
			if (difficulty > 1) 
            {
				//InstanceMap *instanceMap = map->ToInstanceMap();
				//InstanceScript *instanceScript = instanceMap->GetInstanceScript();

				//ChatHandler(player->GetSession()).PSendSysMessage("Entered %s (difficulty = %d, numInGroup = %d)",
					//map->GetMapName(), difficulty, numInGroup);

				_unitDifficulty[player->GetGUID()] = difficulty;
				for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i) 
                {
					player->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_VALUE, float(difficulty * 450), true);
				}
				player->SetFullHealth();
				if (player->getPowerType() == POWER_MANA) 
                {
					player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));
				}
			}
		}

		void ClearBuffs(Player *player, Map *map) 
        {
			std::map<ObjectGuid, int>::iterator unitDifficultyIterator = _unitDifficulty.find(player->GetGUID());
			if (unitDifficultyIterator != _unitDifficulty.end()) 
            {
				int difficulty = unitDifficultyIterator->second;
				_unitDifficulty.erase(unitDifficultyIterator);

				for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i) 
                {
					player->HandleStatModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_VALUE, float(difficulty * 450), false);
				}
			}
		}
	};
}


void AddSC_VAS_AutoBalance()
{
    new VAS_AutoBalance_WorldScript;
    new VAS_AutoBalance_PlayerScript;
    new VAS_AutoBalance_UnitScript;
    new VAS_AutoBalance_AllCreatureScript;
    new VAS_AutoBalance_AllMapScript;
    new VAS_AutoBalance_WorldMapScript;
    new solocraft_player_instance_handler();
    new DamageLoader();
}
