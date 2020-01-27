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
#include "Player.h"
#include "Language.h"

class PlayerTimeReward : public PlayerScript
{
public:
	PlayerTimeReward() : PlayerScript("PlayerTimeReward") { }

	// extract items for mail
	typedef std::pair<uint32, uint32> ItemPair;
	typedef std::list< ItemPair > ItemPairs;
	ItemPairs mailItems;

	void SendMail(Player* player, std::string subject, std::string text, uint32 money = 0)
	{
		if (mailItems.size() > MAX_MAIL_ITEMS)
			return;

		// from console show not existed sender
		MailSender sender(MAIL_NORMAL, 0, MAIL_STATIONERY_GM);

		// fill mail
		MailDraft draft(subject, text);

		SQLTransaction trans = CharacterDatabase.BeginTransaction();

		for (ItemPairs::const_iterator itr = mailItems.begin(); itr != mailItems.end(); ++itr)
        {
            if (Item* item = Item::CreateItem(itr->first, itr->second))
            {
                item->SaveToDB(trans);                               // save for prevent lost at next mail load, if send fail then item will deleted
                draft.AddItem(item);
            }
        }

		if (money != 0)
			draft.AddMoney(money);
		draft.SendMailTo(trans, MailReceiver(player, player->GetGUID()), sender);
		CharacterDatabase.CommitTransaction(trans);
		mailItems.clear();
	}
	
	void OnTime(Player* player, uint32 total, uint32 /*level*/)
	{
		if (!sWorld->getBoolConfig(CONFIG_AWARDS_PER_TIME))
			return;
			
		switch (total)
		{
			case 3600: // 1 hours
				player->AddItem(sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_1_HOURS), (sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_1_HOURS_COUNT)));
				ChatHandler(player->GetSession()).SendSysMessage(LANG_PLAYER_TIME_REWARD);
				break;		
			case 10800: // 3 hours
				player->AddItem(sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_3_HOURS), (sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_3_HOURS_COUNT)));
				ChatHandler(player->GetSession()).SendSysMessage(LANG_PLAYER_TIME_REWARD);
				break;
			case 21600: // 6 hours
				player->AddItem(sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_6_HOURS), (sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_6_HOURS_COUNT))); 
				ChatHandler(player->GetSession()).SendSysMessage(LANG_PLAYER_TIME_REWARD);
				break;
			case 43200: // 12 hours
				player->AddItem(sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_12_HOURS), (sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_12_HOURS_COUNT)));
				ChatHandler(player->GetSession()).SendSysMessage(LANG_PLAYER_TIME_REWARD);
				break;
			case 86400: // 1 day
				player->AddItem(sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_1_DAY), (sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_1_DAY_COUNT)));
				ChatHandler(player->GetSession()).SendSysMessage(LANG_PLAYER_TIME_REWARD);
				break;				
			case 172800: // 2 day
				player->AddItem(sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_2_DAY), (sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_2_DAY_COUNT)));
				ChatHandler(player->GetSession()).SendSysMessage(LANG_PLAYER_TIME_REWARD);
				break;				
			case 345600: // 4 day
				player->AddItem(sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_4_DAY), (sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_4_DAY_COUNT)));
				ChatHandler(player->GetSession()).SendSysMessage(LANG_PLAYER_TIME_REWARD);
				break;				
			case 691200: // 8 day
				player->AddItem(sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_8_DAY), (sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_8_DAY_COUNT)));
				ChatHandler(player->GetSession()).SendSysMessage(LANG_PLAYER_TIME_REWARD);
				break;				
			case 1382400: // 16 day
				player->AddItem(sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_16_DAY), (sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_16_DAY_COUNT)));
				ChatHandler(player->GetSession()).SendSysMessage(LANG_PLAYER_TIME_REWARD);
				break;				
			case 2592000: // 30 day
				player->AddItem(sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_30_DAY), (sWorld->getIntConfig(CONFIG_AWARDS_PER_TIME_30_DAY_COUNT)));
				ChatHandler(player->GetSession()).SendSysMessage(LANG_PLAYER_TIME_REWARD);			
/*				
			case 115200: // 32 hours
				mailItems.push_back(ItemPair(5133, 1));
				mailItems.push_back(ItemPair(5133, 3));
				mailItems.push_back(ItemPair(5133, 5));
				SendMail(player, "Заголовок письма", "Содержание письма", 5000000); // Send Items +500 gold
				ChatHandler(player->GetSession()).SendSysMessage(LANG_PLAYER_TIME_REWARD);
*/
				break;
			default:
				break;
		}
	}
};


void AddSC_player_time_reward() 
{		
  new PlayerTimeReward();
}