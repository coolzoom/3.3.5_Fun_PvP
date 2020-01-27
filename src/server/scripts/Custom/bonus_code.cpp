/*
Patch: Bonus Code
Date: 09.02.2018
Update: fixed & update 19.02.2018
By: r0m1ntik
*/

/*  UPDATE - Добавить в базу characters(update v2)

DROP TABLE IF EXISTS `bonus_code`;
CREATE TABLE `bonus_code` (
  `code` int(10) unsigned NOT NULL DEFAULT '0',
  `item1` int(10) unsigned NOT NULL DEFAULT '0',
  `count1` int(10) unsigned NOT NULL DEFAULT '0',
  `item2` int(10) unsigned NOT NULL DEFAULT '0',
  `count2` int(10) unsigned NOT NULL DEFAULT '0',
  `item3` int(10) unsigned NOT NULL DEFAULT '0',
  `count3` int(10) unsigned NOT NULL DEFAULT '0',
  `honor` int(10) unsigned NOT NULL DEFAULT '0',
  `arena` int(10) unsigned NOT NULL DEFAULT '0',  
  PRIMARY KEY (`code`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

*/

#include "ScriptMgr.h"
#include "Chat.h"

using namespace std;

#define vk    "https://vk.com/x_wowfun"
#define error "|cffFFFFFFК сожалению вы ввели неверный код, убедитесь в том что вы не ошиблись.|r"

class npc_bonus_code : public CreatureScript
{
public:
    npc_bonus_code() : CreatureScript("npc_bonus_code") { }
	
    bool OnGossipHello(Player* player, Creature * creature)
    {
        	std::string name = player->GetName();
			std::ostringstream femb;
		    femb << "Уважаемый|cffB404AE " << name << "|r\n";
			femb << "Посещяйте чаще нашу группу ВК, с помощью промо кодов вы сможете получить:\n\n";
		    femb << "* До 3-их предметов:\n";
		    femb << "* Очки арены, Очки чести и бонусы на ваш аккаунт\n";
		    femb << "\nНаша группа: |cffB404AE " << vk << " |r\n";

		player->PlayerTalkClass->ClearMenus();
        player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_BATTLE, "|TInterface\\icons\\Inv_sigil_freya:25:25:-19:0|tВвести код", GOSSIP_SENDER_MAIN, 1, "Приветствую Вас !\n\n|cffccff66    Введите код приза|r\n\nКод можно узнать в нашей группе ВК:\nhttps://vk.com/wow_idk\n\nСледите за новостями!", 0, true);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25:25:-19:0|tОбновить", GOSSIP_SENDER_MAIN, 2);
        player->PlayerTalkClass->SendGossipMenu(femb.str().c_str(), creature->GetGUID());
		return true;
    }

	bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 uiAction, const char* code)
    {
		switch(uiAction)
		{
                case 1:
				BonusCode(player, creature, code); 
                break;                            
		}
        return true;
	}
	
	bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
	{
		player->PlayerTalkClass->ClearMenus();

			switch (action)
			{
				case 2:
				OnGossipHello(player, creature);
			    break;																				
			}
            return true;
	}    

    bool BonusCode(Player* player, Creature* creature, char const* code)
	{
        QueryResult result = CharacterDatabase.PQuery("SELECT code, item1, count1, item2, count2, item3, count3, honor, arena, coin FROM `bonus_code`");
        if (result)
        {
            uint32 player_code = atoi(code); // Код который игрок вводит
            Field *fields      = result->Fetch(); // вывод с базы что нам нужно
			uint32 code        = fields[0].GetUInt32(); // Код который в бд
            uint32 item1       = fields[1].GetUInt32(); // первый предмет
            uint32 count1      = fields[2].GetUInt32(); // количесво первого предмета
            uint32 item2       = fields[3].GetUInt32(); // второй предмет
            uint32 count2      = fields[4].GetUInt32(); // количесво второго предмета
            uint32 item3       = fields[5].GetUInt32();  // третий предмет
            uint32 count3      = fields[6].GetUInt32(); // количесво третьего предмета
            uint32 honor       = fields[7].GetUInt32(); // количесво хонора
            uint32 arena       = fields[8].GetUInt32(); // количесво арена
            uint32 coin        = fields[9].GetUInt32(); // количесво бонусов
            std::string name   = player->GetName(); // выводим ник

            if (code == player_code) // Если код который в базе совпадает с введеным игроком кодам
            {
                if (item1 > 0 && count1 > 0)
                {
                    player->AddItem(item1, count1); // первый предмет(не обязательно)
                }
                if (item2 > 0 && count2 > 0)
                {
                    player->AddItem(item2, count2); // второй предмет(не обязательно)
                }
                if (item3 > 0 && count3 > 0)
                {
                    player->AddItem(item3, count3); // третий предмет(не обязательно)
                }
                player->ModifyHonorPoints(+honor); // даем хонор(не обязательно)
                player->ModifyArenaPoints(+arena); // даем арену(не обязательно)

                if (coin > 0)
                {
                    LoginDatabase.PQuery("UPDATE account SET coin = coin+%u WHERE id = %u", coin, player->GetSession()->GetAccountId());
                }

                CharacterDatabase.PQuery("DELETE FROM bonus_code WHERE code = %u", code); // после угада кода удаляем с базы

                std::ostringstream msg;
                msg << "|cffffffff[|cffB404AE " << name << "|r|cffffffff ] ввел код [|cffB404AE " << code << "|r|cffffffff ] и забрал свою награду!\n|cffffffffХочешь быть следующим победителем?\n|cffffffffСледи за новостями в группе ВК|cffB404AE " << vk << "|r";
                sWorld->SendServerMessage(SERVER_MSG_STRING, msg.str().c_str());
                player->PlayerTalkClass->SendCloseGossip();                
            }
            else
            {
                ChatHandler(player->GetSession()).PSendSysMessage(error);
                player->PlayerTalkClass->SendCloseGossip(); 
            }
        }
        else // Если нету кодов в базе
        {
            ChatHandler(player->GetSession()).PSendSysMessage("|cffFFFFFFК сожалению код уже использовали, следите за новостями в нашей группе Вк\n%s|r", vk);
            player->PlayerTalkClass->SendCloseGossip();
            return true; 
        }
        return true;
    }     

}; 

void AddSC_npc_bonus_code()
{
	new npc_bonus_code();
}   