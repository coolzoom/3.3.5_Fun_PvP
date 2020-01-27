#include "ScriptMgr.h"
#include "Chat.h"

class npc_title_exchange : public CreatureScript
{
public:
    npc_title_exchange() : CreatureScript("npc_title_exchange") { }

    bool OnGossipHello(Player* player, Creature* creature)
	{
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tПосланник - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 130); // ActionId == Title ID
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tСеребрянный Заступник - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 171);
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tСеребрянный Защитник - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 131);
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tИз пепельного союза - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 176); // ActionId == Title ID
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tЗвездный странник - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 165);
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tПобедивший павшего Короля - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 174);
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tХмелевар - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 133); // ActionId == Title ID
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tНебесный Заступник - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 159);
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tЗавоеватель Наскрамаса - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 122);
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tКрестоносец - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 156); // ActionId == Title ID
		//player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|t, Death's Demise - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 158);
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tДипломат - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 79);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tПремудрый - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 74); // ActionId == Title ID
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tthe Exalted - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 77);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tПремудрый - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 146);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tХранитель Огня - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 76); // ActionId == Title ID
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\spell_chargepositive:30:30:-18:0|tСледующая Страница...."                                      , GOSSIP_SENDER_MAIN, 1001);
		player->PlayerTalkClass->SendGossipMenu(85000, creature->GetGUID());
		return true;
	}

	bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 title) override
    {
		if (!player || !creature || !title) // Prevent Hack
            return false;
            
        if (title == 1001)
        {
             player->PlayerTalkClass->ClearMenus();
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tСтраж Огня - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 75);
            //player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tthe Flawless Victor - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 128);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tЗащитник Кенария - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 132); // ActionId == Title ID
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tТыквер - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 124);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tПосланник Титанов - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 166);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tШеф-повар - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 84); // ActionId == Title ID
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tЧокнутый - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 145);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tСвергнувший Короля - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 175); // ActionId == Title ID
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tХранитель Мудрости - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 125);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tВесельчак - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 135);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tИскатель магии - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 120); // ActionId == Title ID
            //player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tMerrymaker - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 134);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tСумеречный - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 140);
           // player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tObsidian Slayer - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 139);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tМорской Дьявол - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 83);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tГоворящий со звездами - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 164); // ActionId == Title ID
            //player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tthe Undying- 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 142);
            //player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "|TInterface\\icons\\Inv_qiraj_jewelengraved:30:30:-18:0|tVanquisher - 65 Хиджал Поинт"                  , GOSSIP_SENDER_MAIN, 163);
            player->PlayerTalkClass->SendGossipMenu(85000, creature->GetGUID());
            return true;
        }
        player->PlayerTalkClass->SendCloseGossip();

		const int32 TokenId = 43514;
            
		/*switch (title)
		{
            default:
                Cost = 65; // Default Cost for all titles thats not defined in this switch
                break;
        }*/
        
        if (player->HasItemCount(TokenId, 65))
		{ 
			player->DestroyItemCount(TokenId, 65, true);
        
			if (CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(title))
				player->SetTitle(titleEntry);

			ChatHandler(player->GetSession()).PSendSysMessage("Звание успешно выдано!");
			return true;
		}
		//ChatHandler(player->GetSession()).PSendSysMessage("Not have the required number of |Hitem:28558:0:0:0:0:0:0:0:0|h|cffff8000[Хиджал Поинт]|r|h!");

		return false;
	}
};

void AddSC_npc_title_exchange()
{
	new npc_title_exchange();
}