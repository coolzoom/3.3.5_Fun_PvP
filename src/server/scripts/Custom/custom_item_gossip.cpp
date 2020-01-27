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

#include "ScriptMgr.h"
#include "Chat.h"
#include "Define.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "Pet.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "AchievementMgr.h"

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
#define NORMAL_PET_PAGE_1           GOSSIP_ACTION_INFO_DEF + 1
#define NORMAL_PET_PAGE_2           GOSSIP_ACTION_INFO_DEF + 2
#define EXOTIC_PET                  GOSSIP_ACTION_INFO_DEF + 3
#define STABLE_PET                  GOSSIP_ACTION_INFO_DEF + 4
#define RESET_PET_TALENTS           GOSSIP_ACTION_INFO_DEF + 5
#define RENAME_PET                  GOSSIP_ACTION_INFO_DEF + 6
#define MAIN_MENU                   GOSSIP_ACTION_INFO_DEF + 7
#define BUFFME                      GOSSIP_ACTION_INFO_DEF + 1


class item_beastmaster : public ItemScript
{
public:
    item_beastmaster() : ItemScript("item_beastmaster") {}

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


    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        if (player->IsInCombat())
        {
            player->GetSession()->SendAreaTriggerMessage("Вы не можете использовать этот итем в бою!");
            player->CLOSE_GOSSIP_MENU();
            return true;
        }
        player->PlayerTalkClass->ClearMenus();

        player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_BeastTaming:30|t Новый питомец", GOSSIP_SENDER_MAIN, NORMAL_PET_PAGE_1);
        if (player->CanTameExoticPets())
        {
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_BeastMastery:30|t Новые экзотические питомцы", GOSSIP_SENDER_MAIN, EXOTIC_PET);
        }
        player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\INV_Box_PetCarrier_01:30|t Стойло", GOSSIP_SENDER_MAIN, STABLE_PET);
        player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_GoForTheThroat:30|t Сброс талантов питомца", GOSSIP_SENDER_MAIN, RESET_PET_TALENTS);
        player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\INV_Scroll_11:30|t Переименовать питомца", GOSSIP_SENDER_MAIN, RENAME_PET);
        player->SEND_GOSSIP_MENU(1, item->GetGUID());
        return true;
    }

    void OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action) override
    {
        
        player->PlayerTalkClass->ClearMenus();

        switch(action)
        {
        case MAIN_MENU:
            player->PlayerTalkClass->ClearMenus();

            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_BeastTaming:30|t Новый питомец", GOSSIP_SENDER_MAIN, NORMAL_PET_PAGE_1);
            if (player->CanTameExoticPets())
            {
                player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_BeastMastery:30|t Новые экзотические питомцы", GOSSIP_SENDER_MAIN, EXOTIC_PET);
            }
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\INV_Box_PetCarrier_01:30|t Стойло", GOSSIP_SENDER_MAIN, STABLE_PET);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_GoForTheThroat:30|t Сброс талантов питомца", GOSSIP_SENDER_MAIN, RESET_PET_TALENTS);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\INV_Scroll_11:30|t Переименовать питомца", GOSSIP_SENDER_MAIN, RENAME_PET);
            player->SEND_GOSSIP_MENU(1, item->GetGUID());
            break;

        case NORMAL_PET_PAGE_1:
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Bat:30|t Bat", GOSSIP_SENDER_MAIN,                   1);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Bear:30|t Bear", GOSSIP_SENDER_MAIN,                 2);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Boar:30|t Boar", GOSSIP_SENDER_MAIN,                 3);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Cat:30|t Cat", GOSSIP_SENDER_MAIN,                   4);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Vulture:30|t Carrion Bird", GOSSIP_SENDER_MAIN,      5);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Crab:30|t Crab", GOSSIP_SENDER_MAIN,                 6);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Crocolisk:30|t Crocolisk", GOSSIP_SENDER_MAIN,       7);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_DragonHawk:30|t DragonHawk", GOSSIP_SENDER_MAIN,     8);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Gorilla:30|t Gorilla", GOSSIP_SENDER_MAIN,           9);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Wolf:30|t BlightHound", GOSSIP_SENDER_MAIN,          10);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Hyena:30|t Hyena", GOSSIP_SENDER_MAIN,               11);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Moth:30|t Moth", GOSSIP_SENDER_MAIN,                 12);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Owl:30|t Owl", GOSSIP_SENDER_MAIN,                   13);
            player->ADD_GOSSIP_ITEM(2, "|TInterface\\Icons\\Trade_Engineering:30|t Следующая страница ->", GOSSIP_SENDER_MAIN, NORMAL_PET_PAGE_2);
            player->ADD_GOSSIP_ITEM(2, "|TInterface\\Icons\\Trade_Engineering:30|t Главное меню", GOSSIP_SENDER_MAIN, MAIN_MENU);
            player->SEND_GOSSIP_MENU(1, item->GetGUID());
            break;

        case NORMAL_PET_PAGE_2:
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Raptor:30|t Raptor", GOSSIP_SENDER_MAIN,             14);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Ravager:30|t Ravager", GOSSIP_SENDER_MAIN,           15);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_TallStrider:30|t Tallstrider", GOSSIP_SENDER_MAIN,   16);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Scorpid:30|t Scorpid", GOSSIP_SENDER_MAIN,           17);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Spider:30|t Spider", GOSSIP_SENDER_MAIN,             18);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_WindSerpent:30|t Serpent", GOSSIP_SENDER_MAIN,       19);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Turtle:30|t Turtle", GOSSIP_SENDER_MAIN,             20);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Wasp:30|t Wasp", GOSSIP_SENDER_MAIN,                 21);
            player->ADD_GOSSIP_ITEM(2, "|TInterface\\Icons\\Trade_Engineering:30|t <- Предыдущая страница", GOSSIP_SENDER_MAIN, NORMAL_PET_PAGE_1);
            player->ADD_GOSSIP_ITEM(2, "|TInterface\\Icons\\Trade_Engineering:30|t Главное меню", GOSSIP_SENDER_MAIN, MAIN_MENU);
            player->SEND_GOSSIP_MENU(1, item->GetGUID());
            break;

        case EXOTIC_PET:
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Chimera:30|t Chimera", GOSSIP_SENDER_MAIN,           22);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_CoreHound:30|t Core Hound", GOSSIP_SENDER_MAIN,      23);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Devilsaur:30|t Devilsaur", GOSSIP_SENDER_MAIN,       24);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Rhino:30|t Rhino", GOSSIP_SENDER_MAIN,               25);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Silithid:30|t Silithid", GOSSIP_SENDER_MAIN,         26);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Worm:30|t Worm", GOSSIP_SENDER_MAIN,                 27);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\INV_Misc_Head_Tiger_01:30|t Loque'nahak", GOSSIP_SENDER_MAIN,           28);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\Ability_Hunter_Pet_Wolf:30|t Skoll", GOSSIP_SENDER_MAIN,                29);
            player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\ability_mount_spectraltiger:30|t Spectral Tiger", GOSSIP_SENDER_MAIN,   30);
            player->ADD_GOSSIP_ITEM(2, "|TInterface\\Icons\\Trade_Engineering:30|t Назад в главное меню", GOSSIP_SENDER_MAIN, MAIN_MENU);
            player->SEND_GOSSIP_MENU(1, item->GetGUID());
            break;
            
        case 1:
            CreatePet(player, item, BAT);
            break;

        case 2:
            CreatePet(player, item, BEAR);
            break;

        case 3:
            CreatePet(player, item, BOAR);
            break;

        case 4:
            CreatePet(player, item, CAT);
            break;

        case 5:
            CreatePet(player, item, CARRION_BIRD);
            break;

        case 6:
            CreatePet(player, item, CRAB);
            break;

        case 7:
            CreatePet(player, item, CROCOLISK);
            break;

        case 8:
            CreatePet(player, item, DRAGONHAWK);
            break;

        case 9:
            CreatePet(player, item, GORILLA);
            break;

        case 10:
            CreatePet(player, item, BLIGHT_HOUND);
            break;

        case 11:
            CreatePet(player, item, HYENA);
            break;

        case 12:
            CreatePet(player, item, MOTH);
            break;

        case 13:
            CreatePet(player, item, OWL);
            break;

        case 14:
            CreatePet(player, item, RAPTOR);
            break;

        case 15:
            CreatePet(player, item, RAVAGER);
            break;

        case 16:
            CreatePet(player, item, TALLSTRIDER);
            break;

        case 17:
            CreatePet(player, item, SCORPID);
            break;

        case 18:
            CreatePet(player, item, SPIDER);
            break;

        case 19:
            CreatePet(player, item, SERPENT);
            break;

        case 20:
            CreatePet(player, item, TURTLE);
            break;

        case 21:
            CreatePet(player, item, WASP);
            break;

        case 22:
            CreatePet(player, item, CHIMERA);
            break;
            
        case 23:
            CreatePet(player, item, CORE_HOUND);
            break;
            
        case 24:
            CreatePet(player, item, DEVILSAUR);
            break;

        case 25:
            CreatePet(player, item, RHINO);
            break;

        case 26:
            CreatePet(player, item, SILITHID);
            break;

        case 27:
            CreatePet(player, item, WORM);
            break;

        case 28:
            CreatePet(player, item, LOQUE_NAHAK);
            break;

        case 29:
            CreatePet(player, item, SKOLL);
            break;

        case 30:
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

        }
    }
};

enum spells
{
    POWER_WORD_FORTITUDE = 48162,
    PRAYER_OF_SPRITE = 48074,
    SHADOW_BUFF = 48170,
    KINGS_BUFF = 43223,
    ARCANE_BUFF = 36880,
    MARK_OF_THE_WILD = 48469,
    BLESSING_OF_WINDOM = 48938,
    BLESSING_OF_SANCTUARY = 25899,
    BLESSING_OF_MIGHT = 48934,
	DK_BAFF = 57623,
	DK_BAF = 15366,
	DK_BAFFF = 47440
};

class buff_item : public ItemScript
{
public:
    buff_item() : ItemScript("buff_item") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        if (player ->IsInCombat() || player ->IsInFlight() || player ->GetMap()->IsBattlegroundOrArena() || player ->HasStealthAura() || player ->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || player ->isDead())
        {
            player->GetSession()->SendAreaTriggerMessage("Вы не можете использовать этот итем в бою или на арене!");
            player->CLOSE_GOSSIP_MENU();
            return true;
        }
        player->PlayerTalkClass->ClearMenus();

        player->ADD_GOSSIP_ITEM(4, "|TInterface\\Icons\\INV_Scroll_11:30|t Бафни меня", GOSSIP_SENDER_MAIN, BUFFME);
        player->SEND_GOSSIP_MENU(1, item->GetGUID());
        return true;
    }

    void OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action) override
    {
        
        player->PlayerTalkClass->ClearMenus();

        switch(action)
        {
        case BUFFME:              
            player->CastSpell(player, POWER_WORD_FORTITUDE, true);
            player->CastSpell(player, KINGS_BUFF, true);
            player->CastSpell(player, MARK_OF_THE_WILD, true);
            player->CastSpell(player, PRAYER_OF_SPRITE, true);
            player->CastSpell(player, ARCANE_BUFF, true);
            player->CastSpell(player, SHADOW_BUFF, true);
			player->CastSpell(player, DK_BAFF, true);
			player->CastSpell(player, DK_BAF, true);
			player->CastSpell(player, DK_BAFFF, true);
            player->CLOSE_GOSSIP_MENU();
            break;
        }
    }	
};

#define NPC_ENTRY_TO_SUMMON 190013

class item_custom_summon : public ItemScript
{
public:
    item_custom_summon() : ItemScript("item_custom_summon") {}

    bool OnUse(Player* player, Item* pItem, SpellCastTargets const& /*targets*/)
    {
        if (player ->IsInCombat() || player ->IsInFlight() || player ->GetMap()->IsBattlegroundOrArena() || player ->HasStealthAura() || player ->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || player ->isDead() || player ->GetAreaId() == 616)
        {
            player->SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW, pItem, NULL);
            return true;
        }
        
        else if (player->FindNearestCreature(80410, 40) || player->FindNearestCreature(190013, 8))
        {
            player->GetSession()->SendNotification("Вы не можете призвать телепортера в данный момент.Попробуйте позже.");
        }
        
        else
        player->SummonCreature(NPC_ENTRY_TO_SUMMON,player->GetPositionX()+4 ,player->GetPositionY(), player->GetPositionZ()+4, 0,TEMPSUMMON_TIMED_DESPAWN,40000);
        return false;
    }
};
#define CONST_ARENA_RENAME 10
#define CONST_ARENA_CUSTOMIZE 10
#define CONST_ARENA_CHANGE_FACTION 50
#define CONST_ARENA_CHANGE_RACE 25

#define EMOTE_NO_VIP "|cffff0000Этим предметом могут пользоваться только V.I.P. игроки!|r"
#define EMOTE_COOLDOWN "|cffff0000Нельзя пользоваться этой функцией так часто!|r"
#define EMOTE_NO_SICKENSS "|cff00ffffУ вас нет 'Слабости после воскрешения'!|r"
#define EMOTE_NO_DESERTER "|cff00ffffУ вас нет 'Дезертира'!|r"
#define EMOTE_COMBAT "|cffff0000Вы в бою! Чтобы использовать данного NPC, выйдите из боя!|r"
#define EMOTE_NO_ARENA_POINTS "|cffff0000У вас недостаточно О.А. для совершения покупки!|r"
#define EMOTE_ALREADY_ITEM "|cff00ffffУ вас уже имеется данная вещь!|r"
#define EMOTE_NO_ETHEREAL_CREDITS "|cffff0000У вас недостаточно 'Эфириальных Монет'!|r"
#define EMOTE_ALREADY_SPELL "|cff00ffffУ вас уже имеется данное заклинание!|r"
#define EMOTE_ALREADY_ACHIEVEMENT "|cff00ffffУ вас уже имеется данное достижение!|r"

#define MSG_RENAME_COMPLETE "|cff00ff00Оплата успешно произведена. Сделайте логаут и введите новое имя своего персонажа.Не забудьте после смены имени, выйти из игры и удалить из клиента папку 'Cache'!|r"
#define MSG_CUSTOMIZE_COMPLETE "|cff00ff00Оплата успешно произведена. Сделайте логаут и измените внешность своего персонажа.Не забудьте после смены внешности, выйти из игры и удалить из клиента папку 'Cache'!|r"
#define MSG_CHANGE_FACTION_COMPLETE "|cff00ff00Оплата успешно произведена. Сделайте логаут и измените фракцию своего персонажа.Не забудьте после смены фракции, выйти из игры и удалить из клиента папку 'Cache'!|r" 
#define MSG_CHANGE_RACE_COMPLETE "|cff00ff00Оплата успешно произведена. Сделайте логаут и измените расу своего персонажа.Не забудьте после смены расы, выйти из игры и удалить из клиента папку 'Cache'!|r"
#define MSG_REMOVE_SICKNESS_COMPLETE "|cff00ff00'Слабость после воскрешения' снята и восполнены полностью здоровье и мана.|r"
#define MSG_REMOVE_DESERTER_COMPLETE "|cff00ff00'Дезертир' снят! Вы можете снова вернуться на 'Поле Боя'!|r"
#define MSG_MORPH_COMPLETE "|cff00ff00Облик вашего персонажа успешно изменен!|r"
#define MSG_DEMORPH_COMPLETE "|cff00ff00Облик вашего персонажа успешно изменен на стандартный!|r"
#define MSG_RESET_COOLDOWN "|cff0000ffВсе заклинания и способности успешно обновлены! КД сброшено!|r"
#define MSG_CHARACTER_SAVE_TO_DB "|cff0000ffВаш персонаж успешно сохранен!|r"
#define MSG_HEAL_COMPLETE "|cff00ff00Ваши здоровье и мана полностью восстановлены|r"
#define MSG_REPAIR_ITEMS_COMPLETE "|cff00ff00Все ваши вещи успешно починены!|r"
#define MSG_RESET_QUEST_STATUS_COMPLETE "|cff0000ffКД заданий успешно сброшено!|r"
#define MSG_RESET_RAID_INSTANCES_COOLDOWN_COMPLETE "|cff0000ffРейдовые подземелья успешно обновлены! КД сброшено!|r"

enum  defines
{
    ETHEREAL_CREDIT = 38186 // Эфириальная монета
};

class vip_item : public ItemScript
{
public:
    vip_item() : ItemScript("vip_item") { }

    

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override // Any hook here
    {
        if (player ->IsInCombat() || player ->IsInFlight() || player ->GetMap()->IsBattlegroundOrArena() || player ->HasStealthAura() || player ->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || player ->isDead() || player ->GetAreaId() == 616)
        {
            player->SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW, item, NULL);
            return true;
        }
        
        if (!player->GetSession()->IsPremium() && !player->IsGameMaster())
        {
            player->CLOSE_GOSSIP_MENU();
            player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_VIP);
            return true;
        }

        player->PlayerTalkClass->ClearMenus();
        player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_shadow_deathscream:30|t Снять 'Слабость после воскрешения'", GOSSIP_SENDER_MAIN, 1209);
        player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/ability_druid_cower:30|t Снять 'Дезертира'", GOSSIP_SENDER_MAIN, 1210);
        player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_divineillumination:30|t Исцеление", GOSSIP_SENDER_MAIN, 1202);
        player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_shadow_twistedfaith:30|t Сохранить персонажа", GOSSIP_SENDER_MAIN, 1213);
        player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/Trade_BlackSmithing:30|t Починка вещей", GOSSIP_SENDER_MAIN, 1207);
        player->ADD_GOSSIP_ITEM(0, "|TInterface/icons/Spell_chargepositive:30|tСменить Цвет Чата", GOSSIP_SENDER_MAIN, 100);
        player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_borrowedtime:30|t Сбросить 'КД'", GOSSIP_SENDER_MAIN, 1212);
        player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_borrowedtime:30|t Сбросить 'КД' заданий", GOSSIP_SENDER_MAIN, 1215);
        player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_borrowedtime:30|t Сбросить 'КД' рейдовых подземелий", GOSSIP_SENDER_MAIN, 1217);
        player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_crate_04:30|t Выдача сумок", GOSSIP_SENDER_MAIN, 1216);
        player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Изменение персонажа ->", GOSSIP_SENDER_MAIN, 1205);
        player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_shadow_nethercloak:30|t V.I.P. Морфы ->", GOSSIP_SENDER_MAIN, 1203);
        player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_auramastery:30|t V.I.P. Заклинания ->", GOSSIP_SENDER_MAIN, 1206);
        player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_holyguidance:30|t Покупка достижений ->", GOSSIP_SENDER_MAIN, 1211);
        player->SEND_GOSSIP_MENU(68,item->GetGUID());
        return true;
    }
    
    void OnGossipSelect(Player* player, Item* item, uint32 /*sender*/, uint32 action) override
    {
        if (!player->getAttackers().empty())
        {
            player->GetSession()->SendAreaTriggerMessage(EMOTE_COMBAT);
            player->CLOSE_GOSSIP_MENU();
            return;
        }

        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {
            
        case 100:
            {
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cffD00000Красный|r", GOSSIP_SENDER_MAIN, 101);	// готово
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cff3333CCСиний|r", GOSSIP_SENDER_MAIN, 102);	// готово
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cff009900Зелёный|r", GOSSIP_SENDER_MAIN, 103);	// готово
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cff0099FFГолубой|r", GOSSIP_SENDER_MAIN, 104);	// готово	
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cffFF99FFРозовый|r", GOSSIP_SENDER_MAIN, 105); // готово	
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cff9192a1Серый|r", GOSSIP_SENDER_MAIN, 107); // готово+
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cffb37700Коричневый|r", GOSSIP_SENDER_MAIN, 108); // готово+
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargepositive:25|t|cffffd9b3Бежевый|r", GOSSIP_SENDER_MAIN, 109); // готово+
                player->ADD_GOSSIP_ITEM(0, "|TInterface\\icons\\Spell_chargenegative:25|t|Вернуть обычный цвет", GOSSIP_SENDER_MAIN, 110); // готово
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, item->GetGUID());			
            }
            break;
            
        case 101: // Красный
            {
                LoginDatabase.PExecute("UPDATE account SET color = '1' WHERE id = %u", player->GetSession()->GetAccountId());	// |cffD00000 цвет
                ChatHandler(player->GetSession()).PSendSysMessage("|cffD00000Вы успешно изменили свой цвет чата на: Красный");
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
            
        case 102: // Синий
            {
                LoginDatabase.PExecute("UPDATE account SET color = '2' WHERE id = %u", player->GetSession()->GetAccountId());	// |cff3300CC цвет
                ChatHandler(player->GetSession()).PSendSysMessage("|cff3333CCВы успешно изменили свой цвет чата на: Синий");
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
            
        case 103: // Зелёный
            {
                LoginDatabase.PExecute("UPDATE account SET color = '3' WHERE id = %u", player->GetSession()->GetAccountId());	// |cff009900 цвет
                ChatHandler(player->GetSession()).PSendSysMessage("|cff009900Вы успешно изменили свой цвет чата на: Зелёный");
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
            
        case 104: // Голубой
            {
                LoginDatabase.PExecute("UPDATE account SET color = '4' WHERE id = %u", player->GetSession()->GetAccountId()); // |cff0099FF цвет
                ChatHandler(player->GetSession()).PSendSysMessage("|cff0099FFВы успешно изменили свой цвет чата на: Голубой");				
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
            
        case 105: // Розовый
            {
                LoginDatabase.PExecute("UPDATE account SET color = '5' WHERE id = %u", player->GetSession()->GetAccountId()); // |cffFF99FF цвет
                ChatHandler(player->GetSession()).PSendSysMessage("|cffFF99FFВы успешно изменили свой цвет чата на: Розовый");				
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
            
        case 107: // серый
            {
                LoginDatabase.PExecute("UPDATE account SET color = '7' WHERE id = %u", player->GetSession()->GetAccountId());
                ChatHandler(player->GetSession()).PSendSysMessage("|cff9192a1Вы успешно изменили свой цвет чата на: Серый");
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;	
            
        case 108: // Коричневый
            {
                LoginDatabase.PExecute("UPDATE account SET color = '8' WHERE id = %u", player->GetSession()->GetAccountId());
                ChatHandler(player->GetSession()).PSendSysMessage("|cffb37700Вы успешно изменили свой цвет чата на: Коричневый");
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;	

        case 109: // Бежевый
            {
                LoginDatabase.PExecute("UPDATE account SET color = '9' WHERE id = %u", player->GetSession()->GetAccountId());
                ChatHandler(player->GetSession()).PSendSysMessage("|cffffd9b3Вы успешно изменили свой цвет чата на: Бежевый");
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;	

        case 110: // обычный
            {
                LoginDatabase.PExecute("UPDATE account SET color = '0' WHERE id = %u", player->GetSession()->GetAccountId()); 
                ChatHandler(player->GetSession()).PSendSysMessage("Вы успешно изменили свой цвет чата на: Обычный");				
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
            
        case 1202: // Исцеление
            if (player->HasAura(45523))
            {
                player->CLOSE_GOSSIP_MENU();
                player->GetSession()->SendAreaTriggerMessage(EMOTE_COOLDOWN);
            }
            else
            {
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, 25840, true);
                player->CastSpell(player, 45523, true);
            }
            break;
        case 1207: // Починка вещей
            player->CLOSE_GOSSIP_MENU();
            player->DurabilityRepairAll(false, 0, false);
            player->GetSession()->SendAreaTriggerMessage(MSG_REPAIR_ITEMS_COMPLETE);
            break;
        case 1203: // V.I.P. Морфы
            player->PlayerTalkClass->ClearMenus();
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Пират (Мужчина)|r", GOSSIP_SENDER_MAIN, 499);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Пират (Женщина)|r", GOSSIP_SENDER_MAIN, 500);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Ниндзя (Мужчина)|r", GOSSIP_SENDER_MAIN, 501);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Ниндзя (Женщина)|r", GOSSIP_SENDER_MAIN, 502);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Ворген|r", GOSSIP_SENDER_MAIN, 503);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Огр|r", GOSSIP_SENDER_MAIN, 504);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Крок Гроза Плети|r", GOSSIP_SENDER_MAIN, 505);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Таунка|r", GOSSIP_SENDER_MAIN, 506);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Алекстраза Хранительница Жизни|r", GOSSIP_SENDER_MAIN, 507);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Акама|r", GOSSIP_SENDER_MAIN, 508);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Механогном|r", GOSSIP_SENDER_MAIN, 509);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Гоблин (Мужчина)|r", GOSSIP_SENDER_MAIN, 510);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Гоблин (Женщина)|r", GOSSIP_SENDER_MAIN, 511);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Кровавая королева Лана'тель|r", GOSSIP_SENDER_MAIN, 512);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Темная валь'кира|r", GOSSIP_SENDER_MAIN, 513);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Светлая валь'кира|r", GOSSIP_SENDER_MAIN, 514);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Метаморфоза|r", GOSSIP_SENDER_MAIN, 515);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Медив|r", GOSSIP_SENDER_MAIN, 516);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Верховный правитель Саурфанг|r", GOSSIP_SENDER_MAIN, 517);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_shadow_twistedfaith:30|t Снять морф|r", GOSSIP_SENDER_MAIN, 600);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:30|t Следующая страница|r", GOSSIP_SENDER_MAIN, 1208);
            player->SEND_GOSSIP_MENU(68,item->GetGUID());
            break;
        case 1208:
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Леди Сильвана Ветрокрылая|r", GOSSIP_SENDER_MAIN, 518);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Король Вариан Ринн|r", GOSSIP_SENDER_MAIN, 519);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Тралл", GOSSIP_SENDER_MAIN, 520);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Тиранда Шелест Ветра|r", GOSSIP_SENDER_MAIN, 521);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Малфурион Ярость Бури|r", GOSSIP_SENDER_MAIN, 522);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Терон Кровожад|r", GOSSIP_SENDER_MAIN, 523);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Леди Джайна Праудмур|r", GOSSIP_SENDER_MAIN, 524);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Рексар|r", GOSSIP_SENDER_MAIN, 525);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Fel Orc|r", GOSSIP_SENDER_MAIN, 526);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Broken Draenei|r", GOSSIP_SENDER_MAIN, 527);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Рыцарь смерти|r", GOSSIP_SENDER_MAIN, 528);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Вурдалак|r", GOSSIP_SENDER_MAIN, 529);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Верховный лорд Болвар Фордрагон|r", GOSSIP_SENDER_MAIN, 530);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Саурфанг Смертоносный|r", GOSSIP_SENDER_MAIN, 531);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Тирион Фордринг|r", GOSSIP_SENDER_MAIN, 532);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Артас Менетил (Король-лич)|r", GOSSIP_SENDER_MAIN, 533);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Артас Менетил (Принц Лордерона)|r", GOSSIP_SENDER_MAIN, 534);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Железный дворф|r", GOSSIP_SENDER_MAIN, 535);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_shadow_twistedfaith:30|t Снять морф|r", GOSSIP_SENDER_MAIN, 600);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:30|t Предыдущая страница|r", GOSSIP_SENDER_MAIN, 1203);
            player->SEND_GOSSIP_MENU(68, item->GetGUID());
            break;
        case 499:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Пират (Мужчина)
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(4620);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 500:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Пират(Женщина)
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(4619);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 501:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Ниндзя (Мужчина)
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(4617);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 502:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Ниндзя (Женщина)
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(4618);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 503:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Ворген
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(657);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 504:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Огр
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(19752);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 505:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Крок Гроза Плети
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(30911);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 506:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Таунка
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(24970);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 507:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Алекстраза Хранительница Жизни 
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(28227);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 508:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Акама
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(20681);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 509:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Механогном
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(24115);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 510:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Гоблин (Мужчина)
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(20582);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 511:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Гоблин (Женщина)
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(20583);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 512:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Кровавая королева Лана'тель
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(31093);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.4);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 513:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Темная валь'кира
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(29267);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.5);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 514:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Светлая валь'кира
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(29240);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.5);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 515:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Метаморфоза
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(25277);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 516:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Медив
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(18718);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.8);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 517:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Верховный правитель Саурфанг
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(14732);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 518:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Леди Сильвана Ветрокрылая
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(28213);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 519:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Король Вариан Ринн
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(28127);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 520:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Тралл
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(4527);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 521:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Тиранда Шелест Ветра
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(7274);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 522:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Малфурион Ярость Бури
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(15399);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.5);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 523:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Терон Кровожад
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(21262);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.5);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 524:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Леди Джайна Праудмур
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(30863);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 525:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Рексар
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(20918);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.6);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 526:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Fel Orc
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(21267);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 527:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Broken Draenei
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(21105);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 528:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Рыцарь смерти
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(26571);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.6);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 529:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Вурдалак
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(22496);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.6);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 530:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Верховный лорд Болвар Фордрагон
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(24879);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.8);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 531:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Саурфанг Смертоносный
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(30790);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 532:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Тирион Фордринг
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(31286);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.6);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 533:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Артас Менетил (Король-лич)
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(27545);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 534:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Артас Менетил (Принц Лордерона)
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(24949);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 535:
            if (player->HasItemCount(ETHEREAL_CREDIT, 4))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 4, true, false);
                player->GetSession()->SendAreaTriggerMessage(MSG_MORPH_COMPLETE);
                player->CLOSE_GOSSIP_MENU(); // Железный дворф
                player->CastSpell(player, 52866, true);
                player->SetDisplayId(26239);
                player->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.8);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 600: // Demorph Player (Remove all morphs)
            player->GetSession()->SendAreaTriggerMessage(MSG_DEMORPH_COMPLETE);
            player->CLOSE_GOSSIP_MENU();
            player->DeMorph();
            break;
        case 1206: // V.I.P. Заклинания
            player->PlayerTalkClass->ClearMenus();
            player->GetSession()->SendAreaTriggerMessage("|cffff0000Важно!|r |cff00ffffСтоимость покупки одного V.I.P. заклинания|r |cffff000010|r |cff00ffff'Эфириальных монет'!|r");
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_misc_gem_pearl_04:30|t Эликсир Гогельмогеля", GOSSIP_SENDER_MAIN, 4000);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_misc_gem_pearl_04:30|t Увеличитель мира", GOSSIP_SENDER_MAIN, 4001);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_misc_gem_pearl_04:30|t Статуэтка Потерянного времени", GOSSIP_SENDER_MAIN, 4002);
            //player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_misc_gem_pearl_04:30|t Поминовение усопших", GOSSIP_SENDER_MAIN, 4003);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_misc_gem_pearl_04:30|t Варево Бешеного Сердца", GOSSIP_SENDER_MAIN,4004);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_misc_gem_pearl_04:30|t Железная походная фляга", GOSSIP_SENDER_MAIN, 4005);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_misc_gem_pearl_04:30|t Сфера Обмана", GOSSIP_SENDER_MAIN, 4006);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_misc_gem_pearl_04:30|t Иллюзия Зиморожденных", GOSSIP_SENDER_MAIN, 4007);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_misc_gem_pearl_04:30|t Броня огров Гордока", GOSSIP_SENDER_MAIN, 4008);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_misc_gem_pearl_04:30|t Гномский транспортер", GOSSIP_SENDER_MAIN, 4009);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_misc_gem_pearl_04:30|t Транспортер Зоны 52", GOSSIP_SENDER_MAIN, 4010);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_misc_gem_pearl_04:30|t Транспортер: Круговзор", GOSSIP_SENDER_MAIN, 4011);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_misc_gem_pearl_04:30|t Передатчик станции Тошли", GOSSIP_SENDER_MAIN, 4012);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_misc_gem_pearl_04:30|t Путь Иллидана", GOSSIP_SENDER_MAIN, 4013);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/inv_misc_gem_pearl_04:30|t Путь Кенария", GOSSIP_SENDER_MAIN, 4014);
            //player->ADD_GOSSIP_ITEM(5, "|TInterface/ICONS/inv_misc_qirajicrystal_05:30|t Черный киражский боевой танк", GOSSIP_SENDER_MAIN, 4015);
            player->SEND_GOSSIP_MENU(68,item->GetGUID());
            break;
        case 4000: // Эликсир Гогельмогеля
            if (player->HasSpell(16589))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_gem_pearl_04:20|tЭликсир Гогельмогеля'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(16589, false);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 4001: // Увеличитель мира
            if (player->HasSpell(23126))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_gem_pearl_04:20|tУвеличитель мира'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(23126, false);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 4002: // Статуэтка Потерянного времени
            if (player->HasSpell(41301))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_gem_pearl_04:20|tСтатуэтка Потерянного времени'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(41301, false);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 4003: // Поминовение усопших
            if (player->HasSpell(65386))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_gem_pearl_04:20|tПоминовение усопших'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(65386, false);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 4004: // Варево Бешеного Сердца
            if (player->HasSpell(61340))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_gem_pearl_04:20|tВарево Бешеного Сердца'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(61340, false);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 4005: // Железная походная фляга
            if (player->HasSpell(58501))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_gem_pearl_04:20|tЖелезная походная фляга'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(58501, false);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 4006: // Сфера Обмана
            if (player->HasSpell(16739))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_gem_pearl_04:20|tСфера Обмана'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(16739, false);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 4007: // Иллюзия Зиморожденных
            if (player->HasSpell(73320))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_gem_pearl_04:20|tИллюзия Зиморожденных'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(73320, false);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 4008: // Броня огров Гордока
            if (player->HasSpell(22736))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_gem_pearl_04:20|tБроня огров Гордока'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(22736, false);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 4009: // Гномский транспортер
            if (player->HasSpell(23453))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_gem_pearl_04:20|tГномский транспортер'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(23453, false);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 4010: // Транспортер Зоны 52
            if (player->HasSpell(36890))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_gem_pearl_04:20|tТранспортер Зоны 52'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(36890, false);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 4011: // Транспортер: Круговзор
            if (player->HasSpell(23442))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_gem_pearl_04:20|tТранспортер: Круговзор'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(23442, false);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 4012: // Передатчик станции Тошли
            if (player->HasSpell(36941))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_gem_pearl_04:20|tПередатчик станции Тошли'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(36941, true);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 4013: // Путь Иллидана
            if (player->HasSpell(50247))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_gem_pearl_04:20|tПуть Иллидана'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(50247, true);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 4014: // Путь Кенария
            if (player->HasSpell(65745))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_gem_pearl_04:20|tПуть Кенария'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(65745, true);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 4015: // Черный киражский боевой танк
            if (player->HasSpell(31700))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_SPELL);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 10))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 10, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили V.I.P. заклинание|r '|TInterface/ICONS/inv_misc_qirajicrystal_05:20|tЧерный киражский боевой танк'");
                player->CLOSE_GOSSIP_MENU();
                player->LearnSpell(31700, true);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 1205:
            player->PlayerTalkClass->ClearMenus();
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Смена имени [|cffff000010|r Очков Арены]", GOSSIP_SENDER_MAIN, 2000);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Смена внешности [|cffff000010|r Очков Арены]", GOSSIP_SENDER_MAIN, 2001);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Смена фракции [|cffff000050|r Очков Арены]", GOSSIP_SENDER_MAIN, 2002);
            player->ADD_GOSSIP_ITEM(6, "|TInterface/ICONS/spell_holy_rapture:30|t Смена расы [|cffff000025|r Очков Арены]", GOSSIP_SENDER_MAIN, 2003);
            player->SEND_GOSSIP_MENU(68,item->GetGUID());
            break;	
        case 2000: // Смена имени
            if (player->GetArenaPoints() < CONST_ARENA_RENAME)
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ARENA_POINTS);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->CLOSE_GOSSIP_MENU();
                player->SetAtLoginFlag(AT_LOGIN_RENAME);
                player->ModifyArenaPoints(-CONST_ARENA_RENAME);
                player->GetSession()->SendAreaTriggerMessage(MSG_RENAME_COMPLETE);
            }
            break;
        case 2001: // Смена внешности
            if (player->GetArenaPoints() < CONST_ARENA_CUSTOMIZE)
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ARENA_POINTS);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->CLOSE_GOSSIP_MENU();
                player->SetAtLoginFlag(AT_LOGIN_CUSTOMIZE);
                player->ModifyArenaPoints(-CONST_ARENA_CUSTOMIZE);
                player->GetSession()->SendAreaTriggerMessage(MSG_CUSTOMIZE_COMPLETE);
            }
            break;
        case 2002: // Смена фракции
            if (player->GetArenaPoints() < CONST_ARENA_CHANGE_FACTION)
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ARENA_POINTS);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->CLOSE_GOSSIP_MENU();
                player->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
                player->ModifyArenaPoints(-CONST_ARENA_CHANGE_FACTION);
                player->GetSession()->SendAreaTriggerMessage(MSG_CHANGE_FACTION_COMPLETE);
            }
            break;
        case 2003: // Смена расы
            if (player->GetArenaPoints() < CONST_ARENA_CHANGE_RACE)
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ARENA_POINTS);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->CLOSE_GOSSIP_MENU();
                player->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
                player->ModifyArenaPoints(-CONST_ARENA_CHANGE_RACE);
                player->GetSession()->SendAreaTriggerMessage(MSG_CHANGE_RACE_COMPLETE);
            }
            break;
        case 1209: // Снять 'Слабость после воскрешения'
            if(!player->HasAura(15007))
            {
                player->CLOSE_GOSSIP_MENU();
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_SICKENSS);
            }
            else
            {
                player->CLOSE_GOSSIP_MENU();
                player->RemoveAurasDueToSpell(15007);
                player->SetHealth(player->GetMaxHealth()); // Restore Health
                player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA)); // Restore Mana
                player->GetSession()->SendAreaTriggerMessage(MSG_REMOVE_SICKNESS_COMPLETE);
            }
            break;
        case 1210: // Снять 'Дезертира'
            if(!player->HasAura(26013))
            {
                player->CLOSE_GOSSIP_MENU();
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_DESERTER);
            }
            else
            {
                player->CLOSE_GOSSIP_MENU();
                player->RemoveAurasDueToSpell(26013);
                player->GetSession()->SendAreaTriggerMessage(MSG_REMOVE_DESERTER_COMPLETE);
            }
            break;
        case 1212: // Сбросить 'КД'
            if (player->HasAura(45523))
            {
                player->CLOSE_GOSSIP_MENU();
                player->GetSession()->SendAreaTriggerMessage(EMOTE_COOLDOWN);
            }
            else
            {
                player->CLOSE_GOSSIP_MENU();
                player->RemoveArenaSpellCooldowns(true);
                player->GetSession()->SendAreaTriggerMessage(MSG_RESET_COOLDOWN);
                player->CastSpell(player, 45523, true);
            }
            break;
        case 1213: // Сохранить персонажа
            player->CLOSE_GOSSIP_MENU();
            player->SaveToDB();
            player->GetSession()->SendAreaTriggerMessage(MSG_CHARACTER_SAVE_TO_DB);
            break;
        case 1215: // Сбросить 'КД' заданий
            player->CLOSE_GOSSIP_MENU();
            player->ResetDailyQuestStatus();
            player->ResetWeeklyQuestStatus();
            player->GetSession()->SendAreaTriggerMessage(MSG_RESET_QUEST_STATUS_COMPLETE);
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
                player->GetSession()->SendAreaTriggerMessage(MSG_RESET_RAID_INSTANCES_COOLDOWN_COMPLETE);
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 1216: // Выдача сумок
            if (player->HasItemCount(23162,4))
            {
                player->CLOSE_GOSSIP_MENU();
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_ITEM);
            }
            else
            {
                player->CLOSE_GOSSIP_MENU();
                player->AddItem(23162, 4);
            }
            break;
        case 1211: // Покупка достижений
            player->PlayerTalkClass->ClearMenus();
            player->GetSession()->SendAreaTriggerMessage("|cffff0000Важно!|r |cff00ffffСтоимость покупки одного достижения|r |cffff000025|r |cff00ffff'Эфириальный монет!'|r");
            player->ADD_GOSSIP_ITEM(5, "|TInterface/ICONS/achievement_zone_silithus_01:30|t Король-скарабей", GOSSIP_SENDER_MAIN, 5000);
            player->ADD_GOSSIP_ITEM(5, "|TInterface/ICONS/inv_mace_25:30|t Длань А'дала", GOSSIP_SENDER_MAIN, 5001);
            player->ADD_GOSSIP_ITEM(5, "|TInterface/ICONS/inv_mace_51:30|t Защитник наару", GOSSIP_SENDER_MAIN, 5002);
            player->ADD_GOSSIP_ITEM(5, "|TInterface/ICONS/achievement_boss_onyxia:30|t Логово Ониксии (60-й уровень)", GOSSIP_SENDER_MAIN, 5003);
            player->ADD_GOSSIP_ITEM(5, "|TInterface/ICONS/inv_misc_celebrationcake_01:30|t 4-я годовщина World of Warcraft", GOSSIP_SENDER_MAIN, 5004);
            player->ADD_GOSSIP_ITEM(5, "|TInterface/ICONS/inv_misc_celebrationcake_01:30|t 5-я годовщина World of Warcraft", GOSSIP_SENDER_MAIN, 5005);
            player->ADD_GOSSIP_ITEM(5, "|TInterface/ICONS/spell_shadow_deathscream:30|t Чертовски редкие", GOSSIP_SENDER_MAIN, 5006);
            player->ADD_GOSSIP_ITEM(5, "|TInterface/ICONS/achievement_zone_dragonblight_09:30|t Обморожение", GOSSIP_SENDER_MAIN, 5007);
            player->ADD_GOSSIP_ITEM(5, "|TInterface/ICONS/inv_misc_book_04:30|t Эрудированность", GOSSIP_SENDER_MAIN, 5008);
            player->ADD_GOSSIP_ITEM(5, "|TInterface/ICONS/inv_misc_book_11:30|t Ученье - свет!", GOSSIP_SENDER_MAIN, 5009);
            player->SEND_GOSSIP_MENU(68, item->GetGUID());
            break;
        case 5000: // Король-скарабей
            if (player->HasAchieved(416))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_ACHIEVEMENT);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 25))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 25, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/achievement_zone_silithus_01:30|t Король-скарабей'");
                player->CLOSE_GOSSIP_MENU();
                player->CompletedAchievement(sAchievementMgr->GetAchievement(416));
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 5001: // Длань А'дала
            if (player->HasAchieved(431))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_ACHIEVEMENT);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 25))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 25, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/inv_mace_25:30|t Длань А'дала'");
                player->CLOSE_GOSSIP_MENU();
                player->CompletedAchievement(sAchievementMgr->GetAchievement(431));
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 5002: // Защитник наару
            if (player->HasAchieved(432))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_ACHIEVEMENT);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 25))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 25, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/inv_mace_51:30|t Защитник наару'");
                player->CLOSE_GOSSIP_MENU();
                player->CompletedAchievement(sAchievementMgr->GetAchievement(432));
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 5003: // Логово Ониксии (60-й уровень)
            if (player->HasAchieved(684))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_ACHIEVEMENT);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 25))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 25, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/achievement_boss_onyxia:30|t Логово Ониксии (60-й уровень)'");
                player->CLOSE_GOSSIP_MENU();
                player->CompletedAchievement(sAchievementMgr->GetAchievement(684));
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 5004: // 4-я годовщина World of Warcraft
            if (player->HasAchieved(2398))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_ACHIEVEMENT);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 25))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 25, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/inv_misc_celebrationcake_01:30|t 4-я годовщина World of Warcraft");
                player->CLOSE_GOSSIP_MENU();
                player->CompletedAchievement(sAchievementMgr->GetAchievement(2398));
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 5005: // 5-я годовщина World of Warcraft
            if (player->HasAchieved(4400))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_ACHIEVEMENT);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 25))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 25, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/inv_misc_celebrationcake_01:30|t 5-я годовщина World of Warcraft");
                player->CLOSE_GOSSIP_MENU();
                player->CompletedAchievement(sAchievementMgr->GetAchievement(4400));
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 5006: // Чертовски редкие
            if (player->HasAchieved(1312))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_ACHIEVEMENT);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 25))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 25, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/spell_shadow_deathscream:25|t Чертовски редкие'");
                player->CLOSE_GOSSIP_MENU();
                player->CompletedAchievement(sAchievementMgr->GetAchievement(1312));
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 5007: // Обморожение
            if (player->HasAchieved(2257))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_ACHIEVEMENT);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 25))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 25, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/achievement_zone_dragonblight_09:25|t Обморожение'");
                player->CLOSE_GOSSIP_MENU();
                player->CompletedAchievement(sAchievementMgr->GetAchievement(2257));
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 5008: // Эрудированность
            if (player->HasAchieved(1244))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_ACHIEVEMENT);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 25))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 25, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/inv_misc_book_04:25|t Эрудированность'");
                player->CLOSE_GOSSIP_MENU();
                player->CompletedAchievement(sAchievementMgr->GetAchievement(1244));
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        case 5009: // Ученье - свет!
            if (player->HasAchieved(1956))
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_ALREADY_ACHIEVEMENT);
                player->PlayerTalkClass->SendCloseGossip();
            }
            else if (player->HasItemCount(ETHEREAL_CREDIT, 25))
            {
                player->DestroyItemCount(ETHEREAL_CREDIT, 25, true, false);
                player->GetSession()->SendAreaTriggerMessage("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/inv_misc_book_11:30|t Ученье - свет!'");
                player->CLOSE_GOSSIP_MENU();
                player->CompletedAchievement(sAchievementMgr->GetAchievement(1956));
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage(EMOTE_NO_ETHEREAL_CREDITS);
                player->PlayerTalkClass->SendCloseGossip();
            }
            break;
        }
    }
};


class item_custom_level : public ItemScript
{
public:
    item_custom_level() : ItemScript("item_custom_level") {}

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) 
    {
        if (player->IsInCombat())
        {
            player->GetSession()->SendNotification("Вы не можете использовать данный итем в бою");
            return false;
        }
        if (player->IsInFlight())
        {
            player->GetSession()->SendNotification("Вы не можете использовать данный итем в полете");
            return false;
        }
        if (player->getClass() == CLASS_DEATH_KNIGHT)
        {
            player->GetSession()->SendNotification("Вы не можете использовать данный итем.");
            return false;
        }
        if (player->isDead()) // Is player mounted
        {
            player->GetSession()->SendNotification("Вы не можете использовать данный итем пока мертвы.");
            return true;
        }
        if (player->IsMounted()) // Is player mounted
        {
            player->GetSession()->SendNotification("Вы не можете использовать данный итем находять на средстве передвижения");
            return true;
        }
        if (player->getLevel() == 80 || player->getLevel() > 80)
        {
            player->GetSession()->SendNotification("Вы уже достигли максимального уровня.");
            return true;
        }
        player->GiveLevel(80);
        player->DestroyItemCount(44728, 1, true);
        player->SaveToDB();

    }
};

void AddSC_custom_item_gossip()
{
    new item_beastmaster;
    new buff_item;
    new item_custom_summon;
    new vip_item;
    new item_custom_level;
}