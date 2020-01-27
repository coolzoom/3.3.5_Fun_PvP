#include "Player.h"
#include "Chat.h"
#define Welcome_Name "WoW-IDK"

class Player_Boa : public PlayerScript
{

public:
    Player_Boa() : PlayerScript("Player_Boa") { }

    void OnLogin(Player* player, bool firstLogin) override
    {
        if (!firstLogin) // Run script only on first login
            return;
        std::ostringstream ss;
        ss << "|cffFF0000["<<Welcome_Name<<"]|r Приветствуем " << player->GetName() << " на нашем сервере! ";
        sWorld->SendServerMessage(SERVER_MSG_STRING, ss.str().c_str());
    }
};

void AddSC_Player_Boa()
{
    new Player_Boa();
}