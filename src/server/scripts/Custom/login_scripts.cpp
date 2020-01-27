#include "ScriptMgr.h"
#include "TicketMgr.h"
#include "Config.h"
#include "AccountMgr.h"
#include "AchievementMgr.h"

#define SPELL_MASK_CLASS        1
#define SPELL_MASK_RIDING       2
#define SPELL_MASK_MOUNT        4
#define SPELL_MASK_WEAPON       8
#define SPELL_MASK_PROFESSION   16
#define SPELL_MASK_DUAL_SPEC    32

class login_script : public PlayerScript
{
public:
	login_script() : PlayerScript("login_script") { }
	void OnLogin(Player* player)
	{
                //player->RemoveAurasDueToSpell(29659);
				//player->LearnSpell(63706, player);
				//player->LearnSpell(63707, player);
                      // AchievementEntry const* achievementEntry = sAchievementMgr->GetAchievement(2398);
                       //player->CompletedAchievement(achievementEntry);
    }
};

// Dont let new players to use chat. This is in case that new players are going to advertise
class system_censure : public PlayerScript
{
public:
	system_censure() : PlayerScript("system_censure") {}
	void OnChat(Player* player, uint32 /*type*/, uint32 lang, std::string& msg, Player* receiver) // Whisper
	{
		// Whispers only to GM is available
		if (receiver->GetSession()->GetSecurity() >= 1)
			return;

		CheckMessage(player, msg, lang, receiver, NULL, NULL, NULL);
	}

	void OnChat(Player* player, uint32 /*type*/, uint32 lang, std::string& msg, Channel* channel) // LFG and channels
	{
		CheckMessage(player, msg, lang, NULL, NULL, NULL, channel);
	}

	void CheckMessage(Player* player, std::string& msg, uint32 lang, Player* /*receiver*/, Group* /*group*/, Guild* /*guild*/, Channel* channel)
	{
		// VIPs can use the chat too, also lang addon
		if (player->GetSession()->GetSecurity() >= 1 || lang == LANG_ADDON)
			return;

		uint32 reqPlayedTime = 10 * MINUTE; // 10 minutes

		if (player->GetTotalPlayedTime() <= reqPlayedTime)
		{
			uint32 remainingTime = ((10 * MINUTE) - player->GetTotalPlayedTime()) / MINUTE;
			uint32 currentTime = 10 - remainingTime;

			msg = "";

			ChatHandler(player->GetSession()).PSendSysMessage("Для использования глобальных каналов или лс требуется не менее 10 минут игрового времени. Обратите внимание, что вы все еще можете обратится к GM.");
			player->GetSession()->SendAreaTriggerMessage("Для использования глобальных каналов или лс требуется не менее 10 минут игрового времени.");
			ChatHandler(player->GetSession()).PSendSysMessage("Ваше игровое время: %u минут.", currentTime);
			return;
		}
	}
};


void AddSC_gm_login()
{
	new system_censure();
}