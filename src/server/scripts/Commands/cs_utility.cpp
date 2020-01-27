#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Chat.h"
#include "Group.h"
#include "ArenaTeam.h"
#include "ArenaTeamMgr.h"

//This script is made by Blex and was orginally posted on www.ac-web.org
//This script was created after being inspired by Arena-Tournament's player commands. www.arena-tournament.com

class utility_commandscript : public CommandScript
{
public:
    utility_commandscript() : CommandScript("utility_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> utilityCommandTable =
        {
            { "changerace",             SEC_PLAYER,  false, &HandleChangeRaceCommand,           "" },
			{ "changefaction",			SEC_PLAYER,  false, &HandleChangeFactionCommand,		"" },
			{ "maxskills",			    SEC_PLAYER,  false, &HandleMaxSkillsCommand,	    	"" },
			{ "customize",			    SEC_PLAYER,  false, &HandleCustomizeCommand,	       	"" },
			{ "mmr",			        SEC_PLAYER,  false, &HandleMMRCommand,      	       	"" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "utility",           SEC_PLAYER,      true, NULL,                   "", utilityCommandTable },
        };
        return commandTable;
    }

    static bool HandleChangeRaceCommand(ChatHandler* handler, char const*)
    {

        Player* me = handler->GetSession()->GetPlayer();
		me->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
		handler->PSendSysMessage("Relog to change race of your character.");
        return true;
    }

    static bool HandleChangeFactionCommand(ChatHandler* handler, char const*)
    {

        Player* me = handler->GetSession()->GetPlayer();
		
        me->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
		handler->PSendSysMessage("Relog to change faction of your character.");
        return true;
    }

    static bool HandleMaxSkillsCommand(ChatHandler* handler, char const*)
    {

        Player* me = handler->GetSession()->GetPlayer();
		me->UpdateSkillsForLevel();
		handler->PSendSysMessage("Your weapon skills are now maximized.");
        return true;
    }

	static bool HandleCustomizeCommand(ChatHandler* handler, char const*)
    {

        Player* me = handler->GetSession()->GetPlayer();
		me->SetAtLoginFlag(AT_LOGIN_CUSTOMIZE);
		handler->PSendSysMessage("Relog to customize your character.");
        return true;
    }

	 static bool HandleMMRCommand(ChatHandler* handler, char const*)
     {
        Player* player = handler->GetSession()->GetPlayer();

        // 2s MMR check
        uint16 mmrTwo = 0;
        {
			if (ArenaTeam *getmmr = sArenaTeamMgr->GetArenaTeamById(player->GetArenaTeamId(0)))
			    mmrTwo = getmmr->GetMember(player->GetGUID())->MatchMakerRating;
		     else if (!mmrTwo)
			    mmrTwo = 1500;
        }
		uint16 mmrThree = 0;
        {
			if (ArenaTeam *getmmr = sArenaTeamMgr->GetArenaTeamById(player->GetArenaTeamId(1)))
			    mmrThree = getmmr->GetMember(player->GetGUID())->MatchMakerRating;
		     else if (!mmrThree)
			    mmrThree = 1500;
        }
		uint16 mmrOne = 0;
        {
			if (ArenaTeam *getmmr = sArenaTeamMgr->GetArenaTeamById(player->GetArenaTeamId(2)))
			    mmrOne = getmmr->GetMember(player->GetGUID())->MatchMakerRating;
		     else if (!mmrOne)
			    mmrOne = 1000;
        }
	    handler->PSendSysMessage("Your current mmr in 2v2 arena match is: %u.", mmrTwo);
	    handler->PSendSysMessage("Your current mmr in 3v3 arena match is: %u.", mmrThree);
	    handler->PSendSysMessage("Your current mmr in Solo Queue 3v3 arena match is: %u.", mmrOne);
		return true;
      }
};

void AddSC_utility_commandscript()
{
    new utility_commandscript();
}

