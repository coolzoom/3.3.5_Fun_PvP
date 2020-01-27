#include "ScriptMgr.h"
#include "Player.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include <sstream>

class FastArenaCrystal : public GameObjectScript
{
    public:

        FastArenaCrystal()
            : GameObjectScript("FastArenaCrystal")
        {
        }

        bool OnGossipHello(Player* player, GameObject* go)
        {
            player->m_clicked = true;

            // Don't let spectators to use arena crystal
            if (player->IsSpectator())
            {
                player->GetSession()->SendAreaTriggerMessage("You're not be able to do this while spectating.");
                return false;
            }

            if (Battleground *bg = player->GetBattleground())
                if (bg->isArena())
				{
                    uint8 ReadyPlayers = 0;

                    for (Battleground::BattlegroundPlayerMap::const_iterator itr = bg->GetPlayers().begin(); itr != bg->GetPlayers().end(); ++itr)
                        if (Player *plr = ObjectAccessor::FindPlayer(itr->first))
                        {
                            if (plr->m_clicked == true)
                                ++ReadyPlayers;
                        }
                    
                    ChatHandler(player->GetSession()).PSendSysMessage("You clicked on arena crystal, prepare yourself arena can start earlier");
                    if (ReadyPlayers > 1)
                        ChatHandler(player->GetSession()).PSendSysMessage("Ready players: %u.", ReadyPlayers);
                    else
                        ChatHandler(player->GetSession()).PSendSysMessage("Ready player: %u.", ReadyPlayers);

                    bg->ClickFastStart(player, go);
				}  
            return false;
        }
};

void AddSC_fast_arena_start()
{
    new FastArenaCrystal();
}
