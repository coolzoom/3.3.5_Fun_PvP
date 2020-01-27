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

#include "InstanceSaveMgr.h"
#include "GroupMgr.h"
#include "Language.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "Group.h"
#include "SocialMgr.h"

enum ArmorSpells
{
	PLATE_MAIL       = 750,
	ARMOR_MAIL       = 8737
};

enum WeaponProficiencies
{
	BLOCK            = 107,
	BOWS             = 264,
	CROSSBOWS        = 5011,
	DAGGERS          = 1180,
	FIST_WEAPONS     = 15590,
	GUNS             = 266,
	ONE_H_AXES       = 196,
	ONE_H_MACES      = 198,
	ONE_H_SWORDS     = 201,
	POLEARMS         = 200,
	SHOOT            = 5019,
	STAVES           = 227,
	TWO_H_AXES       = 197,
	TWO_H_MACES      = 199,
	TWO_H_SWORDS     = 202,
	WANDS            = 5009,
	THROW_WAR        = 2567
};

class CopyGearCommands : public CommandScript
{
public:
	CopyGearCommands() : CommandScript("CopyGearCommands") { }

	std::vector<ChatCommand> GetCommands() const
	{
		static std::vector<ChatCommand> HandleCopyGearCommandTable =
		{
			{ "gear", rbac::RBAC_PERM_COMMAND_COPY_GEAR, false, &HandleCopyGearCommand, "" },
			{ "talents", rbac::RBAC_PERM_COMMAND_COPY_TALENTS, false, &HandleCopyTalentsCommand, "" },
			{ "all", rbac::RBAC_PERM_COMMAND_COPY_ALL, false, &HandleCopyGearAndTalentsCommand, "" },
		};

		static std::vector<ChatCommand> commandTable =
		{
			{ "copy", rbac::RBAC_PERM_COMMAND_COPY, true, NULL, "", HandleCopyGearCommandTable },
		};
		return commandTable;
	}

	static bool HandleCopyGearCommand(ChatHandler* handler, const char* args)
	{
		std::string name;
		Player* player = handler->GetSession()->GetPlayer();
		Player* target;
		char* targetName = strtok((char*)args, " "); // Get entered name

		if (targetName)
		{
			name = targetName;
			normalizePlayerName(name);
			target = ObjectAccessor::FindPlayerByName(name.c_str());
		}
		else // If no name was entered - use target
		{
			target = handler->getSelectedPlayer();
			if (target)
				name = target->GetName().c_str();
		}

		if (target)
		{
			if (target == player)
			{
				handler->PSendSysMessage("Вы не можете скопировать себя.");
				return false;
			}

			if (player->getClass() != target->getClass())
			{
				handler->PSendSysMessage("Ваша цель,не соотвествует вашему классу.");
				return false;
			}

			// Restrict copying from the opposite faction
			if (player->GetTeam() != target->GetTeam())
			{
				handler->PSendSysMessage("Вы можете скопировать,только игроков своей фракции.");
				return false;
			}
			else if (target->getRace() == RACE_HUMAN && player->getRace() != RACE_HUMAN)
			{
				handler->PSendSysMessage("Успешно скопирован эквип у %s",
					target->GetName().c_str());
				return false;
			}

			CopyGear(player, target);
			handler->PSendSysMessage("Успешно скопирован эквип у %s",
				target->GetName().c_str());

			LearnPlateMailSpells(player);
			LearnWeaponSkills(player);
			player->UpdateSkillsForLevel();
			player->SaveToDB();
			return true;
		}
		else
			handler->PSendSysMessage("Выберете цель.");

		return true;
	}

	static bool HandleCopyTalentsCommand(ChatHandler* handler, const char* args)
	{
		std::string name;
		Player* player = handler->GetSession()->GetPlayer();
		Player* target;
		char* targetName = strtok((char*)args, " "); // Get entered name

		if (targetName)
		{
			name = targetName;
			normalizePlayerName(name);
			target = ObjectAccessor::FindPlayerByName(name.c_str());
		}
		else // If no name was entered - use target
		{
			target = handler->getSelectedPlayer();
			if (target)
				name = target->GetName().c_str();
		}

		if (target)
		{
			if (target == player)
			{
				handler->PSendSysMessage("Вы не можете скопировать себя.");
				return false;
			}

			if (player->getClass() != target->getClass())
			{
				handler->PSendSysMessage("Ваша цель,не соотвествует вашему классу.");
				return false;
			}

			CopyTalents(player, target);
			handler->PSendSysMessage("Успешно скопированы таланты у %s",
				target->GetName().c_str());

			player->UpdateSkillsForLevel();
			player->SaveToDB();
			return true;
		}
		else
			handler->PSendSysMessage("Выберете цель.");

		return true;
	}

	static bool HandleCopyGearAndTalentsCommand(ChatHandler* handler, const char* args)
	{
		std::string name;
		Player* player = handler->GetSession()->GetPlayer();
		Player* target;
		char* targetName = strtok((char*)args, " "); // Get entered name

		if (targetName)
		{
			name = targetName;
			normalizePlayerName(name);
			target = ObjectAccessor::FindPlayerByName(name.c_str());
		}
		else // If no name was entered - use target
		{
			target = handler->getSelectedPlayer();
			if (target)
				name = target->GetName().c_str();
		}

		if (target)
		{
			if (target == player)
			{
				handler->PSendSysMessage("Вы не можете скопировать себя.");
				return false;
			}

			if (player->getClass() != target->getClass())
			{
				handler->PSendSysMessage("Ваша цель,не соотвествует вашему классу.");
				return false;
			}

			// Restrict copying from the opposite faction
			if (player->GetTeam() != target->GetTeam())
			{
				handler->PSendSysMessage("Вы можете скопировать,только игроков своей фракции.");
				return false;
			}
			else if (target->getRace() == RACE_HUMAN && player->getRace() != RACE_HUMAN)
			{
				handler->PSendSysMessage("Успешно скопировано у %s",
					target->GetName().c_str());
				return false;
			}

			CopyGear(player, target);
			CopyTalents(player, target);
			handler->PSendSysMessage("Успешно скопирован эквип и таланты у %s",
				target->GetName().c_str());

			LearnPlateMailSpells(player);
			LearnWeaponSkills(player);
			player->UpdateSkillsForLevel();
			player->SaveToDB();
			return true;
		}
		else
			handler->PSendSysMessage("Выберете цель.");

		return true;
	}

private:
	static void ApplyBonus(Player* player, Item* item, EnchantmentSlot slot, uint32 bonusEntry)
	{
		if (!item)
			return;

		if (!bonusEntry || bonusEntry == 0)
			return;

		player->ApplyEnchantment(item, slot, false);
		item->SetEnchantment(slot, bonusEntry, 0, 0);
		player->ApplyEnchantment(item, slot, true);
	}

	// Get the gear from the selected player
	static void CopyGear(Player* player, Player* target)
	{
		for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
		{
			// Delete previous items if equipped
			player->DestroyItem(INVENTORY_SLOT_BAG_0, i, true);

			if (Item* equippedItem = target->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
			{
				player->EquipNewItem(i, target->GetItemByPos(INVENTORY_SLOT_BAG_0, i)->GetEntry(), true);
				ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, i),
					PERM_ENCHANTMENT_SLOT, target->GetItemByPos(INVENTORY_SLOT_BAG_0, i)->GetEnchantmentId(PERM_ENCHANTMENT_SLOT));
				ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, i),
					SOCK_ENCHANTMENT_SLOT, target->GetItemByPos(INVENTORY_SLOT_BAG_0, i)->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT));
				ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, i),
					SOCK_ENCHANTMENT_SLOT_2, target->GetItemByPos(INVENTORY_SLOT_BAG_0, i)->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2));
				ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, i),
					SOCK_ENCHANTMENT_SLOT_3, target->GetItemByPos(INVENTORY_SLOT_BAG_0, i)->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3));
				ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, i),
					BONUS_ENCHANTMENT_SLOT, target->GetItemByPos(INVENTORY_SLOT_BAG_0, i)->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT));
				ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, i),
					PRISMATIC_ENCHANTMENT_SLOT, target->GetItemByPos(INVENTORY_SLOT_BAG_0, i)->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT));
			}
		}
	}

	static void CopyTalents(Player* player, Player* target)
	{
		// Reset talents if player already have some talent points spent
		if (player->GetFreeTalentPoints() < 71)
		{
			player->ResetTalents(true);
			player->SendTalentsInfoData(false);
		}

		for (uint32 talentEntry = 0; talentEntry < sTalentStore.GetNumRows(); ++talentEntry)
		{
			TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentEntry);

			if (!talentInfo)
				continue;

			for (int8 rank = MAX_TALENT_RANK - 1; rank >= 0; --rank)
			{
				if (talentInfo->RankID[rank] == 0)
					continue;

				if (target->HasTalent(talentInfo->RankID[rank], target->GetActiveSpec()))
				{
					player->LearnSpell(talentInfo->RankID[rank], false);
					player->AddTalent(talentInfo->RankID[rank], player->GetActiveSpec(), true);
				}
			}
		}

		CopyGlyphs(player, target);
		player->SetFreeTalentPoints(target->GetFreeTalentPoints());
		player->SendTalentsInfoData(false);
	}

	static void CopyGlyphs(Player* player, Player* target)
	{
		for (uint8 slot = 0; slot < MAX_GLYPH_SLOT_INDEX; ++slot)
		{
			uint32 glyph = target->GetGlyph(slot);

			// Skip empty slots
			if (glyph == 0)
				continue;

			if (GlyphPropertiesEntry const* gp = sGlyphPropertiesStore.LookupEntry(glyph))
			{
				if (uint32 oldGlyph = player->GetGlyph(slot))
				{
					player->RemoveAurasDueToSpell(sGlyphPropertiesStore.LookupEntry(oldGlyph)->SpellId);
					player->SetGlyph(slot, 0);
				}
				player->CastSpell(player, gp->SpellId, true);
				player->SetGlyph(slot, glyph);
			}
		}
	}

	static void LearnWeaponSkills(Player* player)
	{
		WeaponProficiencies wepSkills[] = {
			BLOCK, BOWS, CROSSBOWS, DAGGERS, FIST_WEAPONS, GUNS, ONE_H_AXES, ONE_H_MACES,
			ONE_H_SWORDS, POLEARMS, SHOOT, STAVES, TWO_H_AXES, TWO_H_MACES, TWO_H_SWORDS, WANDS, THROW_WAR
		};

		uint32 size = 17;

		for (uint32 i = 0; i < size; ++i)
			if (player->HasSpell(wepSkills[i]))
				continue;

		switch (player->getClass())
		{
		case CLASS_WARRIOR:
			player->LearnSpell(THROW_WAR, false);
			player->LearnSpell(TWO_H_SWORDS, false);
			player->LearnSpell(TWO_H_MACES, false);
			player->LearnSpell(TWO_H_AXES, false);
			player->LearnSpell(STAVES, false);
			player->LearnSpell(POLEARMS, false);
			player->LearnSpell(ONE_H_SWORDS, false);
			player->LearnSpell(ONE_H_MACES, false);
			player->LearnSpell(ONE_H_AXES, false);
			player->LearnSpell(GUNS, false);
			player->LearnSpell(FIST_WEAPONS, false);
			player->LearnSpell(DAGGERS, false);
			player->LearnSpell(CROSSBOWS, false);
			player->LearnSpell(BOWS, false);
			player->LearnSpell(BLOCK, false);
			break;
		case CLASS_PRIEST:
			player->LearnSpell(WANDS, false);
			player->LearnSpell(STAVES, false);
			player->LearnSpell(SHOOT, false);
			player->LearnSpell(ONE_H_MACES, false);
			player->LearnSpell(DAGGERS, false);
			break;
		case CLASS_PALADIN:
			player->LearnSpell(TWO_H_SWORDS, false);
			player->LearnSpell(TWO_H_MACES, false);
			player->LearnSpell(TWO_H_AXES, false);
			player->LearnSpell(POLEARMS, false);
			player->LearnSpell(ONE_H_SWORDS, false);
			player->LearnSpell(ONE_H_MACES, false);
			player->LearnSpell(ONE_H_AXES, false);
			player->LearnSpell(BLOCK, false);
			break;
		case CLASS_ROGUE:
			player->LearnSpell(ONE_H_SWORDS, false);
			player->LearnSpell(ONE_H_MACES, false);
			player->LearnSpell(ONE_H_AXES, false);
			player->LearnSpell(GUNS, false);
			player->LearnSpell(FIST_WEAPONS, false);
			player->LearnSpell(DAGGERS, false);
			player->LearnSpell(CROSSBOWS, false);
			player->LearnSpell(BOWS, false);
			break;
		case CLASS_DEATH_KNIGHT:
			player->LearnSpell(TWO_H_SWORDS, false);
			player->LearnSpell(TWO_H_MACES, false);
			player->LearnSpell(TWO_H_AXES, false);
			player->LearnSpell(POLEARMS, false);
			player->LearnSpell(ONE_H_SWORDS, false);
			player->LearnSpell(ONE_H_MACES, false);
			player->LearnSpell(ONE_H_AXES, false);
			break;
		case CLASS_MAGE:
			player->LearnSpell(WANDS, false);
			player->LearnSpell(STAVES, false);
			player->LearnSpell(SHOOT, false);
			player->LearnSpell(ONE_H_SWORDS, false);
			player->LearnSpell(DAGGERS, false);
			break;
		case CLASS_SHAMAN:
			player->LearnSpell(TWO_H_MACES, false);
			player->LearnSpell(TWO_H_AXES, false);
			player->LearnSpell(STAVES, false);
			player->LearnSpell(ONE_H_MACES, false);
			player->LearnSpell(ONE_H_AXES, false);
			player->LearnSpell(FIST_WEAPONS, false);
			player->LearnSpell(DAGGERS, false);
			player->LearnSpell(BLOCK, false);
			break;
		case CLASS_HUNTER:
			player->LearnSpell(THROW_WAR, false);
			player->LearnSpell(TWO_H_SWORDS, false);
			player->LearnSpell(TWO_H_AXES, false);
			player->LearnSpell(STAVES, false);
			player->LearnSpell(POLEARMS, false);
			player->LearnSpell(ONE_H_SWORDS, false);
			player->LearnSpell(ONE_H_AXES, false);
			player->LearnSpell(GUNS, false);
			player->LearnSpell(FIST_WEAPONS, false);
			player->LearnSpell(DAGGERS, false);
			player->LearnSpell(CROSSBOWS, false);
			player->LearnSpell(BOWS, false);
			break;
		case CLASS_DRUID:
			player->LearnSpell(TWO_H_MACES, false);
			player->LearnSpell(STAVES, false);
			player->LearnSpell(POLEARMS, false);
			player->LearnSpell(ONE_H_MACES, false);
			player->LearnSpell(FIST_WEAPONS, false);
			player->LearnSpell(DAGGERS, false);
			break;
		case CLASS_WARLOCK:
			player->LearnSpell(WANDS, false);
			player->LearnSpell(STAVES, false);
			player->LearnSpell(SHOOT, false);
			player->LearnSpell(ONE_H_SWORDS, false);
			player->LearnSpell(DAGGERS, false);
			break;
		default:
			break;
		}
	}

	static void LearnPlateMailSpells(Player* player)
	{
		switch (player->getClass())
		{
		case CLASS_WARRIOR:
		case CLASS_PALADIN:
		case CLASS_DEATH_KNIGHT:
			player->LearnSpell(PLATE_MAIL, true);
			break;
		case CLASS_SHAMAN:
		case CLASS_HUNTER:
			player->LearnSpell(ARMOR_MAIL, true);
			break;
		default:
			break;
		}
	}
};

void AddSC_player_custom_commands()
{
    new CopyGearCommands();
}