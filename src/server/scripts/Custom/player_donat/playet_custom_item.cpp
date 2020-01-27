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

#include "Define.h"
#include "GossipDef.h"
#include "Item.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "player_store_manager.h"
#include "WorldSession.h"
#include "ObjectMgr.h"
#include "GameEventMgr.h"
#include "InstanceSaveMgr.h"
#include "Pet.h"
#include "Language.h"
#include "ScriptMgr.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "AccountMgr.h"
#include "Chat.h"
#include <sstream>
#include <string>
#include <vector>

//      Creature        entry
#define CHIMERA         21879
#define CORE_HOUND      21108
#define DEVILSAUR       20931
#define RHINO           30445
#define SILITHID        5460
#define WORM            30148
#define LOQUE_NAHAK     32517
#define SKOLL           35189
#define GONDRIA         33776
#define SPIDER          2349
#define DRAGONHAWK      27946
#define BAT             28233
#define RAVAGER         17199
#define RAPTOR          14821
#define SERPENT         28358
#define BEAR            29319
#define BOAR            29996
#define WASP            28085
#define CAT             28097
#define CARRION_BIRD    26838
#define CRAB            24478
#define CROCOLISK       1417
#define GORILLA         28213
#define BLIGHT_HOUND    29452
#define HYENA           13036
#define MOTH            27421
#define OWL             23136
#define TALLSTRIDER     22807
#define SCORPID         9698
#define TURTLE          25482

//Gossip Menu
#define NORMAL_PET_PAGE_1           GOSSIP_ACTION_INFO_DEF + 1111
#define NORMAL_PET_PAGE_2           GOSSIP_ACTION_INFO_DEF + 222
#define EXOTIC_PET                  GOSSIP_ACTION_INFO_DEF + 333
#define STABLE_PET                  GOSSIP_ACTION_INFO_DEF + 444
#define RESET_PET_TALENTS           GOSSIP_ACTION_INFO_DEF + 555
#define RENAME_PET                  GOSSIP_ACTION_INFO_DEF + 666

#define EMOTE_ALREADY_ITEM "|cff00ffffУ вас уже имеется данная вещь!|r"
#define CONST_ARENA_RENAME 500
#define CONST_ARENA_CUSTOMIZE 300
#define CONST_ARENA_CHANGE_FACTION 1000
#define CONST_ARENA_CHANGE_RACE 500
#define MSG_RENAME_COMPLETE "|cff00ff00Оплата успешно произведена. Сделайте логаут и введите новое имя своего персонажа.Не забудьте после смены имени, выйти из игры и удалить из клиента папку 'Cache'!|r"
#define MSG_CUSTOMIZE_COMPLETE "|cff00ff00Оплата успешно произведена. Сделайте логаут и измените внешность своего персонажа.Не забудьте после смены внешности, выйти из игры и удалить из клиента папку 'Cache'!|r"
#define MSG_CHANGE_FACTION_COMPLETE "|cff00ff00Оплата успешно произведена. Сделайте логаут и измените фракцию своего персонажа.Не забудьте после смены фракции, выйти из игры и удалить из клиента папку 'Cache'!|r" 
#define MSG_CHANGE_RACE_COMPLETE "|cff00ff00Оплата успешно произведена. Сделайте логаут и измените расу своего персонажа.Не забудьте после смены расы, выйти из игры и удалить из клиента папку 'Cache'!|r"
#define EMOTE_NO_ARENA_POINTS "|cffff0000У вас недостаточно О.А. для совершения покупки!|r"
#define MSG_RESET_COOLDOWN "|cff0000ffВсе заклинания и способности успешно обновлены! КД сброшено!|r"
#define EMOTE_COOLDOWN "|cff00ffffВы не можете использовать функцию так часто!|r"
#define MSG_RESET_QUEST_STATUS_COMPLETE "|cff0000ffКД заданий успешно сброшено!|r"
#define MSG_RESET_RAID_INSTANCES_COOLDOWN_COMPLETE "|cff0000ffРейдовые подземелья успешно обновлены! КД сброшено!|r"
#define CONST_ARENA_POINT 100
#define CONST_ARENA_POINT2 1000

#define CONST_HONOR_POINT 2000
#define CONST_HONOR_POINT2 20000
#define CONST_HONOR_POINT3 30000
#define CONST_HONOR_POINT4 300000
#define CONST_HONOR_POINT5 200000

enum enus
{
    ARENA_MAX_RESULTS = 15
};

enum Costs
{
    // Ид Итемов
    TOKEN_ID = 49426, // Лёд
    TOKEN_IDD = 47241,  // Триумф
    token1 = 1,
    token2 = 5,
    token3 = 10,
    token4 = 20,
    token5 = 50,
    token6 = 600,
    token7 = 50,
    token8 = 150,
    token9 = 400, // 200k honor
    token10 = 125,
};

uint32 aurass[] = { 15366, 16609, 48162, 48074, 48170, 43223, 36880, 69994, 33081, 26035, 48469,57623, 47440 };

using namespace std;

class custom_item : public ItemScript
{

private:
    std::string accountQuery = "UPDATE account SET coin = %u WHERE id = %u;";
    std::string currentDP = "|TInterface\\icons\\Inv_misc_token_thrallmar:25:25:-15:0|tБонусов на аккаунте : ";
    std::string notEnoughDP = "У вас недостаточно бонусов, пополните их в личном кабинете";
    std::string dpQuery = "SELECT coin FROM account WHERE id=";
    std::string GuildQuery = "SELECT level, xp FROM guild_level WHERE guild = %u";
public:
    custom_item() : ItemScript("custom_item") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets)
    {
        if (player->GetMap()->IsBattlegroundOrArena())
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Вы не можете использовать Итем на арене или бг!");
            player->CLOSE_GOSSIP_MENU();
            return false;
        }
        
        if (player->IsInCombat()) 
        {
            ChatHandler(player->GetSession()).PSendSysMessage(LANG_YOU_IN_COMBAT);
            player->CLOSE_GOSSIP_MENU();
            return false;
        }

        if (player->IsInFlight())
        {
            ChatHandler(player->GetSession()).PSendSysMessage(LANG_YOU_IN_FLIGHT);
            player->CLOSE_GOSSIP_MENU();
            return false;
        }
        
        if (player->HasStealthAura())
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Вы не можете использовать итем в режиме незаметности!");
            player->CLOSE_GOSSIP_MENU();
            return false;
        }

        if (player->isDead() || player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Вы не можете использовать итем пока мертвы!");
            player->CLOSE_GOSSIP_MENU();
            return false;
        }

        player->PlayerTalkClass->ClearMenus();

        // показатель бонусов с системой определение кол.бонусов (бонус, бонуса, бонусов)
        uint32 coins = Maelstrom::sStoreMgr->GetAccountCoins(player->GetSession()->GetAccountId());
std::string msg = "|TInterface\\icons\\Inv_misc_token_thrallmar:25:25:-15:0|tУ вас |cffE80000" + std::to_string(coins) + "|r Бонусов";
        player->ADD_GOSSIP_ITEM(0, msg.c_str(), GOSSIP_SENDER_MAIN, 300000);
        player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Achievement_zone_grizzlyhills_01:25:25:-15:0|tВендоры", GOSSIP_SENDER_MAIN, 200500);
        //player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_holy_wordfortitude:25:25:-15:0|tПолучить баффы", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 150);
        player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_shadow_charm:25:25:-15:0|tСмена расовой способности", GOSSIP_SENDER_MAIN, 776);
        if (player->getClass() == CLASS_HUNTER)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|TInterface\\Icons\\Ability_Hunter_Pet_Rhino:25:25:-15:0|tПеты Для Охотников", GOSSIP_SENDER_MAIN, 100524);
        }
        player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/spell_holy_borrowedtime:25:25:-15:0|t Сбросить 'КД' рейдовых подземелий", GOSSIP_SENDER_MAIN, 1217);
        if (player->GetSession()->IsPremium())
        {
            player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Inv_inscription_pigment_bug07:25:25:-15:0|tУправление Вип Аккаунтом", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
        }
        if (!player->GetSession()->IsPremium())
        {
            player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Inv_inscription_pigment_bug07:25:25:-15:0|tВип Аккаунт [|cffE80000Купить|r]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
        }
        player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Inv_misc_tournaments_banner_scourge:25:25:-15:0|tУправление Персонажем", GOSSIP_SENDER_MAIN, 1245);
        player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\inv_misc_frostemblem_01:25:25:-15:0|tОбменник валют", GOSSIP_SENDER_MAIN, 1249);
        //player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Achievement_reputation_wyrmresttemple:25:25:-15:0|tУправление гильдией *NEW", GOSSIP_SENDER_MAIN, 1993);
        player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Inv_inscription_weaponscroll01:25:25:-15:0|tИнформация о сервере", GOSSIP_SENDER_MAIN, 777);
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
        return false;
    }

    bool PlayerAlreadyHasTwoProfessions(const Player *pPlayer) const
    {
        uint32 skillCount = 0;

        if (pPlayer->HasSkill(SKILL_MINING))
        skillCount++;
        if (pPlayer->HasSkill(SKILL_SKINNING))
        skillCount++;
        if (pPlayer->HasSkill(SKILL_HERBALISM))
        skillCount++;
        if (skillCount >= 2)
        return true;

        for (uint32 i = 1; i < sSkillLineStore.GetNumRows(); ++i)
        {
            SkillLineEntry const *SkillInfo = sSkillLineStore.LookupEntry(i);
            if (!SkillInfo)
            continue;

            if (SkillInfo->categoryId == SKILL_CATEGORY_SECONDARY)
            continue;

            if ((SkillInfo->categoryId != SKILL_CATEGORY_PROFESSION) || !SkillInfo->canLink)
            continue;

            const uint32 skillID = SkillInfo->id;
            if (pPlayer->HasSkill(skillID))
            skillCount++;

            if (skillCount >= 4)
            return true;
        }
        return false;
    }

    bool LearnAllRecipesInProfession(Player *pPlayer, SkillType skill)
    {
        ChatHandler handler(pPlayer->GetSession());
        char* skill_name;

        SkillLineEntry const *SkillInfo = sSkillLineStore.LookupEntry(skill);
        skill_name = SkillInfo->name[handler.GetSessionDbcLocale()];

        if (!SkillInfo)
        {
            //TC_LOG_ERROR("server.loading", "Profession NPC: received non-valid skill ID (LearnAllRecipesInProfession)");
        }

        LearnSkillRecipesHelper(pPlayer, SkillInfo->id);

        pPlayer->SetSkill(SkillInfo->id, pPlayer->GetSkillStep(SkillInfo->id), 450, 450);
        handler.PSendSysMessage(LANG_COMMAND_LEARN_ALL_RECIPES, skill_name);
        return true;
    }

    void LearnSkillRecipesHelper(Player *player, uint32 skill_id)
    {
        uint32 classmask = player->getClassMask();
        for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
        {
            SkillLineAbilityEntry const *skillLine = sSkillLineAbilityStore.LookupEntry(j);
            if (!skillLine)
            continue;

            // wrong skill
            if (skillLine->skillId != skill_id)
            continue;

            // not high rank
            if (skillLine->forward_spellid)
            continue;

            // skip racial skills
            if (skillLine->racemask != 0)
            continue;

            // skip wrong class skills
            if (skillLine->classmask && (skillLine->classmask & classmask) == 0)
            continue;

            SpellInfo const * spellInfo = sSpellMgr->GetSpellInfo(skillLine->spellId);
            if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, player, false))
            continue;

            player->LearnSpell(skillLine->spellId, false);
        }
    }

    bool IsSecondarySkill(SkillType skill) const
    {
        return skill == SKILL_COOKING || skill == SKILL_FIRST_AID;
    }

    void CompleteLearnProfession(Player *player, SkillType skill)
    {
        if (PlayerAlreadyHasTwoProfessions(player) && !IsSecondarySkill(skill))
        ChatHandler(player->GetSession()).PSendSysMessage("Вы уже изучили доступное число профессий!");
        else
        {
            if (!LearnAllRecipesInProfession(player, skill))
            TC_LOG_DEBUG("server", "Profession NPC: CompleteLearnProfession (LearnAllRecipesInProfession) : smthing wrong!");
        }
    }
    void OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();

        if (sender == GOSSIP_SENDER_MAIN)
        {
            switch (action)
            {
                
            case 776:
                player->PlayerTalkClass->ClearMenus();
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_shadow_charm:25:25:-15:0|tКаждый за себя", GOSSIP_SENDER_MAIN, 1776);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_holy_holyprotection:25:25:-15:0|tДар наару", GOSSIP_SENDER_MAIN, 1777);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_shadow_teleport:25:25:-15:0|tВолшебный поток", GOSSIP_SENDER_MAIN, 1778);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_shadow_raisedead:25:25:-15:0|tВоля Отрекшихся", GOSSIP_SENDER_MAIN, 1779);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_shadow_unholystrength:25:25:-15:0|tКаменная форма", GOSSIP_SENDER_MAIN, 1780);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Racial_orc_berserkerstrength:25:25:-15:0|tКровавое неистовство", GOSSIP_SENDER_MAIN, 1781);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Ability_ambush:25:25:-15:0|tСлиться с тенью", GOSSIP_SENDER_MAIN, 1782);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Racial_troll_berserk:25:25:-15:0|tБерсерк", GOSSIP_SENDER_MAIN, 1783);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Ability_rogue_trip:25:25:-15:0|tМастер побега", GOSSIP_SENDER_MAIN, 1784);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Ability_warstomp:25:25:-15:0|tГромовая поступь", GOSSIP_SENDER_MAIN, 1785);            
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                break;
            case 1776:
                player->RemoveSpell(28880);
                player->RemoveSpell(26297);
                player->RemoveSpell(25046);
                player->RemoveSpell(7744);
                player->RemoveSpell(59752);
                player->RemoveSpell(20594);
                player->RemoveSpell(20572);
                player->RemoveSpell(58984);
                player->RemoveSpell(20549);
                player->RemoveSpell(20589);
                player->RemoveSpell(33697);
                player->RemoveSpell(28730);
                player->RemoveSpell(59545);
                player->RemoveSpell(59544);
                player->RemoveSpell(59547);
                player->RemoveSpell(59543);
                player->RemoveSpell(59542);
                player->RemoveSpell(59543);
                player->RemoveSpell(59548);
                player->RemoveSpell(50613);
                player->RemoveSpell(28730);
                player->RemoveSpell(25046);
                player->RemoveSpell(20572);
                player->RemoveSpell(33702);
                player->RemoveSpell(33697);
                player->LearnSpell(59752, false);
                player->SaveToDB();
                player->PlayerTalkClass->SendCloseGossip();            
                break;
            case 1777:
                player->RemoveSpell(28880);
                player->RemoveSpell(26297);
                player->RemoveSpell(25046);
                player->RemoveSpell(7744);
                player->RemoveSpell(59752);
                player->RemoveSpell(20594);
                player->RemoveSpell(20572);
                player->RemoveSpell(58984);
                player->RemoveSpell(20549);
                player->RemoveSpell(20589);
                player->RemoveSpell(33697);
                player->RemoveSpell(28730);
                player->RemoveSpell(59545);
                player->RemoveSpell(59544);
                player->RemoveSpell(59547);
                player->RemoveSpell(59543);
                player->RemoveSpell(59542);
                player->RemoveSpell(59543);
                player->RemoveSpell(59548);
                player->RemoveSpell(50613);
                player->RemoveSpell(28730);
                player->RemoveSpell(25046);
                player->RemoveSpell(20572);
                player->RemoveSpell(33702);
                player->RemoveSpell(33697);
                if (player->getClass() == CLASS_DEATH_KNIGHT)
                {
                    player->LearnSpell(59545, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_WARRIOR || player->getClass() == CLASS_ROGUE)
                {
                    player->LearnSpell(28880, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_PRIEST)
                {
                    player->LearnSpell(59544, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_SHAMAN || player->getClass() == CLASS_DRUID)
                {
                    player->LearnSpell(59547, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_HUNTER)
                {
                    player->LearnSpell(59543, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_PALADIN)
                {
                    player->LearnSpell(59542, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_MAGE || player->getClass() == CLASS_WARLOCK)
                {
                    player->LearnSpell(59548, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                break;
            case 1778:
                player->RemoveSpell(28880);
                player->RemoveSpell(26297);
                player->RemoveSpell(25046);
                player->RemoveSpell(7744);
                player->RemoveSpell(59752);
                player->RemoveSpell(20594);
                player->RemoveSpell(20572);
                player->RemoveSpell(58984);
                player->RemoveSpell(20549);
                player->RemoveSpell(20589);
                player->RemoveSpell(33697);
                player->RemoveSpell(28730);
                player->RemoveSpell(59545);
                player->RemoveSpell(59544);
                player->RemoveSpell(59547);
                player->RemoveSpell(59543);
                player->RemoveSpell(59542);
                player->RemoveSpell(59543);
                player->RemoveSpell(59548);
                player->RemoveSpell(50613);
                player->RemoveSpell(28730);
                player->RemoveSpell(25046);
                if (player->getClass() == CLASS_DEATH_KNIGHT)
                {
                    player->LearnSpell(50613, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_WARRIOR || player->getClass() == CLASS_ROGUE)
                {
                    player->LearnSpell(25046, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_PRIEST)
                {
                    player->LearnSpell(28730, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_SHAMAN || player->getClass() == CLASS_DRUID)
                {
                    player->LearnSpell(28730, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_HUNTER)
                {
                    player->LearnSpell(28730, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_PALADIN)
                {
                    player->LearnSpell(28730, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_MAGE || player->getClass() == CLASS_WARLOCK)
                {
                    player->LearnSpell(28730, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                break;
            case 1779:
                player->RemoveSpell(28880);
                player->RemoveSpell(26297);
                player->RemoveSpell(25046);
                player->RemoveSpell(7744);
                player->RemoveSpell(59752);
                player->RemoveSpell(20594);
                player->RemoveSpell(20572);
                player->RemoveSpell(58984);
                player->RemoveSpell(20549);
                player->RemoveSpell(20589);
                player->RemoveSpell(33697);
                player->RemoveSpell(28730);
                player->RemoveSpell(59545);
                player->RemoveSpell(59544);
                player->RemoveSpell(59547);
                player->RemoveSpell(59543);
                player->RemoveSpell(59542);
                player->RemoveSpell(59543);
                player->RemoveSpell(59548);
                player->RemoveSpell(50613);
                player->RemoveSpell(28730);
                player->RemoveSpell(25046);
                player->RemoveSpell(20572);
                player->RemoveSpell(33702);
                player->RemoveSpell(33697);
                player->LearnSpell(7744, false);
                player->SaveToDB();
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1780:
                player->RemoveSpell(28880);
                player->RemoveSpell(26297);
                player->RemoveSpell(25046);
                player->RemoveSpell(7744);
                player->RemoveSpell(59752);
                player->RemoveSpell(20594);
                player->RemoveSpell(20572);
                player->RemoveSpell(58984);
                player->RemoveSpell(20549);
                player->RemoveSpell(20589);
                player->RemoveSpell(33697);
                player->RemoveSpell(28730);
                player->RemoveSpell(59545);
                player->RemoveSpell(59544);
                player->RemoveSpell(59547);
                player->RemoveSpell(59543);
                player->RemoveSpell(59542);
                player->RemoveSpell(59543);
                player->RemoveSpell(59548);
                player->RemoveSpell(50613);
                player->RemoveSpell(28730);
                player->RemoveSpell(25046);
                player->RemoveSpell(20572);
                player->RemoveSpell(33702);
                player->RemoveSpell(33697);
                player->LearnSpell(20594, false);
                player->SaveToDB();
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1781:
                player->RemoveSpell(28880);
                player->RemoveSpell(26297);
                player->RemoveSpell(25046);
                player->RemoveSpell(7744);
                player->RemoveSpell(59752);
                player->RemoveSpell(20594);
                player->RemoveSpell(20572);
                player->RemoveSpell(58984);
                player->RemoveSpell(20549);
                player->RemoveSpell(20589);
                player->RemoveSpell(33697);
                player->RemoveSpell(28730);
                player->RemoveSpell(59545);
                player->RemoveSpell(59544);
                player->RemoveSpell(59547);
                player->RemoveSpell(59543);
                player->RemoveSpell(59542);
                player->RemoveSpell(59543);
                player->RemoveSpell(59548);
                player->RemoveSpell(50613);
                player->RemoveSpell(28730);
                player->RemoveSpell(25046);
                player->RemoveSpell(20572);
                player->RemoveSpell(33702);
                player->RemoveSpell(33697);
                player->RemoveSpell(20572);
                player->RemoveSpell(33702);
                player->RemoveSpell(33697);
                if (player->getClass() == CLASS_DEATH_KNIGHT)
                {
                    player->LearnSpell(20572, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_WARRIOR || player->getClass() == CLASS_ROGUE)
                {
                    player->LearnSpell(20572, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_PRIEST)
                {
                    player->LearnSpell(33702, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_SHAMAN || player->getClass() == CLASS_DRUID)
                {
                    player->LearnSpell(33697, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_HUNTER)
                {
                    player->LearnSpell(20572, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_PALADIN)
                {
                    player->LearnSpell(33697, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else if (player->getClass() == CLASS_MAGE || player->getClass() == CLASS_WARLOCK)
                {
                    player->LearnSpell(33702, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                }
                break;
            case 1782:
                player->RemoveSpell(28880);
                player->RemoveSpell(26297);
                player->RemoveSpell(25046);
                player->RemoveSpell(7744);
                player->RemoveSpell(59752);
                player->RemoveSpell(20594);
                player->RemoveSpell(20572);
                player->RemoveSpell(58984);
                player->RemoveSpell(20549);
                player->RemoveSpell(20589);
                player->RemoveSpell(33697);
                player->RemoveSpell(28730);
                player->RemoveSpell(59545);
                player->RemoveSpell(59544);
                player->RemoveSpell(59547);
                player->RemoveSpell(59543);
                player->RemoveSpell(59542);
                player->RemoveSpell(59543);
                player->RemoveSpell(59548);
                player->RemoveSpell(50613);
                player->RemoveSpell(28730);
                player->RemoveSpell(25046);
                player->RemoveSpell(20572);
                player->RemoveSpell(33702);
                player->RemoveSpell(33697);
                player->LearnSpell(58984, false);
                player->SaveToDB();
                player->PlayerTalkClass->SendCloseGossip();            
                break;
            case 1783:
                player->RemoveSpell(28880);
                player->RemoveSpell(26297);
                player->RemoveSpell(25046);
                player->RemoveSpell(7744);
                player->RemoveSpell(59752);
                player->RemoveSpell(20594);
                player->RemoveSpell(20572);
                player->RemoveSpell(58984);
                player->RemoveSpell(20549);
                player->RemoveSpell(20589);
                player->RemoveSpell(33697);
                player->RemoveSpell(28730);
                player->RemoveSpell(59545);
                player->RemoveSpell(59544);
                player->RemoveSpell(59547);
                player->RemoveSpell(59543);
                player->RemoveSpell(59542);
                player->RemoveSpell(59543);
                player->RemoveSpell(59548);
                player->RemoveSpell(50613);
                player->RemoveSpell(28730);
                player->RemoveSpell(25046);
                player->RemoveSpell(20572);
                player->RemoveSpell(33702);
                player->RemoveSpell(33697);
                player->LearnSpell(26297, false);
                player->SaveToDB();
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1784:
                player->RemoveSpell(28880);
                player->RemoveSpell(26297);
                player->RemoveSpell(25046);
                player->RemoveSpell(7744);
                player->RemoveSpell(59752);
                player->RemoveSpell(20594);
                player->RemoveSpell(20572);
                player->RemoveSpell(58984);
                player->RemoveSpell(20549);
                player->RemoveSpell(20589);
                player->RemoveSpell(33697);
                player->RemoveSpell(28730);
                player->RemoveSpell(59545);
                player->RemoveSpell(59544);
                player->RemoveSpell(59547);
                player->RemoveSpell(59543);
                player->RemoveSpell(59542);
                player->RemoveSpell(59543);
                player->RemoveSpell(59548);
                player->RemoveSpell(50613);
                player->RemoveSpell(28730);
                player->RemoveSpell(25046);
                player->RemoveSpell(20572);
                player->RemoveSpell(33702);
                player->RemoveSpell(33697);
                player->LearnSpell(20589, false);
                player->SaveToDB();
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1785:
                player->RemoveSpell(28880);
                player->RemoveSpell(26297);
                player->RemoveSpell(25046);
                player->RemoveSpell(7744);
                player->RemoveSpell(59752);
                player->RemoveSpell(20594);
                player->RemoveSpell(20572);
                player->RemoveSpell(58984);
                player->RemoveSpell(20549);
                player->RemoveSpell(20589);
                player->RemoveSpell(33697);
                player->RemoveSpell(28730);
                player->RemoveSpell(59545);
                player->RemoveSpell(59544);
                player->RemoveSpell(59547);
                player->RemoveSpell(59543);
                player->RemoveSpell(59542);
                player->RemoveSpell(59543);
                player->RemoveSpell(59548);
                player->RemoveSpell(50613);
                player->RemoveSpell(28730);
                player->RemoveSpell(25046);
                player->RemoveSpell(20572);
                player->RemoveSpell(33702);
                player->RemoveSpell(33697);
                player->LearnSpell(20549, false);
                player->SaveToDB();
                player->PlayerTalkClass->SendCloseGossip();
                break;            
            case GOSSIP_ACTION_INFO_DEF + 150:
                if (player->HasAura(45523))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    player->GetSession()->SendAreaTriggerMessage("|cffC67171Нельзя использовать услугу так часто!");
                }
                else
                {

                    player->RemoveAurasByType(SPELL_AURA_MOUNTED);
                    for (int i = 0; i < 16; i++)
                    player->AddAura(aurass[i], player);
                    player->GetSession()->SendNotification("|cffC67171Отлично, вы получили баффы!");
                    player->CastSpell(player, 45523, true);
                    player->PlayerTalkClass->SendCloseGossip();
                }
                break;
            case 1245:
                player->PlayerTalkClass->ClearMenus();
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\achievement_level_80:25:25:-15:0|tКлассовые навыки|r", GOSSIP_SENDER_MAIN, 101);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\achievement_general:25:25:-15:0|tДвойная специализация|r", GOSSIP_SENDER_MAIN, 102);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\ability_marksmanship:25:25:-15:0|tСброс талантов|r", GOSSIP_SENDER_MAIN, 103);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Ability_Repair:25:25:-15:0|tПрофессии|r", GOSSIP_SENDER_MAIN, 104);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\spell_nature_swiftness:25:25:-15:0|tВерховая езда|r", GOSSIP_SENDER_MAIN, 105);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Ability_DualWield:25:25:-15:0|tОружейные навыки|r", GOSSIP_SENDER_MAIN, 106);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\trade_engineering:25:25:-15:0|tУлучшить навыки защиты и владения оружием до максимума|r", GOSSIP_SENDER_MAIN, 107);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|TInterface\\icons\\Spell_chargenegative:25:25:-15:0|tЗакрыть", GOSSIP_SENDER_MAIN, 200110);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                break;
            case 101: // Классовые навыки

                switch (player->getClass())
                {
                case CLASS_WARRIOR:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(7384, false);
                    player->LearnSpell(47436, false);
                    player->LearnSpell(47450, false);
                    player->LearnSpell(11578, false);
                    player->LearnSpell(47465, false);
                    player->LearnSpell(47502, false);
                    player->LearnSpell(34428, false);
                    player->LearnSpell(1715, false);
                    player->LearnSpell(2687, false);
                    player->LearnSpell(71, false);
                    player->LearnSpell(7386, false);
                    player->LearnSpell(355, false);
                    player->LearnSpell(72, false);
                    player->LearnSpell(47437, false);
                    player->LearnSpell(57823, false);
                    player->LearnSpell(694, false);
                    player->LearnSpell(2565, false);
                    player->LearnSpell(676, false);
                    player->LearnSpell(47520, false);
                    player->LearnSpell(20230, false);
                    player->LearnSpell(12678, false);
                    player->LearnSpell(47471, false);
                    player->LearnSpell(1161, false);
                    player->LearnSpell(871, false);
                    player->LearnSpell(2458, false);
                    player->LearnSpell(20252, false);
                    player->LearnSpell(47475, false);
                    player->LearnSpell(18499, false);
                    player->LearnSpell(1680, false);
                    player->LearnSpell(6552, false);
                    player->LearnSpell(47488, false);
                    player->LearnSpell(1719, false);
                    player->LearnSpell(23920, false);
                    player->LearnSpell(47440, false);
                    player->LearnSpell(3411, false);
                    player->LearnSpell(64382, false);
                    player->LearnSpell(55694, false);
                    player->LearnSpell(57755, false);
                    player->LearnSpell(42459, false);
                    player->LearnSpell(750, false);
                    player->LearnSpell(5246, false);
                    player->LearnSpell(3127, false);
                    if (player->HasSpell(12294))
                    player->LearnSpell(47486, false);
                    if (player->HasSpell(20243))
                    player->LearnSpell(47498, false);
                    player->SaveToDB();
                    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_PALADIN:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(3127, false);
                    player->LearnSpell(19746, false);
                    player->LearnSpell(19752, false);
                    player->LearnSpell(750, false);
                    player->LearnSpell(48942, false);
                    player->LearnSpell(48782, false);
                    player->LearnSpell(48932, false);
                    player->LearnSpell(20271, false);
                    player->LearnSpell(498, false);
                    player->LearnSpell(10308, false);
                    player->LearnSpell(1152, false);
                    player->LearnSpell(10278, false);
                    player->LearnSpell(48788, false);
                    player->LearnSpell(53408, false);
                    player->LearnSpell(48950, false);
                    player->LearnSpell(48936, false);
                    player->LearnSpell(31789, false);
                    player->LearnSpell(62124, false);
                    player->LearnSpell(54043, false);
                    player->LearnSpell(25780, false);
                    player->LearnSpell(1044, false);
                    player->LearnSpell(20217, false);
                    player->LearnSpell(48819, false);
                    player->LearnSpell(48801, false);
                    player->LearnSpell(48785, false);
                    player->LearnSpell(5502, false);
                    player->LearnSpell(20164, false);
                    player->LearnSpell(10326, false);
                    player->LearnSpell(1038, false);
                    player->LearnSpell(53407, false);
                    player->LearnSpell(48943, false);
                    player->LearnSpell(20165, false);
                    player->LearnSpell(48945, false);
                    player->LearnSpell(642, false);
                    player->LearnSpell(48947, false);
                    player->LearnSpell(20166, false);
                    player->LearnSpell(4987, false);
                    player->LearnSpell(48806, false);
                    player->LearnSpell(6940, false);
                    player->LearnSpell(48817, false);
                    player->LearnSpell(48934, false);
                    player->LearnSpell(48938, false);
                    player->LearnSpell(25898, false);
                    player->LearnSpell(32223, false);
                    player->LearnSpell(31884, false);
                    player->LearnSpell(54428, false);
                    player->LearnSpell(61411, false);
                    player->LearnSpell(53601, false);
                    player->LearnSpell(33388, false);
                    player->LearnSpell(33391, false);
                    if (player->HasSpell(20925))
                    player->LearnSpell(48952, false);
                    if (player->HasSpell(31935))
                    player->LearnSpell(48827, false);
                    if (player->HasSpell(20911))
                    player->LearnSpell(25899, false);
                    if (player->HasSpell(20473))
                    player->LearnSpell(48825, false);
                    if (player->GetTeam() == ALLIANCE)
                    {
                        player->LearnSpell(31801, false);
                        player->LearnSpell(13819, false);
                        player->LearnSpell(23214, false);
                    }
                    if (player->GetTeam() == HORDE)
                    {
                        player->LearnSpell(53736, false);
                        player->LearnSpell(34769, false);
                        player->LearnSpell(34767, false);
                    }
                    player->SaveToDB();
                    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_HUNTER:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(3043, false);
                    player->LearnSpell(3127, false);
                    player->LearnSpell(3045, false);
                    player->LearnSpell(3034, false);
                    player->LearnSpell(8737, false);
                    player->LearnSpell(1494, false);
                    player->LearnSpell(13163, false);
                    player->LearnSpell(48996, false);
                    player->LearnSpell(49001, false);
                    player->LearnSpell(49045, false);
                    player->LearnSpell(53338, false);
                    player->LearnSpell(5116, false);
                    player->LearnSpell(27044, false);
                    player->LearnSpell(883, false);
                    player->LearnSpell(2641, false);
                    player->LearnSpell(6991, false);
                    player->LearnSpell(982, false);
                    player->LearnSpell(1515, false);
                    player->LearnSpell(19883, false);
                    player->LearnSpell(20736, false);
                    player->LearnSpell(48990, false);
                    player->LearnSpell(2974, false);
                    player->LearnSpell(6197, false);
                    player->LearnSpell(1002, false);
                    player->LearnSpell(14327, false);
                    player->LearnSpell(5118, false);
                    player->LearnSpell(49056, false);
                    player->LearnSpell(53339, false);
                    player->LearnSpell(49048, false);
                    player->LearnSpell(19884, false);
                    player->LearnSpell(34074, false);
                    player->LearnSpell(781, false);
                    player->LearnSpell(14311, false);
                    player->LearnSpell(1462, false);
                    player->LearnSpell(19885, false);
                    player->LearnSpell(19880, false);
                    player->LearnSpell(13809, false);
                    player->LearnSpell(13161, false);
                    player->LearnSpell(5384, false);
                    player->LearnSpell(1543, false);
                    player->LearnSpell(19878, false);
                    player->LearnSpell(49067, false);
                    player->LearnSpell(3034, false);
                    player->LearnSpell(13159, false);
                    player->LearnSpell(19882, false);
                    player->LearnSpell(58434, false);
                    player->LearnSpell(49071, false);
                    player->LearnSpell(49052, false);
                    player->LearnSpell(19879, false);
                    player->LearnSpell(19263, false);
                    player->LearnSpell(19801, false);
                    player->LearnSpell(34026, false);
                    player->LearnSpell(34600, false);
                    player->LearnSpell(34477, false);
                    player->LearnSpell(61006, false);
                    player->LearnSpell(61847, false);
                    player->LearnSpell(53271, false);
                    player->LearnSpell(60192, false);
                    player->LearnSpell(62757, false);
                    if (player->HasSpell(19386))
                    player->LearnSpell(49012, false);
                    if (player->HasSpell(53301))
                    player->LearnSpell(60053, false);
                    if (player->HasSpell(19306))
                    player->LearnSpell(48999, false);
                    if (player->HasSpell(19434))
                    player->LearnSpell(49050, false);
                    player->SaveToDB();
                    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_MAGE:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(42921, false);
                    player->LearnSpell(42842, false);
                    player->LearnSpell(42995, false);
                    player->LearnSpell(42833, false);
                    player->LearnSpell(27090, false);
                    player->LearnSpell(33717, false);
                    player->LearnSpell(42873, false);
                    player->LearnSpell(42846, false);
                    player->LearnSpell(12826, false);
                    player->LearnSpell(28271, false);
                    player->LearnSpell(61780, false);
                    player->LearnSpell(61721, false);
                    player->LearnSpell(28272, false);
                    player->LearnSpell(42917, false);
                    player->LearnSpell(43015, false);
                    player->LearnSpell(130, false);
                    player->LearnSpell(42926, false);
                    player->LearnSpell(43017, false);
                    player->LearnSpell(475, false);
                    player->LearnSpell(1953, false);
                    player->LearnSpell(42940, false);
                    player->LearnSpell(12051, false);
                    player->LearnSpell(43010, false);
                    player->LearnSpell(43020, false);
                    player->LearnSpell(43012, false);
                    player->LearnSpell(42859, false);
                    player->LearnSpell(2139, false);
                    player->LearnSpell(42931, false);
                    player->LearnSpell(42985, false);
                    player->LearnSpell(43008, false);
                    player->LearnSpell(45438, false);
                    player->LearnSpell(43024, false);
                    player->LearnSpell(43002, false);
                    player->LearnSpell(43046, false);
                    player->LearnSpell(42897, false);
                    player->LearnSpell(42914, false);
                    player->LearnSpell(66, false);
                    player->LearnSpell(58659, false);
                    player->LearnSpell(30449, false);
                    player->LearnSpell(42956, false);
                    player->LearnSpell(47610, false);
                    player->LearnSpell(61316, false);
                    player->LearnSpell(61024, false);
                    player->LearnSpell(55342, false);
                    player->LearnSpell(53142, false);
                    player->LearnSpell(7301, false);
                    if (player->GetTeam() == ALLIANCE)
                    {
                        player->LearnSpell(32271, false); // Teleport: Exodar.
                        player->LearnSpell(49359, false); // Teleport: Theramore.
                        player->LearnSpell(3565, false); // Teleport: Darnassus.
                        player->LearnSpell(33690, false); // Teleport: Shattrath.
                        player->LearnSpell(3562, false); // Teleport: Ironforge.
                        player->LearnSpell(3561, false); // Teleport: Stormwind.
                        player->LearnSpell(53140, false); // Teleport: Dalaran.
                        player->LearnSpell(53142, false); // Portal: Dalaran.
                        player->LearnSpell(10059, false); // Portal: Stormwind.
                        player->LearnSpell(11419, false); // Portal: Darnassus.
                        player->LearnSpell(32266, false); // Portal: Exodar.
                        player->LearnSpell(11416, false); // Portal: Ironforge.
                        player->LearnSpell(33691, false); // Portal: Shattrath.
                        player->LearnSpell(49360, false); // Portal: Theramore.
                    }
                    if (player->GetTeam() == HORDE)
                    {
                        player->LearnSpell(3567, false); // Teleport: Orgrimmar.
                        player->LearnSpell(35715, false); // Teleport: Shattrath.
                        player->LearnSpell(3566, false); // Teleport: Thunder Bluff.
                        player->LearnSpell(49358, false); // Teleport: Stonard.
                        player->LearnSpell(32272, false); // Teleport: Silvermoon.
                        player->LearnSpell(3563, false); // Teleport: Undercity.
                        player->LearnSpell(53140, false); // Teleport: Dalaran.
                        player->LearnSpell(53142, false); // Portal: Dalaran.
                        player->LearnSpell(11417, false); // Portal: Orgrimmar.
                        player->LearnSpell(35717, false); // Portal: Shattrath.
                        player->LearnSpell(32267, false); // Portal: Silvermoon.
                        player->LearnSpell(49361, false); // Portal: Stonard.
                        player->LearnSpell(11420, false); // Portal: Thunder Bluff.
                        player->LearnSpell(11418, false); // Portal: Undercity.
                    }
                    if (player->HasSpell(11366))
                    player->LearnSpell(42891, false);
                    if (player->HasSpell(11426))
                    player->LearnSpell(43039, false);
                    if (player->HasSpell(44457))
                    player->LearnSpell(55360, false);
                    if (player->HasSpell(31661))
                    player->LearnSpell(42950, false);
                    if (player->HasSpell(11113))
                    player->LearnSpell(42945, false);
                    if (player->HasSpell(44425))
                    player->LearnSpell(44781, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_WARLOCK:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(696, false);
                    player->LearnSpell(47811, false);
                    player->LearnSpell(47809, false);
                    player->LearnSpell(688, false);
                    player->LearnSpell(47813, false);
                    player->LearnSpell(50511, false);
                    player->LearnSpell(57946, false);
                    player->LearnSpell(47864, false);
                    player->LearnSpell(6215, false);
                    player->LearnSpell(47878, false);
                    player->LearnSpell(47855, false);
                    player->LearnSpell(697, false);
                    player->LearnSpell(47856, false);
                    player->LearnSpell(47857, false);
                    player->LearnSpell(5697, false);
                    player->LearnSpell(47884, false);
                    player->LearnSpell(47815, false);
                    player->LearnSpell(47889, false);
                    player->LearnSpell(47820, false);
                    player->LearnSpell(698, false);
                    player->LearnSpell(712, false);
                    player->LearnSpell(126, false);
                    player->LearnSpell(5138, false);
                    player->LearnSpell(5500, false);
                    player->LearnSpell(11719, false);
                    player->LearnSpell(132, false);
                    player->LearnSpell(60220, false);
                    player->LearnSpell(18647, false);
                    player->LearnSpell(61191, false);
                    player->LearnSpell(47823, false);
                    player->LearnSpell(691, false);
                    player->LearnSpell(47865, false);
                    player->LearnSpell(47891, false);
                    player->LearnSpell(47888, false);
                    player->LearnSpell(17928, false);
                    player->LearnSpell(47860, false);
                    player->LearnSpell(47825, false);
                    player->LearnSpell(1122, false);
                    player->LearnSpell(47867, false);
                    player->LearnSpell(18540, false);
                    player->LearnSpell(47893, false);
                    player->LearnSpell(47838, false);
                    player->LearnSpell(29858, false);
                    player->LearnSpell(58887, false);
                    player->LearnSpell(47836, false);
                    player->LearnSpell(61290, false);
                    player->LearnSpell(48018, false);
                    player->LearnSpell(48020, false);
                    player->LearnSpell(33388, false);
                    player->LearnSpell(33391, false);
                    player->LearnSpell(23161, false);
                    if (player->HasSpell(17877))
                    player->LearnSpell(47827, false);
                    if (player->HasSpell(30283))
                    player->LearnSpell(47847, false);
                    if (player->HasSpell(30108))
                    player->LearnSpell(47843, false);
                    if (player->HasSpell(50796))
                    player->LearnSpell(59172, false);
                    if (player->HasSpell(48181))
                    player->LearnSpell(59164, false);
                    if (player->HasSpell(18220))
                    player->LearnSpell(59092, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_ROGUE:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(3127, false);
                    player->LearnSpell(42459, false);
                    player->LearnSpell(48668, false);
                    player->LearnSpell(48638, false);
                    player->LearnSpell(1784, false);
                    player->LearnSpell(48657, false);
                    player->LearnSpell(921, false);
                    player->LearnSpell(1776, false);
                    player->LearnSpell(26669, false);
                    player->LearnSpell(51724, false);
                    player->LearnSpell(6774, false);
                    player->LearnSpell(11305, false);
                    player->LearnSpell(1766, false);
                    player->LearnSpell(48676, false);
                    player->LearnSpell(48659, false);
                    player->LearnSpell(1804, false);
                    player->LearnSpell(8647, false);
                    player->LearnSpell(48691, false);
                    player->LearnSpell(51722, false);
                    player->LearnSpell(48672, false);
                    player->LearnSpell(1725, false);
                    player->LearnSpell(26889, false);
                    player->LearnSpell(2836, false);
                    player->LearnSpell(1833, false);
                    player->LearnSpell(1842, false);
                    player->LearnSpell(8643, false);
                    player->LearnSpell(2094, false);
                    player->LearnSpell(1860, false);
                    player->LearnSpell(57993, false);
                    player->LearnSpell(48674, false);
                    player->LearnSpell(31224, false);
                    player->LearnSpell(5938, false);
                    player->LearnSpell(57934, false);
                    player->LearnSpell(51723, false);
                    if (player->HasSpell(16511))
                    player->LearnSpell(48660, false);
                    if (player->HasSpell(1329))
                    player->LearnSpell(48666, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_PRIEST:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(528, false);
                    player->LearnSpell(2053, false);
                    player->LearnSpell(48161, false);
                    player->LearnSpell(48123, false);
                    player->LearnSpell(48125, false);
                    player->LearnSpell(48066, false);
                    player->LearnSpell(586, false);
                    player->LearnSpell(48068, false);
                    player->LearnSpell(48127, false);
                    player->LearnSpell(48171, false);
                    player->LearnSpell(48168, false);
                    player->LearnSpell(10890, false);
                    player->LearnSpell(6064, false);
                    player->LearnSpell(988, false);
                    player->LearnSpell(48300, false);
                    player->LearnSpell(6346, false);
                    player->LearnSpell(48071, false);
                    player->LearnSpell(48135, false);
                    player->LearnSpell(48078, false);
                    player->LearnSpell(453, false);
                    player->LearnSpell(10955, false);
                    player->LearnSpell(10909, false);
                    player->LearnSpell(8129, false);
                    player->LearnSpell(48073, false);
                    player->LearnSpell(605, false);
                    player->LearnSpell(48072, false);
                    player->LearnSpell(48169, false);
                    player->LearnSpell(552, false);
                    player->LearnSpell(1706, false);
                    player->LearnSpell(48063, false);
                    player->LearnSpell(48162, false);
                    player->LearnSpell(48170, false);
                    player->LearnSpell(48074, false);
                    player->LearnSpell(48158, false);
                    player->LearnSpell(48120, false);
                    player->LearnSpell(34433, false);
                    player->LearnSpell(48113, false);
                    player->LearnSpell(32375, false);
                    player->LearnSpell(64843, false);
                    player->LearnSpell(64901, false);
                    player->LearnSpell(53023, false);
                    if (player->HasSpell(34914))
                    player->LearnSpell(48160, false);
                    if (player->HasSpell(47540))
                    player->LearnSpell(53007, false);
                    if (player->HasSpell(724))
                    player->LearnSpell(48087, false);
                    if (player->HasSpell(19236))
                    player->LearnSpell(48173, false);
                    if (player->HasSpell(34861))
                    player->LearnSpell(48089, false);
                    if (player->HasSpell(15407))
                    player->LearnSpell(48156, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_DEATH_KNIGHT:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(3127, false);
                    player->LearnSpell(50842, false);
                    player->LearnSpell(49941, false);
                    player->LearnSpell(49930, false);
                    player->LearnSpell(47476, false);
                    player->LearnSpell(45529, false);
                    player->LearnSpell(3714, false);
                    player->LearnSpell(56222, false);
                    player->LearnSpell(48743, false);
                    player->LearnSpell(48263, false);
                    player->LearnSpell(49909, false);
                    player->LearnSpell(47528, false);
                    player->LearnSpell(45524, false);
                    player->LearnSpell(48792, false);
                    player->LearnSpell(57623, false);
                    player->LearnSpell(56815, false);
                    player->LearnSpell(47568, false);
                    player->LearnSpell(49895, false);
                    player->LearnSpell(50977, false);
                    player->LearnSpell(49576, false);
                    player->LearnSpell(49921, false);
                    player->LearnSpell(46584, false);
                    player->LearnSpell(49938, false);
                    player->LearnSpell(48707, false);
                    player->LearnSpell(48265, false);
                    player->LearnSpell(61999, false);
                    player->LearnSpell(42650, false);
                    player->LearnSpell(53428, false);
                    player->LearnSpell(53331, false);
                    player->LearnSpell(54447, false);
                    player->LearnSpell(53342, false);
                    player->LearnSpell(54446, false);
                    player->LearnSpell(53323, false);
                    player->LearnSpell(53344, false);
                    player->LearnSpell(70164, false);
                    player->LearnSpell(62158, false);
                    player->LearnSpell(33391, false);
                    player->LearnSpell(48778, false);
                    player->LearnSpell(51425, false);
                    player->LearnSpell(49924, false);
                    if (player->HasSpell(55050))
                    player->LearnSpell(55262, false);
                    if (player->HasSpell(49143))
                    player->LearnSpell(55268, false);
                    if (player->HasSpell(49184))
                    player->LearnSpell(51411, false);
                    if (player->HasSpell(55090))
                    player->LearnSpell(55271, false);
                    if (player->HasSpell(49158))
                    player->LearnSpell(51328, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_SHAMAN:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(2062, false);
                    player->LearnSpell(8737, false);
                    player->LearnSpell(49273, false);
                    player->LearnSpell(49238, false);
                    player->LearnSpell(10399, false);
                    player->LearnSpell(49231, false);
                    player->LearnSpell(58753, false);
                    player->LearnSpell(2484, false);
                    player->LearnSpell(49281, false);
                    player->LearnSpell(58582, false);
                    player->LearnSpell(49233, false);
                    player->LearnSpell(58790, false);
                    player->LearnSpell(58704, false);
                    player->LearnSpell(58643, false);
                    player->LearnSpell(49277, false);
                    player->LearnSpell(61657, false);
                    player->LearnSpell(8012, false);
                    player->LearnSpell(526, false);
                    player->LearnSpell(2645, false);
                    player->LearnSpell(57994, false);
                    player->LearnSpell(8143, false);
                    player->LearnSpell(49236, false);
                    player->LearnSpell(58796, false);
                    player->LearnSpell(58757, false);
                    player->LearnSpell(49276, false);
                    player->LearnSpell(57960, false);
                    player->LearnSpell(131, false);
                    player->LearnSpell(58745, false);
                    player->LearnSpell(6196, false);
                    player->LearnSpell(58734, false);
                    player->LearnSpell(58774, false);
                    player->LearnSpell(58739, false);
                    player->LearnSpell(58656, false);
                    player->LearnSpell(546, false);
                    player->LearnSpell(556, false);
                    player->LearnSpell(66842, false);
                    player->LearnSpell(51994, false);
                    player->LearnSpell(8177, false);
                    player->LearnSpell(58749, false);
                    player->LearnSpell(20608, false);
                    player->LearnSpell(36936, false);
                    player->LearnSpell(36936, false);
                    player->LearnSpell(58804, false);
                    player->LearnSpell(49271, false);
                    player->LearnSpell(8512, false);
                    player->LearnSpell(6495, false);
                    player->LearnSpell(8170, false);
                    player->LearnSpell(66843, false);
                    player->LearnSpell(55459, false);
                    player->LearnSpell(66844, false);
                    player->LearnSpell(3738, false);
                    player->LearnSpell(2894, false);
                    player->LearnSpell(60043, false);
                    player->LearnSpell(51514, false);
                    if (player->GetTeam() == ALLIANCE)
                    player->LearnSpell(32182, false);
                    if (player->GetTeam() == HORDE)
                    player->LearnSpell(2825, false);
                    if (player->HasSpell(61295))
                    player->LearnSpell(61301, false);
                    if (player->HasSpell(974))
                    player->LearnSpell(49284, false);
                    if (player->HasSpell(30706))
                    player->LearnSpell(57722, false);
                    if (player->HasSpell(51490))
                    player->LearnSpell(59159, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_DRUID:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(48378, false);
                    player->LearnSpell(48469, false);
                    player->LearnSpell(48461, false);
                    player->LearnSpell(48463, false);
                    player->LearnSpell(48441, false);
                    player->LearnSpell(53307, false);
                    player->LearnSpell(53308, false);
                    player->LearnSpell(5487, false);
                    player->LearnSpell(48560, false);
                    player->LearnSpell(6795, false);
                    player->LearnSpell(48480, false);
                    player->LearnSpell(53312, false);
                    player->LearnSpell(18960, false);
                    player->LearnSpell(5229, false);
                    player->LearnSpell(48443, false);
                    player->LearnSpell(50763, false);
                    player->LearnSpell(8983, false);
                    player->LearnSpell(8946, false);
                    player->LearnSpell(1066, false);
                    player->LearnSpell(48562, false);
                    player->LearnSpell(783, false);
                    player->LearnSpell(770, false);
                    player->LearnSpell(16857, false);
                    player->LearnSpell(18658, false);
                    player->LearnSpell(768, false);
                    player->LearnSpell(1082, false);
                    player->LearnSpell(5215, false);
                    player->LearnSpell(48477, false);
                    player->LearnSpell(49800, false);
                    player->LearnSpell(48465, false);
                    player->LearnSpell(48572, false);
                    player->LearnSpell(26995, false);
                    player->LearnSpell(48574, false);
                    player->LearnSpell(2782, false);
                    player->LearnSpell(50213, false);
                    player->LearnSpell(2893, false);
                    player->LearnSpell(33357, false);
                    player->LearnSpell(5209, false);
                    player->LearnSpell(48575, false);
                    player->LearnSpell(48447, false);
                    player->LearnSpell(48577, false);
                    player->LearnSpell(48579, false);
                    player->LearnSpell(5225, false);
                    player->LearnSpell(22842, false);
                    player->LearnSpell(49803, false);
                    player->LearnSpell(9634, false);
                    player->LearnSpell(20719, false);
                    player->LearnSpell(48467, false);
                    player->LearnSpell(29166, false);
                    player->LearnSpell(62600, false);
                    player->LearnSpell(22812, false);
                    player->LearnSpell(48470, false);
                    player->LearnSpell(33943, false);
                    player->LearnSpell(49802, false);
                    player->LearnSpell(48451, false);
                    player->LearnSpell(48568, false);
                    player->LearnSpell(33786, false);
                    player->LearnSpell(40120, false);
                    player->LearnSpell(62078, false);
                    player->LearnSpell(52610, false);
                    player->LearnSpell(50464, false);
                    player->LearnSpell(48570, false);
                    if (player->HasSpell(50516))
                    player->LearnSpell(61384, false);
                    if (player->HasSpell(48505))
                    player->LearnSpell(53201, false);
                    if (player->HasSpell(48438))
                    player->LearnSpell(53251, false);
                    if (player->HasSpell(5570))
                    player->LearnSpell(48468, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                switch (player->getRace())
                {
                case RACE_HUMAN:
                    player->LearnSpell(20598, false);
                    player->LearnSpell(20864, false);
                    player->LearnSpell(20597, false);
                    player->LearnSpell(59752, false);
                    player->LearnSpell(20599, false);
                    player->LearnSpell(58985, false);
                    break;
                case RACE_DRAENEI:
                    player->LearnSpell(6562, false);
                    player->LearnSpell(28875, false);
                    player->LearnSpell(59221, false);
                    if (player->getClass() == CLASS_DEATH_KNIGHT)
                    {
                        player->LearnSpell(59545, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_WARRIOR)
                    {
                        player->LearnSpell(28880, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_PRIEST)
                    {
                        player->LearnSpell(59544, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_SHAMAN)
                    {
                        player->LearnSpell(59547, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_HUNTER)
                    {
                        player->LearnSpell(59543, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_PALADIN)
                    {
                        player->LearnSpell(59542, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_MAGE)
                    {
                        player->LearnSpell(59548, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    break;
                case RACE_DWARF:
                    player->LearnSpell(20595, false);
                    player->LearnSpell(59224, false);
                    player->LearnSpell(20596, false);
                    player->LearnSpell(2481, false);
                    player->LearnSpell(20594, false);
                    break;
                case RACE_NIGHTELF:
                    player->LearnSpell(20583, false);
                    player->LearnSpell(58984, false);
                    player->LearnSpell(20582, false);
                    player->LearnSpell(20585, false);
                    break;
                case RACE_GNOME:
                    player->LearnSpell(20589, false);
                    player->LearnSpell(20591, false);
                    player->LearnSpell(20592, false);
                    player->LearnSpell(20593, false);
                    break;
                case RACE_ORC:
                    player->LearnSpell(20573, false);
                    player->LearnSpell(20574, false);
                    player->LearnSpell(21563, false);
                    if (player->getClass() == CLASS_DEATH_KNIGHT)
                    {
                        player->LearnSpell(20572, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_WARRIOR || player->getClass() == CLASS_ROGUE)
                    {
                        player->LearnSpell(20572, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_PRIEST)
                    {
                        player->LearnSpell(33702, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_SHAMAN || player->getClass() == CLASS_DRUID)
                    {
                        player->LearnSpell(33697, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_HUNTER)
                    {
                        player->LearnSpell(20572, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_PALADIN)
                    {
                        player->LearnSpell(33697, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_MAGE || player->getClass() == CLASS_WARLOCK)
                    {
                        player->LearnSpell(33702, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    break;
                case RACE_UNDEAD_PLAYER:
                    player->LearnSpell(7744, false);
                    player->LearnSpell(20577, false);
                    player->LearnSpell(5227, false);
                    player->LearnSpell(20579, false);
                    break;
                case RACE_TAUREN:
                    player->LearnSpell(20551, false);
                    player->LearnSpell(20552, false);
                    player->LearnSpell(20550, false);
                    player->LearnSpell(20549, false);
                    break;
                case RACE_TROLL:
                    player->LearnSpell(20557, false);
                    player->LearnSpell(20558, false);
                    player->LearnSpell(26290, false);
                    player->LearnSpell(20555, false);
                    player->LearnSpell(58943, false);
                    player->LearnSpell(26297, false);
                    break;
                case RACE_BLOODELF:
                    player->LearnSpell(822, false);
                    player->LearnSpell(28877, false);
                    if (player->getClass() == CLASS_DEATH_KNIGHT)
                    {
                        player->LearnSpell(50613, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_WARRIOR || player->getClass() == CLASS_ROGUE)
                    {
                        player->LearnSpell(25046, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_PRIEST)
                    {
                        player->LearnSpell(28730, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_SHAMAN || player->getClass() == CLASS_DRUID)
                    {
                        player->LearnSpell(28730, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_HUNTER)
                    {
                        player->LearnSpell(28730, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_PALADIN)
                    {
                        player->LearnSpell(28730, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_MAGE || player->getClass() == CLASS_WARLOCK)
                    {
                        player->LearnSpell(28730, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    break;
                }
                break;
                
            case 102: // Двойная специализация
                if (player->GetSpecsCount() == 1 && !(player->getLevel() < sWorld->getIntConfig(CONFIG_MIN_DUALSPEC_LEVEL)))
                player->CastSpell(player, 63680, true, NULL, NULL, player->GetGUID());
                player->CastSpell(player, 63624, true, NULL, NULL, player->GetGUID());
                player->CLOSE_GOSSIP_MENU();
                break;
            case 103: // Сброс талантов
                //Добавление меню Да / Нет
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|cff00ff00Да (Сбросить таланты)|r", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 123);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|cffff0000Нет (Закрыть окно)|r", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 124);
                player->PlayerTalkClass->SendGossipMenu(68, item->GetGUID());
                break;

                // Yes Menu
            case GOSSIP_ACTION_INFO_DEF + 123:
                player->ResetTalents(true);
                player->SendTalentsInfoData(false);
                player->CLOSE_GOSSIP_MENU();
                ChatHandler(player->GetSession()).PSendSysMessage("Ваши таланты успешно сброшены.");
                break;

                // No Menu
            case GOSSIP_ACTION_INFO_DEF + 124:
                player->CLOSE_GOSSIP_MENU();
                break;

            case 104:
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Trade_Alchemy:25:25:-15:0|t Алхимия.|r", GOSSIP_SENDER_MAIN, 1144);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Trade_BlackSmithing:25:25:-15:0|t Кузнечное дело.|r", GOSSIP_SENDER_MAIN, 1145);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/INV_Misc_ArmorKit_17:25:25:-15:0|t Кожевничество.|r", GOSSIP_SENDER_MAIN, 1146);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Trade_Tailoring:25:25:-15:0|t Портняжное дело.|r", GOSSIP_SENDER_MAIN, 1147);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Trade_Engineering:25:25:-15:0|t Инженерное дело.|r", GOSSIP_SENDER_MAIN, 1148);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Trade_Engraving:25:25:-15:0|t Наложение чар.|r", GOSSIP_SENDER_MAIN, 1149);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/INV_Misc_Gem_01:25:25:-15:0|t Ювелирное дело.|r", GOSSIP_SENDER_MAIN, 2150);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/INV_Inscription_Tradeskill01:25:25:-15:0|t Начертание.|r", GOSSIP_SENDER_MAIN, 1151);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Spell_Nature_NatureTouchGrow:25:25:-15:0|t Травничество.|r", GOSSIP_SENDER_MAIN, 1152);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/INV_Misc_Pelt_Wolf_01:25:25:-15:0|t Снятие шкур.|r", GOSSIP_SENDER_MAIN, 1153);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Trade_Mining:25:25:-15:0|t Горное дело.|r", GOSSIP_SENDER_MAIN, 1154);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/INV_Misc_Food_15:25:25:-15:0|t Кулинария.|r", GOSSIP_SENDER_MAIN, 1155);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Spell_Holy_SealOfSacrifice:25:25:-15:0|t Первая помощь.|r", GOSSIP_SENDER_MAIN, 1156);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Trade_Fishing:25:25:-15:0|t Рыбная ловля.|r", GOSSIP_SENDER_MAIN, 1157);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargenegative:25:25:-15:0|tЗакрыть", GOSSIP_SENDER_MAIN, 200110);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                break;
            case 1144:
                if (player->HasSkill(SKILL_ALCHEMY))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы уже освоили данную профессию!");
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                CompleteLearnProfession(player, SKILL_ALCHEMY);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1145:
                if (player->HasSkill(SKILL_BLACKSMITHING))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы уже освоили данную профессию!");
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                CompleteLearnProfession(player,  SKILL_BLACKSMITHING);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1146:
                if (player->HasSkill(SKILL_LEATHERWORKING))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы уже освоили данную профессию!");
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                CompleteLearnProfession(player, SKILL_LEATHERWORKING);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1147:
                if (player->HasSkill(SKILL_TAILORING))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы уже освоили данную профессию!");
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                CompleteLearnProfession(player, SKILL_TAILORING);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1148:
                if (player->HasSkill(SKILL_ENGINEERING))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы уже освоили данную профессию!");
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                CompleteLearnProfession(player, SKILL_ENGINEERING);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1149:
                if (player->HasSkill(SKILL_ENCHANTING))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы уже освоили данную профессию!");
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                CompleteLearnProfession(player, SKILL_ENCHANTING);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 2150:
                if (player->HasSkill(SKILL_JEWELCRAFTING))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы уже освоили данную профессию!");
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                CompleteLearnProfession(player, SKILL_JEWELCRAFTING);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1151:
                if (player->HasSkill(SKILL_INSCRIPTION))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы уже освоили данную профессию!");
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                CompleteLearnProfession(player, SKILL_INSCRIPTION);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1152:
                if (player->HasSkill(SKILL_HERBALISM))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы уже освоили данную профессию!");
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }

                CompleteLearnProfession(player, SKILL_HERBALISM);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1153:
                if (player->HasSkill(SKILL_SKINNING))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы уже освоили данную профессию!");
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }

                CompleteLearnProfession(player, SKILL_SKINNING);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1154:
                if (player->HasSkill(SKILL_MINING))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы уже освоили данную профессию!");
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }

                CompleteLearnProfession(player, SKILL_MINING);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1155:
                if (player->HasSkill(SKILL_COOKING))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы уже освоили данную профессию!");
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }

                CompleteLearnProfession(player, SKILL_COOKING);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1156:
                if (player->HasSkill(SKILL_FIRST_AID))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы уже освоили данную профессию!");
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }

                CompleteLearnProfession(player, SKILL_FIRST_AID);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 1157:
                if (player->HasSkill(SKILL_FISHING))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы уже освоили данную профессию!");
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }

                CompleteLearnProfession(player, SKILL_FISHING);
                player->PlayerTalkClass->SendCloseGossip();
                break;

            case 1217: // Сброс КД рейдовых подземелий
                {
                    for (uint8 i = 0; i < MAX_DIFFICULTY; ++i)
                    {
                        Player::BoundInstancesMap &binds = player->GetBoundInstances(Difficulty(i));
                        for (Player::BoundInstancesMap::iterator itr = binds.begin(); itr != binds.end();)
                        {
                            player->UnbindInstance(itr, Difficulty(i));
                        }
                    }
                    ChatHandler(player->GetSession()).PSendSysMessage(MSG_RESET_RAID_INSTANCES_COOLDOWN_COMPLETE);
                    player->CLOSE_GOSSIP_MENU();
                }
                break;
            case 105: // Верховая езда
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, 33389, false); // Apprentice Riding
                player->CastSpell(player, 33392, false); // Journeyman Riding
                player->CastSpell(player, 34092, false); // Expert Riding
                player->CastSpell(player, 34093, false); // Artisan Riding
                player->LearnSpell(54197, false); // Allows you to ride flying mounts in Northrend.
                ChatHandler(player->GetSession()).PSendSysMessage("Ваши навыки верховой езды улучшены до максимума!");
                break;
            case 106: // Оружейные навыки
                switch (player->getClass())
                {
                case CLASS_WARRIOR:
                    player->CLOSE_GOSSIP_MENU();
                    player->LearnSpell(5011, false); // Арбалеты
                    player->LearnSpell(200, false); // Древковое оружие
                    player->LearnSpell(15590, false); // Кулачное оружие
                    player->LearnSpell(264, false); // Луки
                    player->LearnSpell(266, false); // Ружья
                    player->LearnSpell(227, false); // Посохи
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы успешно изучили все оружейные навыки для своего класса");
                    break;
                case CLASS_PALADIN:
                    player->CLOSE_GOSSIP_MENU();
                    player->LearnSpell(197, false); // Двуручные топоры
                    player->LearnSpell(200, false); // Древковое оружие
                    player->LearnSpell(196, false); // Одноручные топоры
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы успешно изучили все оружейные навыки для своего класса");
                    break;
                case CLASS_WARLOCK:
                    player->CLOSE_GOSSIP_MENU();
                    player->LearnSpell(201, false); // Одноручные мечи
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы успешно изучили все оружейные навыки для своего класса");
                    break;
                case CLASS_PRIEST:
                    player->CLOSE_GOSSIP_MENU();
                    player->LearnSpell(1180, false); // Кинжалы
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы успешно изучили все оружейные навыки для своего класса");
                    break;
                case CLASS_HUNTER:
                    player->CLOSE_GOSSIP_MENU();
                    player->LearnSpell(5011, false); // Арбалеты
                    player->LearnSpell(202, false); // Двуручные мечи
                    player->LearnSpell(200, false); // Древковое оружие
                    player->LearnSpell(15590, false); // Кулачное оружие
                    player->LearnSpell(264, false); // Луки
                    player->LearnSpell(2567, false); // Метательное оружие
                    player->LearnSpell(227, false); // Посохи
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы успешно изучили все оружейные навыки для своего класса");
                    break;
                case CLASS_MAGE:
                    player->CLOSE_GOSSIP_MENU();
                    player->LearnSpell(1180, false); // Кинжалы
                    player->LearnSpell(201, false); // Одноручные мечи
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы успешно изучили все оружейные навыки для своего класса");
                    break;
                case CLASS_SHAMAN:
                    player->CLOSE_GOSSIP_MENU();
                    player->LearnSpell(199, false); // Двуручное дробящее оружие
                    player->LearnSpell(197, false); // Двуручные топоры
                    player->LearnSpell(1180, false); // Кинжалы
                    player->LearnSpell(15590, false); // Кулачное оружие
                    player->LearnSpell(196, false); // Одноручные топоры
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы успешно изучили все оружейные навыки для своего класса");
                    break;
                case CLASS_ROGUE:
                    player->CLOSE_GOSSIP_MENU();
                    player->LearnSpell(5011, false); // Арбалеты
                    player->LearnSpell(198, false); // Одноручное дробящее оружие
                    player->LearnSpell(15590, false); // Кулачное оружие
                    player->LearnSpell(264, false); // Луки
                    player->LearnSpell(201, false); // Одноручные мечи
                    player->LearnSpell(266, false); // Ружья
                    player->LearnSpell(196, false); // Одноручные топоры
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы успешно изучили все оружейные навыки для своего класса");
                    break;
                case CLASS_DEATH_KNIGHT:
                    player->CLOSE_GOSSIP_MENU();
                    player->LearnSpell(199, false); // Двуручное дробящее оружие
                    player->LearnSpell(198, false); // Одноручное дробящее оружие
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы успешно изучили все оружейные навыки для своего класса");
                    break;
                case CLASS_DRUID:
                    player->CLOSE_GOSSIP_MENU();
                    player->LearnSpell(199, false); // Двуручное дробящее оружие
                    player->LearnSpell(200, false); // Древковое оружие
                    player->LearnSpell(15590, false); // Кулачное оружие
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы успешно изучили все оружейные навыки для своего класса");
                    break;
                }
            case 107: // Улучшить навыки защиты и владения оружием до максимума
                player->CLOSE_GOSSIP_MENU();
                player->UpdateSkillsToMaxSkillsForLevel();
                ChatHandler(player->GetSession()).PSendSysMessage("Ваши навыки защиты и владения оружием улучшены до максимума.");
                break;
            case 300000:
                {
                    player->PlayerTalkClass->ClearMenus();
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, "|TInterface\\icons\\Inv_misc_coin_01:25:25:-15:0|tИнформация", GOSSIP_SENDER_MAIN, 300001);
                    if (player->getLevel() < 80)
                    {
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, "|TInterface\\icons\\Inv_misc_coin_01:25:25:-15:0|tКупить 80 lvl", GOSSIP_SENDER_MAIN, 300222);
                    }
                    // player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, "|TInterface\\icons\\Inv_misc_token_thrallmar:25:25:-15:0|tМагазин", GOSSIP_SENDER_MAIN, 200100);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|TInterface\\icons\\Spell_chargenegative:25:25:-15:0|tЗакрыть", GOSSIP_SENDER_MAIN, 200110);
                    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                }
                break;
            case 300222:
                {

                    QueryResult result1 = LoginDatabase.PQuery("SELECT coin FROM account WHERE id = %u", player->GetSession()->GetAccountId());
                    if (result1)
                    {

                        Field *fields = result1->Fetch();
                        uint32 coins = fields[0].GetUInt32();
                        uint32 ostatok = 50 - coins; 			

                        if (coins >= 50)
                        {
                            player->GiveLevel(80);
                            LoginDatabase.PQuery("UPDATE account SET coin = coin-50 WHERE id = %u", player->GetSession()->GetAccountId());
                            player->SaveToDB();
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699Вы успешно повысили свой уровень!|r");
                            player->PlayerTalkClass->SendCloseGossip();
                        }
                        else
                        {
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699Недостаточно бонусов|r");
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699У вас: |cff00FF33%u|r", coins);
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699Нужно еще: |cffFF0000%u|r", ostatok);
                            player->PlayerTalkClass->SendCloseGossip();
                        }
                    }
                }
                break;
            
            case 100800:
                player->CLOSE_GOSSIP_MENU();
                player->GetSession()->SendShowBank(player->GetGUID());
                break;

            case 300001:
                player->PlayerTalkClass->ClearMenus();
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, "|TInterface\\icons\\Inv_misc_coin_01:25:25:-15:0|tИнформация", GOSSIP_SENDER_MAIN, 300000);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, "|TInterface\\icons\\Inv_misc_coin_01:25:25:-15:0|tКак это работает ?: Чтобы получить бонусы вы можете приобрести в личном кабинете.", GOSSIP_SENDER_MAIN, 300000);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, "|TInterface\\icons\\Inv_misc_coin_01:25:25:-15:0|tКупить:[1 бонус -> 1 руб].", GOSSIP_SENDER_MAIN, 300000);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, "|TInterface\\icons\\Inv_misc_coin_01:25:25:-15:0|tСайт: Wow-idk.ru", GOSSIP_SENDER_MAIN, 300000);
                player->SEND_GOSSIP_MENU(1, item->GetGUID());
                break;

            
            case 200110:
                player->CLOSE_GOSSIP_MENU();
                break;

            case GOSSIP_ACTION_INFO_DEF + 1:

                player->ADD_GOSSIP_ITEM(0, "Панда", GOSSIP_SENDER_MAIN, 1);
                player->ADD_GOSSIP_ITEM(0, "Человек Женский Пол", GOSSIP_SENDER_MAIN, 2);
                player->ADD_GOSSIP_ITEM(0, "Человек Мужской Пол", GOSSIP_SENDER_MAIN, 3);
                player->ADD_GOSSIP_ITEM(0, "Дворф Мужской Пол", GOSSIP_SENDER_MAIN, 5);
                player->ADD_GOSSIP_ITEM(0, "Темный Эльф Мужской Пол", GOSSIP_SENDER_MAIN, 7);
                player->ADD_GOSSIP_ITEM(0, "Гном Женский Пол", GOSSIP_SENDER_MAIN, 8);
                player->ADD_GOSSIP_ITEM(0, "Гном Мужской Пол", GOSSIP_SENDER_MAIN, 9);
                player->ADD_GOSSIP_ITEM(0, "Дреней Женский Пол", GOSSIP_SENDER_MAIN, 10);
                player->ADD_GOSSIP_ITEM(0, "Таурен Женский Пол", GOSSIP_SENDER_MAIN, 12);
                player->ADD_GOSSIP_ITEM(0, "Таурен Мужской Пол", GOSSIP_SENDER_MAIN, 13);
                player->ADD_GOSSIP_ITEM(0, "Эльф Крови Женский Пол", GOSSIP_SENDER_MAIN, 14);
                player->ADD_GOSSIP_ITEM(0, "Эльф Крови Мужской Пол", GOSSIP_SENDER_MAIN, 15);
                player->ADD_GOSSIP_ITEM(0, "Троль Мужской Пол", GOSSIP_SENDER_MAIN, 17);
                player->ADD_GOSSIP_ITEM(0, "Гоблин", GOSSIP_SENDER_MAIN, 16);
                player->ADD_GOSSIP_ITEM(0, "Эльф Крови Женский Пол (1)", GOSSIP_SENDER_MAIN, 18);
                player->ADD_GOSSIP_ITEM(0, "Эльф Крови Мужской Пол (1)", GOSSIP_SENDER_MAIN, 19);
                player->ADD_GOSSIP_ITEM(0, "Орк Женский Пол", GOSSIP_SENDER_MAIN, 20);
                player->ADD_GOSSIP_ITEM(0, "Орк Женский Пол (1)", GOSSIP_SENDER_MAIN, 21);
                player->ADD_GOSSIP_ITEM(0, "Гном в очках Мужской Пол", GOSSIP_SENDER_MAIN, 22);
                player->ADD_GOSSIP_ITEM(0, "Медив", GOSSIP_SENDER_MAIN, 23);
                player->ADD_GOSSIP_ITEM(0, "Ворген", GOSSIP_SENDER_MAIN, 24);
                player->ADD_GOSSIP_ITEM(0, "Свала вечноскорбящая", GOSSIP_SENDER_MAIN, 30);
                player->ADD_GOSSIP_ITEM(0, "Иллидан", GOSSIP_SENDER_MAIN, 31);
                player->ADD_GOSSIP_ITEM(0, "Всадник", GOSSIP_SENDER_MAIN, 32);
                player->ADD_GOSSIP_ITEM(0, "Король-Лич", GOSSIP_SENDER_MAIN, 33);
                player->ADD_GOSSIP_ITEM(0, "Слаер", GOSSIP_SENDER_MAIN, 34);
                player->ADD_GOSSIP_ITEM(0, "Закрыть", GOSSIP_SENDER_MAIN, 200110);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                break;

            case 1:
                player->SetDisplayId(30414);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 2:
                player->SetDisplayId(19724);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 3:
                player->SetDisplayId(19723);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 5:
                player->SetDisplayId(20317);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 7:
                player->SetDisplayId(20318);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 8:
                player->SetDisplayId(20320);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 9:
                player->SetDisplayId(20580);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 10:
                player->SetDisplayId(20323);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 11:
                player->SetDisplayId(37916);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 12:
                player->SetDisplayId(20584);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 13:
                player->SetDisplayId(20585);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 14:
                player->SetDisplayId(20370);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 15:
                player->SetDisplayId(20369);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 16:
                player->SetDisplayId(20326);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 17:
                player->SetDisplayId(20321);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 18:
                player->SetDisplayId(20352);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 19:
                player->SetDisplayId(20351);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 20:
                player->SetDisplayId(20316);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 21:
                player->SetDisplayId(20454);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 22:
                player->SetDisplayId(20646);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 23:
                player->SetDisplayId(18718);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 24:
                player->SetDisplayId(23996);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 30:
                player->SetDisplayId(26096);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 31:
                player->SetDisplayId(21135);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 32:
                player->SetDisplayId(16416);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 33:
                player->SetDisplayId(25191);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 34:
                player->SetDisplayId(15301);
                player->PlayerTalkClass->SendCloseGossip();
                break;

            case GOSSIP_ACTION_INFO_DEF + 2:
                player->DeMorph();                
                player->PlayerTalkClass->SendCloseGossip();
                break;

            case GOSSIP_ACTION_INFO_DEF + 5:
                {
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Inv_misc_note_02:25:25:-15:0|tСписок доступных комманд", GOSSIP_SENDER_MAIN, 210);
					player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Achievement_zone_sholazar_01:25:25:-15:0|tТелепортер", GOSSIP_SENDER_MAIN, 100521);
					player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Inv_brd_banner:25:25:-15:0|tТрансмогрификация", GOSSIP_SENDER_MAIN, 100522);
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Ability_mount_rocketmountblue:25:25:-15:0|tПолучить Вип Маунта", GOSSIP_SENDER_MAIN, 212);
                    player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/inv_crate_04:25:25:-15:0|t Выдача сумок", GOSSIP_SENDER_MAIN, 218);
                    player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/inv_shirt_red_01:25:25:-15:0|t Выдача временой рубашки", GOSSIP_SENDER_MAIN, 219);
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_magic_polymorphchicken:25:25:-15:0|tМорфы", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Ability_druid_cower:25:25:-15:0|tCнять Морф", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Inv_throwingknife_02:25:25:-15:0|tПочинить всю экипировку", GOSSIP_SENDER_MAIN, 216);
                    player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/ability_druid_cower:25:25:-15:0|t VIP Баффы", GOSSIP_SENDER_MAIN, 217);
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\spell_shadow_deathscream:25:25:-15:0|tСнять Слабость", GOSSIP_SENDER_MAIN, 215);
                    player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/spell_holy_rapture:25:25:-15:0|t Изменение персонажа ->", GOSSIP_SENDER_MAIN, 1205);
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25:25:-15:0|tБанк", GOSSIP_SENDER_MAIN, 214);
                    if (!player ->IsInCombat() || !player ->IsInFlight() || !player ->GetMap()->IsBattlegroundOrArena() || !player ->HasStealthAura() || !player ->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || !player ->isDead() || player ->GetAreaId() != 616)
                    {
                        player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/spell_holy_borrowedtime:25:25:-15:0|t Сбросить 'КД'", GOSSIP_SENDER_MAIN, 1212);
                        player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/spell_holy_divineillumination:25:25:-15:0|t Исцеление", GOSSIP_SENDER_MAIN, 1202);
                    }                   
                    player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/spell_holy_borrowedtime:25:25:-15:0|t Сбросить 'КД' заданий", GOSSIP_SENDER_MAIN, 1215);
                    player->ADD_GOSSIP_ITEM(0, "|TInterface/icons/Spell_chargepositive:25:25:-15:0|tСменить Цвет Чата", GOSSIP_SENDER_MAIN, 196);
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargenegative:25:25:-15:0|tЗакрыть", GOSSIP_SENDER_MAIN, 200110);
                    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                }
                break;

            case 196:
                {
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cffD00000Красный|r", GOSSIP_SENDER_MAIN, 1010);
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cff3333CCСиний|r", GOSSIP_SENDER_MAIN, 1011);	
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cff009900Зелёный|r", GOSSIP_SENDER_MAIN, 1012);	
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cff0099FFГолубой|r", GOSSIP_SENDER_MAIN, 1013);	
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cffFF99FFРозовый|r", GOSSIP_SENDER_MAIN, 1014); 
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cff9192a1Серый|r", GOSSIP_SENDER_MAIN, 1015); 
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cffb37700Коричневый|r", GOSSIP_SENDER_MAIN, 1016); 
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cffffd9b3Бежевый|r", GOSSIP_SENDER_MAIN, 1017); 
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargenegative:25|t|Вернуть обычный цвет", GOSSIP_SENDER_MAIN, 1018); 
                    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());			
                }
                break;
            case 1202: // Исцеление
                if (player->HasAura(45523))
                {
                    player->GetSession()->SendAreaTriggerMessage(EMOTE_COOLDOWN);
                    player->CLOSE_GOSSIP_MENU();
                }
                else
                {
                    player->CastSpell(player, 25840, true);
                    player->CastSpell(player, 45523, true);
                    player->CLOSE_GOSSIP_MENU();
                }
                break;    
            case 1010: // Красный
                {
                    LoginDatabase.PExecute("UPDATE account SET color = '1' WHERE id = %u", player->GetSession()->GetAccountId());	// |cffD00000 цвет
                    ChatHandler(player->GetSession()).PSendSysMessage("|cffD00000Вы успешно изменили свой цвет чата на: Красный");
                    player->PlayerTalkClass->SendCloseGossip();
                }
                break;
                
            case 1011: // Синий
                {
                    LoginDatabase.PExecute("UPDATE account SET color = '2' WHERE id = %u", player->GetSession()->GetAccountId());	// |cff3300CC цвет
                    ChatHandler(player->GetSession()).PSendSysMessage("|cff3333CCВы успешно изменили свой цвет чата на: Синий");
                    player->PlayerTalkClass->SendCloseGossip();
                }
                break;
                
            case 1012: // Зелёный
                {
                    LoginDatabase.PExecute("UPDATE account SET color = '3' WHERE id = %u", player->GetSession()->GetAccountId());	// |cff009900 цвет
                    ChatHandler(player->GetSession()).PSendSysMessage("|cff009900Вы успешно изменили свой цвет чата на: Зелёный");
                    player->PlayerTalkClass->SendCloseGossip();
                }
                break;
                
            case 1013: // Голубой
                {
                    LoginDatabase.PExecute("UPDATE account SET color = '4' WHERE id = %u", player->GetSession()->GetAccountId()); // |cff0099FF цвет
                    ChatHandler(player->GetSession()).PSendSysMessage("|cff0099FFВы успешно изменили свой цвет чата на: Голубой");				
                    player->PlayerTalkClass->SendCloseGossip();
                }
                break;
                
            case 1014: // Розовый
                {
                    LoginDatabase.PExecute("UPDATE account SET color = '5' WHERE id = %u", player->GetSession()->GetAccountId()); // |cffFF99FF цвет
                    ChatHandler(player->GetSession()).PSendSysMessage("|cffFF99FFВы успешно изменили свой цвет чата на: Розовый");				
                    player->PlayerTalkClass->SendCloseGossip();
                }
                break;
                
            case 1015: // серый
                {
                    LoginDatabase.PExecute("UPDATE account SET color = '7' WHERE id = %u", player->GetSession()->GetAccountId());
                    ChatHandler(player->GetSession()).PSendSysMessage("|cff9192a1Вы успешно изменили свой цвет чата на: Серый");
                    player->PlayerTalkClass->SendCloseGossip();
                }
                break;	
                
            case 1016: // Коричневый
                {
                    LoginDatabase.PExecute("UPDATE account SET color = '8' WHERE id = %u", player->GetSession()->GetAccountId());
                    ChatHandler(player->GetSession()).PSendSysMessage("|cffb37700Вы успешно изменили свой цвет чата на: Коричневый");
                    player->PlayerTalkClass->SendCloseGossip();
                }
                break;	

            case 1017: // Бежевый
                {
                    LoginDatabase.PExecute("UPDATE account SET color = '9' WHERE id = %u", player->GetSession()->GetAccountId());
                    ChatHandler(player->GetSession()).PSendSysMessage("|cffffd9b3Вы успешно изменили свой цвет чата на: Бежевый");
                    player->PlayerTalkClass->SendCloseGossip();
                }
                break;	

            case 1018: // обычный
                {
                    LoginDatabase.PExecute("UPDATE account SET color = '0' WHERE id = %u", player->GetSession()->GetAccountId()); 
                    ChatHandler(player->GetSession()).PSendSysMessage("Вы успешно изменили свой цвет чата на: Обычный");				
                    player->PlayerTalkClass->SendCloseGossip();
                }
                break;
            case 214:
                player->PlayerTalkClass->SendCloseGossip();
                player->GetSession()->SendShowBank(player->GetGUID());
                break;
            case 215:
                player->RemoveAura(15007);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 216: 
                player->DurabilityRepairAll(false, 0.0f, true);
                ChatHandler(player->GetSession()).PSendSysMessage("Я починил всю вашу экипировку!");
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 217:
                player->AddAura(23767, player);
                player->AddAura(23737, player);
                player->AddAura(23736, player);
                player->AddAura(23766, player);
                player->AddAura(23735, player);
                player->AddAura(23769, player);
                player->AddAura(23768, player);
                player->AddAura(23738, player);
                player->AddAura(26393, player);                
                player->CLOSE_GOSSIP_MENU();
                break;
            case 218: // Выдача сумок
                if (player->HasItemCount(23162, 4))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    ChatHandler(player->GetSession()).PSendSysMessage(EMOTE_ALREADY_ITEM);
                }
                else
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    player->AddItem(23162, 4);
                }
                break;
            case 219: // Выдача рубашки
            if (player->HasItemCount(42365, 1))
            {
                player->PlayerTalkClass->SendCloseGossip();
                ChatHandler(player->GetSession()).PSendSysMessage(EMOTE_ALREADY_ITEM);
            }
            else
            {
                player->PlayerTalkClass->SendCloseGossip();
                player->AddItem(42365, 1);
            }            
            break;                
            case 1205:
                player->PlayerTalkClass->ClearMenus();
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/spell_holy_rapture:25:25:-15:0|t Смена имени [|cffff0000500|r Очков Арены]", GOSSIP_SENDER_MAIN, 2000);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/spell_holy_rapture:25:25:-15:0|t Смена внешности [|cffff0000300|r Очков Арены]", GOSSIP_SENDER_MAIN, 2001);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/spell_holy_rapture:25:25:-15:0|t Смена фракции [|cffff00001к|r Очков Арены]", GOSSIP_SENDER_MAIN, 2002);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/spell_holy_rapture:25:25:-15:0|t Смена расы [|cffff0000500|r Очков Арены]", GOSSIP_SENDER_MAIN, 2003);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargenegative:25:25:-15:0|tЗакрыть", GOSSIP_SENDER_MAIN, 200110);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                break;
            case 2000: // Смена имени
                if (player->GetArenaPoints() < CONST_ARENA_RENAME)
                {
                    ChatHandler(player->GetSession()).PSendSysMessage(EMOTE_NO_ARENA_POINTS);
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    player->SetAtLoginFlag(AT_LOGIN_RENAME);
                    player->ModifyArenaPoints(-CONST_ARENA_RENAME);
                    ChatHandler(player->GetSession()).PSendSysMessage(MSG_RENAME_COMPLETE);
                }
                break;
            case 2001: // Смена внешности
                if (player->GetArenaPoints() < CONST_ARENA_CUSTOMIZE)
                {
                    ChatHandler(player->GetSession()).PSendSysMessage(EMOTE_NO_ARENA_POINTS);
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    player->SetAtLoginFlag(AT_LOGIN_CUSTOMIZE);
                    player->ModifyArenaPoints(-CONST_ARENA_CUSTOMIZE);
                    ChatHandler(player->GetSession()).PSendSysMessage(MSG_CUSTOMIZE_COMPLETE);
                }
                break;
            case 2002: // Смена фракции
                if (player->GetArenaPoints() < CONST_ARENA_CHANGE_FACTION)
                {
                    ChatHandler(player->GetSession()).PSendSysMessage(EMOTE_NO_ARENA_POINTS);
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    player->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
                    player->ModifyArenaPoints(-CONST_ARENA_CHANGE_FACTION);
                    ChatHandler(player->GetSession()).PSendSysMessage(MSG_CHANGE_FACTION_COMPLETE);
                }
                break;
            case 2003: // Смена расы
                if (player->GetArenaPoints() < CONST_ARENA_CHANGE_RACE)
                {
                    ChatHandler(player->GetSession()).PSendSysMessage(EMOTE_NO_ARENA_POINTS);
                    player->PlayerTalkClass->SendCloseGossip();
                }
                else
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    player->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
                    player->ModifyArenaPoints(-CONST_ARENA_CHANGE_RACE);
                    ChatHandler(player->GetSession()).PSendSysMessage(MSG_CHANGE_RACE_COMPLETE);
                }
                break;
            case 1212: // Сбросить 'КД'
                if (player->HasAura(45523))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    player->GetSession()->SendAreaTriggerMessage(EMOTE_COOLDOWN);
                }
                else
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    player->RemoveArenaSpellCooldowns(true);
                    player->GetSession()->SendAreaTriggerMessage(MSG_RESET_COOLDOWN);
                    player->CastSpell(player, 45523, true);
                }
                break;
            case 1215: // Сбросить 'КД' заданий
                player->PlayerTalkClass->SendCloseGossip();
                player->ResetDailyQuestStatus();
                player->ResetWeeklyQuestStatus();
                ChatHandler(player->GetSession()).PSendSysMessage(MSG_RESET_QUEST_STATUS_COMPLETE);
                break;
            case 210:
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("|cff006699Список комманд доступно VIP");
                    ChatHandler(player->GetSession()).PSendSysMessage("|cff006699.vip app|r Телепортация к игроку в группе");
                    ChatHandler(player->GetSession()).PSendSysMessage("|cff006699.vip bank|r Ваш банк");
                    ChatHandler(player->GetSession()).PSendSysMessage("|cff006699.vip repair|r Бесплатный ремонт");
                    //ChatHandler(player->GetSession()).PSendSysMessage("|cff006699.vip debuff|r Снимает Слабость");
                    ChatHandler(player->GetSession()).PSendSysMessage("|cff006699.vip resettalents|r Сброс талантов");
                    player->PlayerTalkClass->SendCloseGossip();
                }
                break;

            case 777:
                player->ADD_GOSSIP_ITEM(0, "|cffD80000Информация О серверe|r", GOSSIP_SENDER_MAIN, 200110);
                player->ADD_GOSSIP_ITEM(0, "|cff660099Сайт:|r Wow-idk.ru", GOSSIP_SENDER_MAIN, 200110);
                player->ADD_GOSSIP_ITEM(0, "|cffD80000Рейты:|r", GOSSIP_SENDER_MAIN, 200110);
                player->ADD_GOSSIP_ITEM(0, "|cff660099Рейт репутатции:|r x100", GOSSIP_SENDER_MAIN, 200110);
                player->ADD_GOSSIP_ITEM(0, "|cff660099Рейт дропа голды:|r x10", GOSSIP_SENDER_MAIN, 200110);
                player->ADD_GOSSIP_ITEM(0, "|cff660099Рейт Хонора:|r x2", GOSSIP_SENDER_MAIN, 200110);
                player->ADD_GOSSIP_ITEM(0, "|cffD80000Очки Чести | Очки Арены|r", GOSSIP_SENDER_MAIN, 200110);
                player->ADD_GOSSIP_ITEM(0, "|cff660099Максимальное Кол Очков чести:|r 2м", GOSSIP_SENDER_MAIN, 200110);
                player->ADD_GOSSIP_ITEM(0, "|cff660099Максимальное Кол Очков арены:|r 10к", GOSSIP_SENDER_MAIN, 200110);
                player->ADD_GOSSIP_ITEM(0, "|cff660099Начесление Очков Арены:|r Каждые 2 дня", GOSSIP_SENDER_MAIN, 200110);
                player->ADD_GOSSIP_ITEM(0, "|cffD80000Администрация|r", GOSSIP_SENDER_MAIN, 200110);
                player->ADD_GOSSIP_ITEM(0, "|cff660099Куратор Проекта:|r Godnes", GOSSIP_SENDER_MAIN, 200110);
                player->ADD_GOSSIP_ITEM(0, "|cff660099Зам куратора:|r Akzar", GOSSIP_SENDER_MAIN, 200110);
                //player->ADD_GOSSIP_ITEM(0, "|cff660099|r Закрыть", GOSSIP_SENDER_MAIN, 200110);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                break;

            case 212:
                if (player->HasSpell(50281))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    ChatHandler(player->GetSession()).PSendSysMessage(EMOTE_ALREADY_ITEM);
                }
                else
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->LearnSpell(50281, false); // Vip маунт
                    break;
                    ChatHandler(player->GetSession()).PSendSysMessage("|cff006699Вы успешно получили вип маунта|r");
                }
                break;

            case GOSSIP_ACTION_INFO_DEF + 6:
                player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_BATTLE, "Купить VIP на [ |cffD800007|r ] Дней [175 Бонусов]", GOSSIP_SENDER_MAIN, 500100, "Вы уверены что хотите купить VIP аккаунт на 7 дней ?", 0, false);
                player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_BATTLE, "Купить VIP на [ |cffD8000014|r ] Дней [350 Бонусов]", GOSSIP_SENDER_MAIN, 500101, "Вы уверены что хотите купить VIP аккаунт на 14 дней ?", 0, false);
                player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_BATTLE, "Купить VIP на [ |cffD8000031|r ] День [700 Бонусов]", GOSSIP_SENDER_MAIN, 500102, "Вы уверены что хотите купить VIP аккаунт на 31 день ?", 0, false);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                break;

            case 500100:
                {
                    QueryResult result1 = LoginDatabase.PQuery("SELECT coin FROM account WHERE id = %u", player->GetSession()->GetAccountId());
                    if (result1)
                    {

                        Field *fields = result1->Fetch();
                        uint32 coins = fields[0].GetUInt32();
                        uint32 ostatok = 175 - coins; // 7 дней				
                        int setdate;
                        time_t current_time = time(0);
                        setdate = int(current_time);
                        uint32 unsetdate = setdate + 604800; // 7 дней    

                        if (coins >= 175)
                        {
                            LoginDatabase.PExecute("INSERT INTO account_premium VALUES (%u, 193, 2, %u, %u, 1, 1)", player->GetSession()->GetAccountId(), setdate, unsetdate);
                            LoginDatabase.PQuery("UPDATE account SET coin = coin-175 WHERE id = %u", player->GetSession()->GetAccountId());
                            player->SaveToDB();
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699Вы успешно получи Вип статус, сделайте полный релог для премение действий!|r");
                            player->CLOSE_GOSSIP_MENU();
                        }
                        else
                        {
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699Недостаточно бонусов|r");
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699У вас: |cff00FF33%u|r", coins);
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699Нужно еще: |cffFF0000%u|r", ostatok);
                            player->CLOSE_GOSSIP_MENU();
                        }
                    }
                }
                break;
            case 500101:
                {
                    QueryResult result1 = LoginDatabase.PQuery("SELECT coin FROM account WHERE id = %u", player->GetSession()->GetAccountId());
                    if (result1)
                    {

                        Field *fields = result1->Fetch();
                        uint32 coins = fields[0].GetUInt32();
                        uint32 ostatok = 350 - coins; // 14 дней				
                        int setdate;
                        time_t current_time = time(0);
                        setdate = int(current_time);
                        uint32 unsetdate = setdate + 1209600; // 14 дней    

                        if (coins >= 350)
                        {
                            LoginDatabase.PExecute("INSERT INTO account_premium VALUES (%u, 193, 2, %u, %u, 1, 1)", player->GetSession()->GetAccountId(), setdate, unsetdate);
                            LoginDatabase.PQuery("UPDATE account SET coin = coin-350 WHERE id = %u", player->GetSession()->GetAccountId());
                            player->SaveToDB();
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699Вы успешно получи Вип статус, сделайте полный релог для премение действий!|r");
                            player->PlayerTalkClass->SendCloseGossip();
                        }
                        else
                        {
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699Недостаточно бонусов|r");
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699У вас: |cff00FF33%u|r", coins);
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699Нужно еще: |cffFF0000%u|r", ostatok);
                            player->PlayerTalkClass->SendCloseGossip();
                        }
                    }
                }
                break;
            case 500102:
                {
                    QueryResult result1 = LoginDatabase.PQuery("SELECT coin FROM account WHERE id = %u", player->GetSession()->GetAccountId());
                    if (result1)
                    {

                        Field *fields = result1->Fetch();
                        uint32 coins = fields[0].GetUInt32();
                        uint32 ostatok = 700 - coins; // 31 день					
                        int setdate;
                        time_t current_time = time(0);
                        setdate = int(current_time);
                        uint32 unsetdate = setdate  + 2678400; // 31 день      

                        if (coins >= 700)
                        {
                            LoginDatabase.PExecute("INSERT INTO account_premium VALUES (%u, 193, 2, %u, %u, 1, 1)", player->GetSession()->GetAccountId(), setdate, unsetdate);
                            LoginDatabase.PQuery("UPDATE account SET coin = coin-700 WHERE id = %u", player->GetSession()->GetAccountId());
                            player->SaveToDB();
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699Вы успешно получи Вип статус, сделайте полный релог для премение действий!|r");
                            player->PlayerTalkClass->SendCloseGossip();
                        }
                        else
                        {
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699Недостаточно бонусов|r");
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699У вас: |cff00FF33%u|r", coins);
                            ChatHandler(player->GetSession()).PSendSysMessage("|cff006699Нужно еще: |cffFF0000%u|r", ostatok);
                            player->PlayerTalkClass->SendCloseGossip();
                        }
                    }
                }
                break;

            case 200500:
                {
                    player->PlayerTalkClass->ClearMenus();
                    //player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|TInterface\\icons\\Achievement_zone_sholazar_01:25:25:-15:0|tТелепортер", GOSSIP_SENDER_MAIN, 100521);
                    //player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|TInterface\\icons\\Inv_brd_banner:25:25:-15:0|tТрансмогрификация", GOSSIP_SENDER_MAIN, 100522);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|TInterface\\icons\\Achievement_worldevent_lunar:25:25:-15:0|tРеагенты и бытовые товары", GOSSIP_SENDER_MAIN, 100523);
                    player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargenegative:25:25:-15:0|tЗакрыть", GOSSIP_SENDER_MAIN, 200110);
                    player->PlayerTalkClass->SendGossipMenu(62808, item->GetGUID());
                }
                break;

            case 100524:
                {
                    player->PlayerTalkClass->ClearMenus();
                    player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_BeastTaming:25:25:-15:0|t Новый питомец", GOSSIP_SENDER_MAIN, NORMAL_PET_PAGE_1);
                    if (player->CanTameExoticPets())
                    {
                        player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_BeastMastery:25:25:-15:0|t Новые экзотические питомцы", GOSSIP_SENDER_MAIN, EXOTIC_PET);
                    }
                    player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\INV_Box_PetCarrier_01:25:25:-15:0|t Стойло", GOSSIP_SENDER_MAIN, STABLE_PET);
                    player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_GoForTheThroat:25:25:-15:0|t Сброс талантов питомца", GOSSIP_SENDER_MAIN, RESET_PET_TALENTS);
                    player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\INV_Scroll_11:25:25:-15:0|t Переименовать питомца", GOSSIP_SENDER_MAIN, RENAME_PET);
                    player->SEND_GOSSIP_MENU(1, item->GetGUID());
                }
                break;

            case NORMAL_PET_PAGE_1:
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Bat:25:25:-15:0|t Bat", GOSSIP_SENDER_MAIN, 1045);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Bear:25:25:-15:0|t Bear", GOSSIP_SENDER_MAIN, 1046);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Boar:25:25:-15:0|t Boar", GOSSIP_SENDER_MAIN, 1047);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Cat:25:25:-15:0|t Cat", GOSSIP_SENDER_MAIN, 1048);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Vulture:25:25:-15:0|t Carrion Bird", GOSSIP_SENDER_MAIN, 1049);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Crab:25:25:-15:0|t Crab", GOSSIP_SENDER_MAIN, 1050);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Crocolisk:25:25:-15:0|t Crocolisk", GOSSIP_SENDER_MAIN, 1051);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_DragonHawk:25:25:-15:0|t DragonHawk", GOSSIP_SENDER_MAIN, 1052);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Gorilla:25:25:-15:0|t Gorilla", GOSSIP_SENDER_MAIN, 1053);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Wolf:25:25:-15:0|t BlightHound", GOSSIP_SENDER_MAIN, 1054);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Hyena:25:25:-15:0|t Hyena", GOSSIP_SENDER_MAIN, 1055);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Moth:25:25:-15:0|t Moth", GOSSIP_SENDER_MAIN, 1056);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Owl:25:25:-15:0|t Owl", GOSSIP_SENDER_MAIN, 1057);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Trade_Engineering:25:25:-15:0|t Следующая страница ->", GOSSIP_SENDER_MAIN, NORMAL_PET_PAGE_2);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Trade_Engineering:25:25:-15:0|t Главное меню", GOSSIP_SENDER_MAIN, 100524);
                player->SEND_GOSSIP_MENU(1, item->GetGUID());
                break;

            case NORMAL_PET_PAGE_2:
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Raptor:25:25:-15:0|t Raptor", GOSSIP_SENDER_MAIN, 1058);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Ravager:25:25:-15:0|t Ravager", GOSSIP_SENDER_MAIN, 1059);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_TallStrider:25:25:-15:0|t Tallstrider", GOSSIP_SENDER_MAIN, 1060);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Scorpid:25:25:-15:0|t Scorpid", GOSSIP_SENDER_MAIN, 1061);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Spider:25:25:-15:0|t Spider", GOSSIP_SENDER_MAIN, 1062);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_WindSerpent:25:25:-15:0|t Serpent", GOSSIP_SENDER_MAIN, 1063);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Turtle:25:25:-15:0|t Turtle", GOSSIP_SENDER_MAIN, 1064);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Wasp:25:25:-15:0|t Wasp", GOSSIP_SENDER_MAIN, 1065);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Trade_Engineering:25:25:-15:0|t <- Предыдущая страница", GOSSIP_SENDER_MAIN, NORMAL_PET_PAGE_1);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Trade_Engineering:25:25:-15:0|t Главное меню", GOSSIP_SENDER_MAIN, 100524);
                player->SEND_GOSSIP_MENU(1, item->GetGUID());
                break;

            case EXOTIC_PET:
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Chimera:25:25:-15:0|t Chimera", GOSSIP_SENDER_MAIN, 1066);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_CoreHound:25:25:-15:0|t Core Hound", GOSSIP_SENDER_MAIN, 1067);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Devilsaur:25:25:-15:0|t Devilsaur", GOSSIP_SENDER_MAIN, 1068);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Rhino:25:25:-15:0|t Rhino", GOSSIP_SENDER_MAIN, 1069);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Silithid:25:25:-15:0|t Silithid", GOSSIP_SENDER_MAIN, 1070);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Worm:25:25:-15:0|t Worm", GOSSIP_SENDER_MAIN, 1071);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\INV_Misc_Head_Tiger_01:25:25:-15:0|t Loque'nahak", GOSSIP_SENDER_MAIN, 1072);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Ability_Hunter_Pet_Wolf:25:25:-15:0|t Skoll", GOSSIP_SENDER_MAIN, 1073);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\ability_mount_spectraltiger:25:25:-15:0|t Spectral Tiger", GOSSIP_SENDER_MAIN, 1074);
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\Icons\\Trade_Engineering:25:25:-15:0|t Назад в главное меню", GOSSIP_SENDER_MAIN, 100524);
                player->SEND_GOSSIP_MENU(1, item->GetGUID());
                break;
            case 1045:
                CreatePet(player, item, BAT);
                break;

            case 1046:
                CreatePet(player, item, BEAR);
                break;

            case 1047:
                CreatePet(player, item, BOAR);
                break;

            case 1048:
                CreatePet(player, item, CAT);
                break;

            case 1049:
                CreatePet(player, item, CARRION_BIRD);
                break;

            case 1050:
                CreatePet(player, item, CRAB);
                break;

            case 1051:
                CreatePet(player, item, CROCOLISK);
                break;

            case 1052:
                CreatePet(player, item, DRAGONHAWK);
                break;

            case 1053:
                CreatePet(player, item, GORILLA);
                break;

            case 1054:
                CreatePet(player, item, BLIGHT_HOUND);
                break;

            case 1055:
                CreatePet(player, item, HYENA);
                break;

            case 1056:
                CreatePet(player, item, MOTH);
                break;

            case 1057:
                CreatePet(player, item, OWL);
                break;

            case 1058:
                CreatePet(player, item, RAPTOR);
                break;

            case 1059:
                CreatePet(player, item, RAVAGER);
                break;

            case 1060:
                CreatePet(player, item, TALLSTRIDER);
                break;

            case 1061:
                CreatePet(player, item, SCORPID);
                break;

            case 1062:
                CreatePet(player, item, SPIDER);
                break;

            case 1063:
                CreatePet(player, item, SERPENT);
                break;

            case 1064:
                CreatePet(player, item, TURTLE);
                break;

            case 1065:
                CreatePet(player, item, WASP);
                break;

            case 1066:
                CreatePet(player, item, CHIMERA);
                break;

            case 1067:
                CreatePet(player, item, CORE_HOUND);
                break;

            case 1068:
                CreatePet(player, item, DEVILSAUR);
                break;

            case 1069:
                CreatePet(player, item, RHINO);
                break;

            case 1070:
                CreatePet(player, item, SILITHID);
                break;

            case 1071:
                CreatePet(player, item, WORM);
                break;

            case 1072:
                CreatePet(player, item, LOQUE_NAHAK);
                break;

            case 1073:
                CreatePet(player, item, SKOLL);
                break;

            case 1074:
                CreatePet(player, item, GONDRIA);
                break;


            case RESET_PET_TALENTS:
                if (player->GetPet())
                {
                    player->ResetPetTalents();
                    player->GetSession()->SendAreaTriggerMessage("Таланты вашего питомца успешно сброшены!");
                    player->CLOSE_GOSSIP_MENU();

                }
                else
                {
                    player->GetSession()->SendNotification("У вас нет питомца!");
                    player->CLOSE_GOSSIP_MENU();
                }
                break;


            case STABLE_PET:
                player->CastSpell(player, 62757, true); // Call Stabled Pet
                player->CLOSE_GOSSIP_MENU();
                break;


            case RENAME_PET:
                player->CastSpell(player, 59385, true); //Certificate of Ownership Spell - no cast time
                player->CLOSE_GOSSIP_MENU();
                break;

            case 100521:
                {
                    if (player->IsInCombat())
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вы в бою!");
                        OnUse(player, item, SpellCastTargets());
                        return;
                    }

                    if (player->FindNearestCreature(190013, 25.0f, true))
                    {
                        player->GetSession()->SendAreaTriggerMessage("Около вас уже стоит данный моб!");
                        OnUse(player, item, SpellCastTargets());
                        return;
                    }
                    else
                    player->SummonCreature(190013, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 60000);
                    player->CLOSE_GOSSIP_MENU();
                    return;
                }
                break;
            case 100522:
                {
                    if (player->IsInCombat())
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вы в бою!");
                        OnUse(player, item, SpellCastTargets());
                        return;
                    }

                    if (player->FindNearestCreature(190010, 25.0f, true))
                    {
                        player->GetSession()->SendAreaTriggerMessage("Около вас уже стоит данный моб!");
                        OnUse(player, item, SpellCastTargets());
                        return;
                    }
                    else
                    player->SummonCreature(190010, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 60000);
                    player->CLOSE_GOSSIP_MENU();
                    return;
                }
                break;
            case 100523:
                {
                    if (player->IsInCombat())
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вы в бою!");
                        OnUse(player, item, SpellCastTargets());
                        return;
                    }

                    if (player->FindNearestCreature(190014, 25.0f, true))
                    {
                        player->GetSession()->SendAreaTriggerMessage("Около вас уже стоит данный моб!");
                        OnUse(player, item, SpellCastTargets());
                        return;
                    }
                    else
                    player->SummonCreature(190014, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 60000);
                    player->CLOSE_GOSSIP_MENU();
                    return;
                }
                break;
            case 1249:
                player->PlayerTalkClass->ClearMenus();
               // player->ADD_GOSSIP_ITEM(0, "[Лёд] > [Триумф]", GOSSIP_SENDER_MAIN, 111212);
                //player->ADD_GOSSIP_ITEM(0, "[Триумф] > [Лёд]", GOSSIP_SENDER_MAIN, 3983882);
                player->ADD_GOSSIP_ITEM(0, "[Лёд] > [Хонор]", GOSSIP_SENDER_MAIN, 1272616);
                player->ADD_GOSSIP_ITEM(0, "[Хонор] > [Лёд]", GOSSIP_SENDER_MAIN, 1272656);
                player->ADD_GOSSIP_ITEM(0, "[Хонор] > [Ап]", GOSSIP_SENDER_MAIN, 1272657);
                player->ADD_GOSSIP_ITEM(0, "[Ап] > [Хонор]", GOSSIP_SENDER_MAIN, 1272658);
                player->ADD_GOSSIP_ITEM(0, "[Хонор] > [Эфириальная монета]", GOSSIP_SENDER_MAIN, 1272659);
                player->ADD_GOSSIP_ITEM(0, "[Закрыть]", GOSSIP_SENDER_MAIN, 200110);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                break;
            case 111212: 
                player->PlayerTalkClass->ClearMenus();
                player->ADD_GOSSIP_ITEM(0, "[|cffff00001|r Лёд] > [|cffff00005|r Триумф]", GOSSIP_SENDER_MAIN, 154);
                player->ADD_GOSSIP_ITEM(0, "[|cffff00005|r Лёд] > [|cffff000025|r Триумф]", GOSSIP_SENDER_MAIN, 155);
                player->ADD_GOSSIP_ITEM(0, "[|cffff000010|r Лёд] > [|cffff000050|r Триумф]", GOSSIP_SENDER_MAIN, 156);
                player->ADD_GOSSIP_ITEM(0, "[|cffff000020|r Лёд] > [|cffff0000100|r Триумф]", GOSSIP_SENDER_MAIN, 157);
                player->ADD_GOSSIP_ITEM(0, "[|cffff000050|r Лёд] > [|cffff0000250|r Триумф]", GOSSIP_SENDER_MAIN, 158);
                player->ADD_GOSSIP_ITEM(0, "[Закрыть]", GOSSIP_SENDER_MAIN, 200110);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                break;
            case 154: // 1 Эмблем Льда на 5 Эмблем Триумфа
                if (player->HasItemCount(TOKEN_ID, token1))
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->DestroyItemCount(TOKEN_ID, token1, true);
                    player->AddItem(47241, 5);
                    player->GetSession()->SendNotification("Ваши |cff330066 [5 Эмблем Триумфа]|r были добавлены.");
                }
                else
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("У вас недостаточно Эмблем Льда.");
                }
                break;

            case 155: // 5 Эмблем Льда на 25 Эмблем Триумфа
                if (player->HasItemCount(TOKEN_ID, token2))
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->DestroyItemCount(TOKEN_ID, token2, true);
                    player->AddItem(47241, 25);
                    player->GetSession()->SendNotification("Ваши |cff330066 [25 Эмблем Триумфа]|r были добавлены.");
                }
                else
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("У вас недостаточно Эмблем Льда.");
                }
                break;

            case 156: // 10 Эмблем Льда на 50 Эмблем Триумфа
                if (player->HasItemCount(TOKEN_ID, token3))
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->DestroyItemCount(TOKEN_ID, token3, true);
                    player->AddItem(47241, 50);
                    player->GetSession()->SendNotification("Ваши |cff330066 [50 Эмблем Триумфа]|r были добавлены.");
                }
                else
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("У вас недостаточно Эмблем Льда.");
                }
                break;

            case 157: // 20 Эмблем Льда на 100 Эмблем Триумфа
                if (player->HasItemCount(TOKEN_ID, token4))
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->DestroyItemCount(TOKEN_ID, token4, true);
                    player->AddItem(47241, 100);
                    player->GetSession()->SendNotification("Ваши |cff330066 [100 Эмблем Триумфа]|r были добавлены.");
                }
                else
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("У вас недостаточно Эмблем Льда.");
                }
                break;

            case 158: // 50 Эмблем Льда на 250 Эмблем Триумфа
                if (player->HasItemCount(TOKEN_ID, token5))
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->DestroyItemCount(TOKEN_ID, token5, true);
                    player->AddItem(47241, 250);
                    player->GetSession()->SendNotification("Ваши |cff330066 [250 Эмблем Триумфа]|r были добавлены.");
                }
                else
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("У вас недостаточно Эмблем Льда.");
                }
                break;
            case 3983882: 
                {
                    player->ADD_GOSSIP_ITEM(0, "[|cffff00005|r Триумф] > [|cffff00001|r Лёд]", GOSSIP_SENDER_MAIN, 422);
                    player->ADD_GOSSIP_ITEM(0, "[|cffff000050|r Триумф] > [|cffff000010|r Лёд]", GOSSIP_SENDER_MAIN, 423);
                    player->ADD_GOSSIP_ITEM(0, "[|cffff0000125|r Триумф] > [|cffff000025|r Лёд]", GOSSIP_SENDER_MAIN, 424);
                    player->ADD_GOSSIP_ITEM(0, "Назад", GOSSIP_SENDER_MAIN, 56);
                    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                }
                break;

            case 422: // 5 Эмблем Триумфа на 1 Эмблем Льда
                if (player->HasItemCount(TOKEN_IDD, token2))
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->DestroyItemCount(TOKEN_IDD, token2, true);
                    player->AddItem(49426, 1);
                    player->GetSession()->SendNotification("Ваши |cff330066 [1 Эмблем Льда]|r были добавлены.");
                }
                else
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("У вас недостаточно Эмблем Триумфа.");
                }
                break;

            case 423: // 50 Эмблем Триумфа на 10 Эмблем Льда
                if (player->HasItemCount(TOKEN_IDD, token5))
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->DestroyItemCount(TOKEN_IDD, token5, true);
                    player->AddItem(49426, 10);
                    player->GetSession()->SendNotification("Ваши |cff330066 [10 Эмблем Льда]|r были добавлены.");
                }
                else
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("У вас недостаточно Эмблем Триумфа.");
                }
                break;
            case 424: // 50 Эмблем Триумфа на 10 Эмблем Льда
                if (player->HasItemCount(TOKEN_IDD, token10))
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->DestroyItemCount(TOKEN_IDD, token10, true);
                    player->AddItem(49426, 25);
                    player->GetSession()->SendNotification("Ваши |cff330066 [25 Эмблем Льда]|r были добавлены.");
                }
                else
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("У вас недостаточно Эмблем Триумфа.");
                }
                break;

            case 1272616: // Обмен лёд на Хонор 
                {
                    player->ADD_GOSSIP_ITEM(9, "[|cffff00001|r Лёд] > [|cffff0000200|r Хонор]", GOSSIP_SENDER_MAIN, 700);
                    player->ADD_GOSSIP_ITEM(9, "[|cffff000010|r Лёд] > [|cffff00002к|r Хонор]", GOSSIP_SENDER_MAIN, 701);
                    player->ADD_GOSSIP_ITEM(9, "Назад", GOSSIP_SENDER_MAIN, 56);
                    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                }
                break;

            case 700: // 1 лёд на 2к хонора
                if (player->HasItemCount(49426, 1))
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->DestroyItemCount(49426, 1, true);
                    player->ModifyHonorPoints(+200);
                    player->GetSession()->SendNotification("Ваши [200 Очков Чести] были добавлены.");
                }
                else
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("У вас недостаточно Эмблем Льда Валюты.");
                }
                break;

            case 701: // 10 лёд на 20к хонора
                if (player->HasItemCount(49426, 10))
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->DestroyItemCount(49426, 10, true);
                    player->ModifyHonorPoints(+2000);
                    player->GetSession()->SendNotification("Ваши [2000 Очков Чести] были добавлены.");
                }
                else
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("У вас недостаточно Эмблем Льда Валюты.");
                }
                break;

            case 1272656: // Хонор на Лёд
                {
                    player->ADD_GOSSIP_ITEM(0, "[|cffff00002к|r Хонор] > [|cffff00001|r Лёд]", GOSSIP_SENDER_MAIN, 902);
                    player->ADD_GOSSIP_ITEM(0, "[|cffff000020к|r Хонор] > [|cffff000010|r Лёд]", GOSSIP_SENDER_MAIN, 903);
                    player->ADD_GOSSIP_ITEM(0, "Назад", GOSSIP_SENDER_MAIN, 56);
                    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                }
                break;

            case 902: // 2к хонора на 1 лёд
                if (player->GetHonorPoints() < CONST_HONOR_POINT)
                {
                    player->GetSession()->SendNotification("У вас недостаточно Очков чести");
                    player->CLOSE_GOSSIP_MENU();
                }
                else
                {
                    player->ModifyHonorPoints(-CONST_HONOR_POINT);
                    player->AddItem(49426, 1);
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("Ваши |cff330066[1 Эмблем Льда]|r были добавлены.");
                }
                break;

            case 903: // 20к хонора на 10 лёд
                if (player->GetHonorPoints() < CONST_HONOR_POINT2)
                {
                    player->GetSession()->SendNotification("У вас недостаточно Очков чести");
                    player->CLOSE_GOSSIP_MENU();

                }
                else
                {
                    player->ModifyHonorPoints(-CONST_HONOR_POINT2);
                    player->AddItem(49426, 10);
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("Ваши |cff330066[10 Эмблем Льда]|r были добавлены.");
                }
                break;

            case 1272657: // Хонор на АП
                {
                    player->ADD_GOSSIP_ITEM(0, "[|cffff000030к|r Хонор] > [|cffff0000100|r Ап]", GOSSIP_SENDER_MAIN, 26);
                    player->ADD_GOSSIP_ITEM(0, "[|cffff0000300к|r Хонор] > [|cffff00001к|r Ап]", GOSSIP_SENDER_MAIN, 25);
                    player->ADD_GOSSIP_ITEM(0, "Назад", GOSSIP_SENDER_MAIN, 56);
                    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                }
                break;

            case 26: // 10к хонора на 100 апа
                if (player->GetHonorPoints() < CONST_HONOR_POINT3)
                {
                    player->GetSession()->SendNotification("У вас недостаточно Очков чести");
                    player->CLOSE_GOSSIP_MENU();
                }
                else
                {
                    player->ModifyHonorPoints(-CONST_HONOR_POINT3);
                    player->ModifyArenaPoints(+100);
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("Ваши [100 Арена Пойтов] были добавлены.");
                }
                break;

            case 25: // 100к хонора на 1000 апа
                if (player->GetHonorPoints() < CONST_HONOR_POINT4)
                {
                    player->GetSession()->SendNotification("У вас недостаточно Очков чести");
                    player->CLOSE_GOSSIP_MENU();
                }
                else
                {

                    player->ModifyHonorPoints(-CONST_HONOR_POINT4);
                    player->ModifyArenaPoints(+1000);
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("Ваши [1000 Арена Пойтов] были добавлены.");
                }
                break;

            case 1272658: // Ап на хонор
                {
                    player->ADD_GOSSIP_ITEM(0, "[|cffff0000100|r Ап] > [|cffff000010к|r Хонор]", GOSSIP_SENDER_MAIN, 202);
                    player->ADD_GOSSIP_ITEM(0, "[|cffff00001к|r Ап] > [|cffff0000100к|r Хонор]", GOSSIP_SENDER_MAIN, 203);
                    player->ADD_GOSSIP_ITEM(0, "Назад", GOSSIP_SENDER_MAIN, 56);
                    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                }
                break;
            case 202: // 100 апа на 10к хонора
                if (player->GetArenaPoints() < CONST_ARENA_POINT)
                {
                    player->GetSession()->SendNotification("У вас недостаточно Арена Пойнтов");
                    player->CLOSE_GOSSIP_MENU();

                }
                else
                {
                    player->ModifyArenaPoints(-CONST_ARENA_POINT);
                    player->ModifyHonorPoints(+10000);
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("Ваши [10к Очков Чести] успешно выданы.");
                }
                break;
            case 203: // 1000 апа на 100к хонора
                if (player->GetArenaPoints() < CONST_ARENA_POINT2)
                {
                    player->GetSession()->SendNotification("У вас недостаточно Арена Пойнтов");
                    player->CLOSE_GOSSIP_MENU();
                }
                else
                {
                    player->ModifyArenaPoints(-CONST_ARENA_POINT2);
                    player->ModifyHonorPoints(+100000);
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("Ваши [100к Очков Чести] успешно выданы.");
                }
                break;
            case 1272659: // Хонор на Эфир
                {
                    player->ADD_GOSSIP_ITEM(0, "[|cffff000010к|r Хонор] > [|cffff000010|r Эфириальная монета]", GOSSIP_SENDER_MAIN, 222);
                    player->ADD_GOSSIP_ITEM(0, "[|cffff0000100к|r Хонор] > [|cffff0000100|r Эфириальных монет]", GOSSIP_SENDER_MAIN, 223);
                    player->ADD_GOSSIP_ITEM(0, "[|cffff0000200к|r Хонор] > [|cffff0000200|r Эфириальных монет]", GOSSIP_SENDER_MAIN, 224);
                    player->ADD_GOSSIP_ITEM(0, "Назад", GOSSIP_SENDER_MAIN, 56);
                    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
                }
                break;
            case 222: // 20к хонора -> 10 эфириальная монета
                if (player->GetHonorPoints() < CONST_HONOR_POINT2)
                {
                    player->GetSession()->SendNotification("У вас недостаточно Очков чести");
                    player->CLOSE_GOSSIP_MENU();
                }
                else
                {
                    player->ModifyHonorPoints(-CONST_HONOR_POINT2);
                    player->AddItem(38186, 10);
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("Ваши |cff330066[10 Эфириальная монета]|r были добавлены.");
                }
                break;
            case 223: // 100к хонора-> 100 эфириальные монеты
                if (player->GetHonorPoints() < CONST_HONOR_POINT4)
                {
                    player->GetSession()->SendNotification("У вас недостаточно Очков чести");
                    player->CLOSE_GOSSIP_MENU();
                }
                else
                {
                    player->ModifyHonorPoints(-CONST_HONOR_POINT4);
                    player->AddItem(38186, 100);
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("Ваши |cff330066[100 Эфириальныx монет]|r были добавлены.");
                }
                break;
            case 224: // 200к хонора-> 200 эфириальные монеты
                if (player->GetHonorPoints() < CONST_HONOR_POINT5)
                {
                    player->GetSession()->SendNotification("У вас недостаточно Очков чести");
                    player->CLOSE_GOSSIP_MENU();
                }
                else
                {
                    player->ModifyHonorPoints(-CONST_HONOR_POINT5);
                    player->AddItem(38186, 200);
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification("Ваши |cff330066[200 Эфириальных монет]|r были добавлены.");
                }
                break;
}
}
}
    void CreatePet(Player* player, Item* item, uint32 entry)
    {
        if (player->GetPet())
        {
            player->GetSession()->SendAreaTriggerMessage("Вы должны убрать в стойло или отпустить текущего питомца ,чтобы получить нового.");
            player->PlayerTalkClass->SendCloseGossip();
            return;
        }

        Creature *creatureTarget = player->SummonCreature(entry, player->GetPositionX(), player->GetPositionY() + 2, player->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 500);
        if (!creatureTarget) return;

        Pet* pet = player->CreateTamedPetFrom(creatureTarget, 0);

        if (!pet)
        return;

        creatureTarget->setDeathState(JUST_DIED);
        creatureTarget->RemoveCorpse();
        creatureTarget->SetHealth(0);

        pet->SetPower(POWER_HAPPINESS, 1048000);
        pet->SetUInt64Value(UNIT_FIELD_CREATEDBY, player->GetGUID());
        pet->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, player->getFaction());
        pet->SetUInt32Value(UNIT_FIELD_LEVEL, player->getLevel() - 1);
        pet->GetMap()->AddToMap(pet->ToCreature());
        pet->SetUInt32Value(UNIT_FIELD_LEVEL, player->getLevel());

        pet->GetCharmInfo()->SetPetNumber(sObjectMgr->GeneratePetNumber(), true);
        if (!pet->InitStatsForLevel(player->getLevel()))
        pet->UpdateAllStats();


        player->SetMinion(pet, true);

        pet->SavePetToDB(PET_SAVE_AS_CURRENT);
        pet->InitTalentForLevel();
        player->PetSpellInitialize();

        player->PlayerTalkClass->SendCloseGossip();
    }
};

class item_vip_add : public ItemScript
{
public:
    item_vip_add() : ItemScript("item_vip_add") {}

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (player ->IsInCombat() || player ->IsInFlight() || player ->GetMap()->IsBattlegroundOrArena() || player ->HasStealthAura() || player ->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || player ->isDead())
        {
            player->GetSession()->SendAreaTriggerMessage("Вы не можете использовать этот итем в бою или на арене!");
            player->CLOSE_GOSSIP_MENU();
            return true;
        }
        player->PlayerTalkClass->ClearMenus();

        player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_BATTLE, "Поощрение за победу над Йогг-сароном(VIP аккаунт)", GOSSIP_SENDER_MAIN, 10, "Вы уверены что хотите использовать итем и получить VIP на 7 дней?", 0, false);
        player->SEND_GOSSIP_MENU(1, item->GetGUID());
        return true;
    }

    void OnGossipSelect(Player* player, Item* item, uint32 /*sender*/, uint32 action) override
    {
        switch(action)
        {
        case 10:              
            {	
                int setdate;
                time_t current_time = time(0);
                setdate = int(current_time);
                uint32 unsetdate = setdate + 604800; // 7 дней    

                LoginDatabase.PExecute("INSERT INTO account_premium VALUES (%u, 193, 2, %u, %u, 1, 1)", player->GetSession()->GetAccountId(), setdate, unsetdate);
                player->SaveToDB();
                ChatHandler(player->GetSession()).PSendSysMessage("|cff006699Вы успешно получи Вип статус, сделайте полный релог для премение действий!|r");
                player->CLOSE_GOSSIP_MENU();
                player->DestroyItemCount(49768, 1, true);
                
            }
            break;
        }
    }	
};


void AddSC_custom_item()
{
    new custom_item();
    new item_vip_add();
}