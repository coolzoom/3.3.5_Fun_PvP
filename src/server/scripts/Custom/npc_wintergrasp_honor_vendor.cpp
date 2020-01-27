#include "ScriptPCH.h"
#include "SpellAuraEffects.h"
#include "ScriptMgr.h"
#include "Config.h"
#include "Language.h"

class npc_wintergrasp_honor_vendor : public CreatureScript
{
public:
    npc_wintergrasp_honor_vendor() : CreatureScript("npc_wintergrasp_honor_vendor") { }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature) override
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, "Я хочу обменять очки чести на Почётные знаки Озера Ледяных Оков", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

        pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction) override
    {
        pPlayer->PlayerTalkClass->ClearMenus();
        if (uiAction == GOSSIP_ACTION_TRADE)
            pPlayer->GetSession()->SendListInventory(pCreature->GetGUID());

        Map::PlayerList const &playerList = pCreature->GetMap()->GetPlayers();
        if (!playerList.isEmpty())
            for (Map::PlayerList::const_iterator i = playerList.begin(); i != playerList.end(); ++i)
                if (Player *iPlayer = i->GetSource())
                {
                    if (!iPlayer->IsGameMaster() && iPlayer->GetDistance(pCreature) <= 100.0f)
                        iPlayer->SetPvP(true);
                }
        return true;
    }
};

/*######
## npc_flyhack_banner
## Special for Verybad xD
######*/
class npc_flyhack_banner : public CreatureScript
{
public:
    npc_flyhack_banner() : CreatureScript("npc_flyhack_banner") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_flyhack_bannerAI(pCreature);
    }

    struct npc_flyhack_bannerAI : public ScriptedAI
    {
        npc_flyhack_bannerAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            Reset();
        }

        void Reset() override
        {
        }

        void MoveInLineOfSight(Unit* pWho) override
        {
            if (!pWho || pWho->GetTypeId() != TYPEID_PLAYER) return;

            if (pWho->IsWithinDistInMap(me, 60.0f) && pWho->ToPlayer()->GetSession()->GetSecurity() < SEC_GAMEMASTER)
            {
                if (pWho->HasAuraType(SPELL_AURA_FLY) || pWho->HasAuraType(SPELL_AURA_WATER_WALK)
                || pWho->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) || pWho->HasAuraType(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED))
                     return;

                std::string sText = ("Игрок: " + std::string(pWho->GetName()) + " получил премию Дарвина.");

                sWorld->SendGMText(LANG_GM_BROADCAST, sText.c_str());

                //if (SpellEntry const *spellInfo = sSpellStore.LookupEntry(9454))
                  //  Aura::TryCreate(spellInfo, pWho, pWho);

                //sWorld->BanAccount(BAN_CHARACTER, pWho->GetName(), "0", sText.c_str(), "Very autobanner");
                Player* player = pWho->ToPlayer();
                player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, player->GetOrientation());

            }

        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            //DoMeleeAttackIfReady();
        }

    };
};

void AddSC_npc_wintergrasp_honor_vendor()
{
    new npc_wintergrasp_honor_vendor;
    new npc_flyhack_banner;
}
