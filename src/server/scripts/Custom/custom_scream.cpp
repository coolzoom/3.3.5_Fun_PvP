#include "ScriptMgr.h"
#include "Common.h"
#include "Chat.h"
#include "Player.h"
#include "World.h"
#include "Config.h"
#include "WorldSession.h"
#include "Language.h"
#include "Log.h"
#include "SpellAuras.h"
#include "SpellHistory.h"
#include "Language.h"
#include "Pet.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "WorldPacket.h"
#include "ObjectMgr.h"
#include "ArenaTeam.h"
#include "ArenaTeamMgr.h"
#include "ObjectAccessor.h"
#include "Unit.h"
#include "AchievementMgr.h"
#include "player_custom_npc.h"
#include "Language.h"

#define GOSSIP_CLASS_TRAIN  "|TInterface\\icons\\achievement_level_80:25:25:-18:0|tКлассовые навыки|r"
#define GOSSIP_DUAL_TALENT_SPECIALIZATION  "|TInterface\\icons\\achievement_general:25:25:-18:0|tДвойная специализация|r"
#define GOSSIP_RESET_TALENTS  "|TInterface\\icons\\ability_marksmanship:25:25:-18:0|tСброс талантов|r"
#define GOSSIP_PROFESSION_TRAIN "|TInterface\\icons\\Ability_Repair:25:25:-18:0|tПрофессии|r"
#define GOSSIP_RIDING "|TInterface\\icons\\spell_nature_swiftness:25:25:-18:0|tВерховая езда|r"
#define GOSSIP_WEAPON "|TInterface\\icons\\Ability_DualWield:25:25:-18:0|tОружейные навыки|r"
#define GOSSIP_MAX_SKILLS "|TInterface\\icons\\trade_engineering:25:25:-18:0|tУлучшить навыки защиты и владения оружием до максимума|r"
#define GOSSIP_COMBAT_STOP "|TInterface\\icons\\Achievement_BG_AB_defendflags:25:25:-18:0|t Снять бой|r"
#define GOSSIP_RESET_YES    "|cff00ff00Да (Сбросить таланты)|r"
#define GOSSIP_RESET_NO    "|cffff0000Нет (Закрыть окно)|r"

class ultimate_trainer_npc : public CreatureScript
{
public:
    ultimate_trainer_npc() : CreatureScript("ultimate_trainer_npc") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new ultimate_trainer_npcAI(creature);
    }

    struct ultimate_trainer_npcAI : public ScriptedAI
    {
        ultimate_trainer_npcAI(Creature* creature) : ScriptedAI(creature)
        {
            me->CastSpell(me, 43897, true);
        }
    };

    bool OnGossipHello(Player* player, Creature* creature)
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_CLASS_TRAIN, GOSSIP_SENDER_MAIN, 1);
        player->ADD_GOSSIP_ITEM(4, GOSSIP_DUAL_TALENT_SPECIALIZATION, GOSSIP_SENDER_MAIN, 2);
        player->ADD_GOSSIP_ITEM(4, GOSSIP_RESET_TALENTS, GOSSIP_SENDER_MAIN, 3);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_PROFESSION_TRAIN, GOSSIP_SENDER_MAIN, 4);
        player->ADD_GOSSIP_ITEM(4, GOSSIP_RIDING, GOSSIP_SENDER_MAIN, 5);
        player->ADD_GOSSIP_ITEM(4, GOSSIP_WEAPON, GOSSIP_SENDER_MAIN, 6);
        player->ADD_GOSSIP_ITEM(4, GOSSIP_MAX_SKILLS, GOSSIP_SENDER_MAIN, 7);
        player->ADD_GOSSIP_ITEM(4, GOSSIP_COMBAT_STOP, GOSSIP_SENDER_MAIN, 8);
        
        player->SEND_GOSSIP_MENU(68, creature->GetGUID());
        return true;
    }

    bool PlayerAlreadyHasTwoProfessions(const Player *pPlayer) const
    {
        uint32 skillCount = 0;
        
        if (pPlayer->HasSkill(SKILL_MINING))
        skillCount++;
        if (pPlayer->HasSkill(SKILL_SKINNING))
        skillCount++;
        if (pPlayer->HasSkill(SKILL_HERBALISM))
        skillCount++;
        if (skillCount >= 2)
        return true;
        
        for (uint32 i = 1; i < sSkillLineStore.GetNumRows(); ++i)
        {
            SkillLineEntry const *SkillInfo = sSkillLineStore.LookupEntry(i);
            if (!SkillInfo)
            continue;
            
            if (SkillInfo->categoryId == SKILL_CATEGORY_SECONDARY)
            continue;
            
            if ((SkillInfo->categoryId != SKILL_CATEGORY_PROFESSION) || !SkillInfo->canLink)
            continue;
            
            const uint32 skillID = SkillInfo->id;
            if (pPlayer->HasSkill(skillID))
            skillCount++;
            
            if (skillCount >= 4)
            return true;
        }
        return false;
    }
    
    bool LearnAllRecipesInProfession(Player *pPlayer, SkillType skill)
    {
        ChatHandler handler(pPlayer->GetSession());
        char* skill_name;
        
        SkillLineEntry const *SkillInfo = sSkillLineStore.LookupEntry(skill);
        skill_name = SkillInfo->name[handler.GetSessionDbcLocale()];
        
        if (!SkillInfo)
        {
            //TC_LOG_ERROR("server.loading", "Profession NPC: received non-valid skill ID (LearnAllRecipesInProfession)");
        }      
        
        LearnSkillRecipesHelper(pPlayer, SkillInfo->id);
        
        pPlayer->SetSkill(SkillInfo->id, pPlayer->GetSkillStep(SkillInfo->id), 450, 450);
        handler.PSendSysMessage(LANG_COMMAND_LEARN_ALL_RECIPES, skill_name);
        return true;
    }
    
    void LearnSkillRecipesHelper(Player *player, uint32 skill_id)
    {
        uint32 classmask = player->getClassMask();
        for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
        {
            SkillLineAbilityEntry const *skillLine = sSkillLineAbilityStore.LookupEntry(j);
            if (!skillLine)
            continue;
            
            // wrong skill
            if (skillLine->skillId != skill_id)
            continue;
            
            // not high rank
            if (skillLine->forward_spellid)
            continue;
            
            // skip racial skills
            if (skillLine->racemask != 0)
            continue;
            
            // skip wrong class skills
            if (skillLine->classmask && (skillLine->classmask & classmask) == 0)
            continue;
            
            SpellInfo const * spellInfo = sSpellMgr->GetSpellInfo(skillLine->spellId);
            if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, player, false))
            continue;
            
            player->LearnSpell(skillLine->spellId, false);
        }
    }
    
    bool IsSecondarySkill(SkillType skill) const
    {
        return skill == SKILL_COOKING || skill == SKILL_FIRST_AID;
    }
    
    void CompleteLearnProfession(Player *pPlayer, Creature *pCreature, SkillType skill)
    {
        if (PlayerAlreadyHasTwoProfessions(pPlayer) && !IsSecondarySkill(skill));
        else
        {
            if (!LearnAllRecipesInProfession(pPlayer, skill));
        }
    }
    
    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();

        if (sender == GOSSIP_SENDER_MAIN)
        {
            switch (action)
            {
            case 1: // Классовые навыки

                switch (player->getClass())
                {
                case CLASS_WARRIOR:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(7384, false);
                    player->LearnSpell(47436, false);
                    player->LearnSpell(47450, false);
                    player->LearnSpell(11578, false);
                    player->LearnSpell(47465, false);
                    player->LearnSpell(47502, false);
                    player->LearnSpell(34428, false);
                    player->LearnSpell(1715, false);
                    player->LearnSpell(2687, false);
                    player->LearnSpell(71, false);
                    player->LearnSpell(7386, false);
                    player->LearnSpell(355, false);
                    player->LearnSpell(72, false);
                    player->LearnSpell(47437, false);
                    player->LearnSpell(57823, false);
                    player->LearnSpell(694, false);
                    player->LearnSpell(2565, false);
                    player->LearnSpell(676, false);
                    player->LearnSpell(47520, false);
                    player->LearnSpell(20230, false);
                    player->LearnSpell(12678, false);
                    player->LearnSpell(47471, false);
                    player->LearnSpell(1161, false);
                    player->LearnSpell(871, false);
                    player->LearnSpell(2458, false);
                    player->LearnSpell(20252, false);
                    player->LearnSpell(47475, false);
                    player->LearnSpell(18499, false);
                    player->LearnSpell(1680, false);
                    player->LearnSpell(6552, false);
                    player->LearnSpell(47488, false);
                    player->LearnSpell(1719, false);
                    player->LearnSpell(23920, false);
                    player->LearnSpell(47440, false);
                    player->LearnSpell(3411, false);
                    player->LearnSpell(64382, false);
                    player->LearnSpell(55694, false);
                    player->LearnSpell(57755, false);
                    player->LearnSpell(42459, false);
                    player->LearnSpell(750, false);
                    player->LearnSpell(5246, false);
                    player->LearnSpell(3127, false);
                    if (player->HasSpell(12294))
                    player->LearnSpell(47486, false);
                    if (player->HasSpell(20243))
                    player->LearnSpell(47498, false);
                    player->SaveToDB();
                    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_PALADIN:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(3127, false);
                    player->LearnSpell(19746, false);
                    player->LearnSpell(19752, false);
                    player->LearnSpell(750, false);
                    player->LearnSpell(48942, false);
                    player->LearnSpell(48782, false);
                    player->LearnSpell(48932, false);
                    player->LearnSpell(20271, false);
                    player->LearnSpell(498, false);
                    player->LearnSpell(10308, false);
                    player->LearnSpell(1152, false);
                    player->LearnSpell(10278, false);
                    player->LearnSpell(48788, false);
                    player->LearnSpell(53408, false);
                    player->LearnSpell(48950, false);
                    player->LearnSpell(48936, false);
                    player->LearnSpell(31789, false);
                    player->LearnSpell(62124, false);
                    player->LearnSpell(54043, false);
                    player->LearnSpell(25780, false);
                    player->LearnSpell(1044, false);
                    player->LearnSpell(20217, false);
                    player->LearnSpell(48819, false);
                    player->LearnSpell(48801, false);
                    player->LearnSpell(48785, false);
                    player->LearnSpell(5502, false);
                    player->LearnSpell(20164, false);
                    player->LearnSpell(10326, false);
                    player->LearnSpell(1038, false);
                    player->LearnSpell(53407, false);
                    player->LearnSpell(48943, false);
                    player->LearnSpell(20165, false);
                    player->LearnSpell(48945, false);
                    player->LearnSpell(642, false);
                    player->LearnSpell(48947, false);
                    player->LearnSpell(20166, false);
                    player->LearnSpell(4987, false);
                    player->LearnSpell(48806, false);
                    player->LearnSpell(6940, false);
                    player->LearnSpell(48817, false);
                    player->LearnSpell(48934, false);
                    player->LearnSpell(48938, false);
                    player->LearnSpell(25898, false);
                    player->LearnSpell(32223, false);
                    player->LearnSpell(31884, false);
                    player->LearnSpell(54428, false);
                    player->LearnSpell(61411, false);
                    player->LearnSpell(53601, false);
                    player->LearnSpell(33388, false);
                    player->LearnSpell(33391, false);
                    if (player->HasSpell(20925))
                    player->LearnSpell(48952, false);
                    if (player->HasSpell(31935))
                    player->LearnSpell(48827, false);
                    if (player->HasSpell(20911))
                    player->LearnSpell(25899, false);
                    if (player->HasSpell(20473))
                    player->LearnSpell(48825, false);
                    if (player->GetTeam() == ALLIANCE)
                    {
                        player->LearnSpell(31801, false);
                        player->LearnSpell(13819, false);
                        player->LearnSpell(23214, false);
                    }
                    if (player->GetTeam() == HORDE)
                    {
                        player->LearnSpell(53736, false);
                        player->LearnSpell(34769, false);
                        player->LearnSpell(34767, false);
                    }
                    player->SaveToDB();
                    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_HUNTER:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(3043, false);
                    player->LearnSpell(3127, false);
                    player->LearnSpell(3045, false);
                    player->LearnSpell(3034, false);
                    player->LearnSpell(8737, false);
                    player->LearnSpell(1494, false);
                    player->LearnSpell(13163, false);
                    player->LearnSpell(48996, false);
                    player->LearnSpell(49001, false);
                    player->LearnSpell(49045, false);
                    player->LearnSpell(53338, false);
                    player->LearnSpell(5116, false);
                    player->LearnSpell(27044, false);
                    player->LearnSpell(883, false);
                    player->LearnSpell(2641, false);
                    player->LearnSpell(6991, false);
                    player->LearnSpell(982, false);
                    player->LearnSpell(1515, false);
                    player->LearnSpell(19883, false);
                    player->LearnSpell(20736, false);
                    player->LearnSpell(48990, false);
                    player->LearnSpell(2974, false);
                    player->LearnSpell(6197, false);
                    player->LearnSpell(1002, false);
                    player->LearnSpell(14327, false);
                    player->LearnSpell(5118, false);
                    player->LearnSpell(49056, false);
                    player->LearnSpell(53339, false);
                    player->LearnSpell(49048, false);
                    player->LearnSpell(19884, false);
                    player->LearnSpell(34074, false);
                    player->LearnSpell(781, false);
                    player->LearnSpell(14311, false);
                    player->LearnSpell(1462, false);
                    player->LearnSpell(19885, false);
                    player->LearnSpell(19880, false);
                    player->LearnSpell(13809, false);
                    player->LearnSpell(13161, false);
                    player->LearnSpell(5384, false);
                    player->LearnSpell(1543, false);
                    player->LearnSpell(19878, false);
                    player->LearnSpell(49067, false);
                    player->LearnSpell(3034, false);
                    player->LearnSpell(13159, false);
                    player->LearnSpell(19882, false);
                    player->LearnSpell(58434, false);
                    player->LearnSpell(49071, false);
                    player->LearnSpell(49052, false);
                    player->LearnSpell(19879, false);
                    player->LearnSpell(19263, false);
                    player->LearnSpell(19801, false);
                    player->LearnSpell(34026, false);
                    player->LearnSpell(34600, false);
                    player->LearnSpell(34477, false);
                    player->LearnSpell(61006, false);
                    player->LearnSpell(61847, false);
                    player->LearnSpell(53271, false);
                    player->LearnSpell(60192, false);
                    player->LearnSpell(62757, false);
                    if (player->HasSpell(19386))
                    player->LearnSpell(49012, false);
                    if (player->HasSpell(53301))
                    player->LearnSpell(60053, false);
                    if (player->HasSpell(19306))
                    player->LearnSpell(48999, false);
                    if (player->HasSpell(19434))
                    player->LearnSpell(49050, false);
                    player->SaveToDB();
                    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_MAGE:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(42921, false);
                    player->LearnSpell(42842, false);
                    player->LearnSpell(42995, false);
                    player->LearnSpell(42833, false);
                    player->LearnSpell(27090, false);
                    player->LearnSpell(33717, false);
                    player->LearnSpell(42873, false);
                    player->LearnSpell(42846, false);
                    player->LearnSpell(12826, false);
                    player->LearnSpell(28271, false);
                    player->LearnSpell(61780, false);
                    player->LearnSpell(61721, false);
                    player->LearnSpell(28272, false);
                    player->LearnSpell(42917, false);
                    player->LearnSpell(43015, false);
                    player->LearnSpell(130, false);
                    player->LearnSpell(42926, false);
                    player->LearnSpell(43017, false);
                    player->LearnSpell(475, false);
                    player->LearnSpell(1953, false);
                    player->LearnSpell(42940, false);
                    player->LearnSpell(12051, false);
                    player->LearnSpell(43010, false);
                    player->LearnSpell(43020, false);
                    player->LearnSpell(43012, false);
                    player->LearnSpell(42859, false);
                    player->LearnSpell(2139, false);
                    player->LearnSpell(42931, false);
                    player->LearnSpell(42985, false);
                    player->LearnSpell(43008, false);
                    player->LearnSpell(45438, false);
                    player->LearnSpell(43024, false);
                    player->LearnSpell(43002, false);
                    player->LearnSpell(43046, false);
                    player->LearnSpell(42897, false);
                    player->LearnSpell(42914, false);
                    player->LearnSpell(66, false);
                    player->LearnSpell(58659, false);
                    player->LearnSpell(30449, false);
                    player->LearnSpell(42956, false);
                    player->LearnSpell(47610, false);
                    player->LearnSpell(61316, false);
                    player->LearnSpell(61024, false);
                    player->LearnSpell(55342, false);
                    player->LearnSpell(53142, false);
                    player->LearnSpell(7301, false);
                    if (player->GetTeam() == ALLIANCE)
                    {
                        player->LearnSpell(32271, false); // Teleport: Exodar.
                        player->LearnSpell(49359, false); // Teleport: Theramore.
                        player->LearnSpell(3565, false); // Teleport: Darnassus.
                        player->LearnSpell(33690, false); // Teleport: Shattrath.
                        player->LearnSpell(3562, false); // Teleport: Ironforge.
                        player->LearnSpell(3561, false); // Teleport: Stormwind.
                        player->LearnSpell(53140, false); // Teleport: Dalaran.
                        player->LearnSpell(53142, false); // Portal: Dalaran.
                        player->LearnSpell(10059, false); // Portal: Stormwind.
                        player->LearnSpell(11419, false); // Portal: Darnassus.
                        player->LearnSpell(32266, false); // Portal: Exodar.
                        player->LearnSpell(11416, false); // Portal: Ironforge.
                        player->LearnSpell(33691, false); // Portal: Shattrath.
                        player->LearnSpell(49360, false); // Portal: Theramore.
                    }
                    if (player->GetTeam() == HORDE)
                    {
                        player->LearnSpell(3567, false); // Teleport: Orgrimmar.
                        player->LearnSpell(35715, false); // Teleport: Shattrath.
                        player->LearnSpell(3566, false); // Teleport: Thunder Bluff.
                        player->LearnSpell(49358, false); // Teleport: Stonard.
                        player->LearnSpell(32272, false); // Teleport: Silvermoon.
                        player->LearnSpell(3563, false); // Teleport: Undercity.
                        player->LearnSpell(53140, false); // Teleport: Dalaran.
                        player->LearnSpell(53142, false); // Portal: Dalaran.
                        player->LearnSpell(11417, false); // Portal: Orgrimmar.
                        player->LearnSpell(35717, false); // Portal: Shattrath.
                        player->LearnSpell(32267, false); // Portal: Silvermoon.
                        player->LearnSpell(49361, false); // Portal: Stonard.
                        player->LearnSpell(11420, false); // Portal: Thunder Bluff.
                        player->LearnSpell(11418, false); // Portal: Undercity.
                    }
                    if (player->HasSpell(11366))
                    player->LearnSpell(42891, false);
                    if (player->HasSpell(11426))
                    player->LearnSpell(43039, false);
                    if (player->HasSpell(44457))
                    player->LearnSpell(55360, false);
                    if (player->HasSpell(31661))
                    player->LearnSpell(42950, false);
                    if (player->HasSpell(11113))
                    player->LearnSpell(42945, false);
                    if (player->HasSpell(44425))
                    player->LearnSpell(44781, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_WARLOCK:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(696, false);
                    player->LearnSpell(47811, false);
                    player->LearnSpell(47809, false);
                    player->LearnSpell(688, false);
                    player->LearnSpell(47813, false);
                    player->LearnSpell(50511, false);
                    player->LearnSpell(57946, false);
                    player->LearnSpell(47864, false);
                    player->LearnSpell(6215, false);
                    player->LearnSpell(47878, false);
                    player->LearnSpell(47855, false);
                    player->LearnSpell(697, false);
                    player->LearnSpell(47856, false);
                    player->LearnSpell(47857, false);
                    player->LearnSpell(5697, false);
                    player->LearnSpell(47884, false);
                    player->LearnSpell(47815, false);
                    player->LearnSpell(47889, false);
                    player->LearnSpell(47820, false);
                    player->LearnSpell(698, false);
                    player->LearnSpell(712, false);
                    player->LearnSpell(126, false);
                    player->LearnSpell(5138, false);
                    player->LearnSpell(5500, false);
                    player->LearnSpell(11719, false);
                    player->LearnSpell(132, false);
                    player->LearnSpell(60220, false);
                    player->LearnSpell(18647, false);
                    player->LearnSpell(61191, false);
                    player->LearnSpell(47823, false);
                    player->LearnSpell(691, false);
                    player->LearnSpell(47865, false);
                    player->LearnSpell(47891, false);
                    player->LearnSpell(47888, false);
                    player->LearnSpell(17928, false);
                    player->LearnSpell(47860, false);
                    player->LearnSpell(47825, false);
                    player->LearnSpell(1122, false);
                    player->LearnSpell(47867, false);
                    player->LearnSpell(18540, false);
                    player->LearnSpell(47893, false);
                    player->LearnSpell(47838, false);
                    player->LearnSpell(29858, false);
                    player->LearnSpell(58887, false);
                    player->LearnSpell(47836, false);
                    player->LearnSpell(61290, false);
                    player->LearnSpell(48018, false);
                    player->LearnSpell(48020, false);
                    player->LearnSpell(33388, false);
                    player->LearnSpell(33391, false);
                    player->LearnSpell(23161, false);
                    if (player->HasSpell(17877))
                    player->LearnSpell(47827, false);
                    if (player->HasSpell(30283))
                    player->LearnSpell(47847, false);
                    if (player->HasSpell(30108))
                    player->LearnSpell(47843, false);
                    if (player->HasSpell(50796))
                    player->LearnSpell(59172, false);
                    if (player->HasSpell(48181))
                    player->LearnSpell(59164, false);
                    if (player->HasSpell(18220))
                    player->LearnSpell(59092, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_ROGUE:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(3127, false);
                    player->LearnSpell(42459, false);
                    player->LearnSpell(48668, false);
                    player->LearnSpell(48638, false);
                    player->LearnSpell(1784, false);
                    player->LearnSpell(48657, false);
                    player->LearnSpell(921, false);
                    player->LearnSpell(1776, false);
                    player->LearnSpell(26669, false);
                    player->LearnSpell(51724, false);
                    player->LearnSpell(6774, false);
                    player->LearnSpell(11305, false);
                    player->LearnSpell(1766, false);
                    player->LearnSpell(48676, false);
                    player->LearnSpell(48659, false);
                    player->LearnSpell(1804, false);
                    player->LearnSpell(8647, false);
                    player->LearnSpell(48691, false);
                    player->LearnSpell(51722, false);
                    player->LearnSpell(48672, false);
                    player->LearnSpell(1725, false);
                    player->LearnSpell(26889, false);
                    player->LearnSpell(2836, false);
                    player->LearnSpell(1833, false);
                    player->LearnSpell(1842, false);
                    player->LearnSpell(8643, false);
                    player->LearnSpell(2094, false);
                    player->LearnSpell(1860, false);
                    player->LearnSpell(57993, false);
                    player->LearnSpell(48674, false);
                    player->LearnSpell(31224, false);
                    player->LearnSpell(5938, false);
                    player->LearnSpell(57934, false);
                    player->LearnSpell(51723, false);
                    if (player->HasSpell(16511))
                    player->LearnSpell(48660, false);
                    if (player->HasSpell(1329))
                    player->LearnSpell(48666, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_PRIEST:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(528, false);
                    player->LearnSpell(2053, false);
                    player->LearnSpell(48161, false);
                    player->LearnSpell(48123, false);
                    player->LearnSpell(48125, false);
                    player->LearnSpell(48066, false);
                    player->LearnSpell(586, false);
                    player->LearnSpell(48068, false);
                    player->LearnSpell(48127, false);
                    player->LearnSpell(48171, false);
                    player->LearnSpell(48168, false);
                    player->LearnSpell(10890, false);
                    player->LearnSpell(6064, false);
                    player->LearnSpell(988, false);
                    player->LearnSpell(48300, false);
                    player->LearnSpell(6346, false);
                    player->LearnSpell(48071, false);
                    player->LearnSpell(48135, false);
                    player->LearnSpell(48078, false);
                    player->LearnSpell(453, false);
                    player->LearnSpell(10955, false);
                    player->LearnSpell(10909, false);
                    player->LearnSpell(8129, false);
                    player->LearnSpell(48073, false);
                    player->LearnSpell(605, false);
                    player->LearnSpell(48072, false);
                    player->LearnSpell(48169, false);
                    player->LearnSpell(552, false);
                    player->LearnSpell(1706, false);
                    player->LearnSpell(48063, false);
                    player->LearnSpell(48162, false);
                    player->LearnSpell(48170, false);
                    player->LearnSpell(48074, false);
                    player->LearnSpell(48158, false);
                    player->LearnSpell(48120, false);
                    player->LearnSpell(34433, false);
                    player->LearnSpell(48113, false);
                    player->LearnSpell(32375, false);
                    player->LearnSpell(64843, false);
                    player->LearnSpell(64901, false);
                    player->LearnSpell(53023, false);
                    if (player->HasSpell(34914))
                    player->LearnSpell(48160, false);
                    if (player->HasSpell(47540))
                    player->LearnSpell(53007, false);
                    if (player->HasSpell(724))
                    player->LearnSpell(48087, false);
                    if (player->HasSpell(19236))
                    player->LearnSpell(48173, false);
                    if (player->HasSpell(34861))
                    player->LearnSpell(48089, false);
                    if (player->HasSpell(15407))
                    player->LearnSpell(48156, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_DEATH_KNIGHT:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(3127, false);
                    player->LearnSpell(50842, false);
                    player->LearnSpell(49941, false);
                    player->LearnSpell(49930, false);
                    player->LearnSpell(47476, false);
                    player->LearnSpell(45529, false);
                    player->LearnSpell(3714, false);
                    player->LearnSpell(56222, false);
                    player->LearnSpell(48743, false);
                    player->LearnSpell(48263, false);
                    player->LearnSpell(49909, false);
                    player->LearnSpell(47528, false);
                    player->LearnSpell(45524, false);
                    player->LearnSpell(48792, false);
                    player->LearnSpell(57623, false);
                    player->LearnSpell(56815, false);
                    player->LearnSpell(47568, false);
                    player->LearnSpell(49895, false);
                    player->LearnSpell(50977, false);
                    player->LearnSpell(49576, false);
                    player->LearnSpell(49921, false);
                    player->LearnSpell(46584, false);
                    player->LearnSpell(49938, false);
                    player->LearnSpell(48707, false);
                    player->LearnSpell(48265, false);
                    player->LearnSpell(61999, false);
                    player->LearnSpell(42650, false);
                    player->LearnSpell(53428, false);
                    player->LearnSpell(53331, false);
                    player->LearnSpell(54447, false);
                    player->LearnSpell(53342, false);
                    player->LearnSpell(54446, false);
                    player->LearnSpell(53323, false);
                    player->LearnSpell(53344, false);
                    player->LearnSpell(70164, false);
                    player->LearnSpell(62158, false);
                    player->LearnSpell(33391, false);
                    player->LearnSpell(48778, false);
                    player->LearnSpell(51425, false);
                    player->LearnSpell(49924, false);
                    if (player->HasSpell(55050))
                    player->LearnSpell(55262, false);
                    if (player->HasSpell(49143))
                    player->LearnSpell(55268, false);
                    if (player->HasSpell(49184))
                    player->LearnSpell(51411, false);
                    if (player->HasSpell(55090))
                    player->LearnSpell(55271, false);
                    if (player->HasSpell(49158))
                    player->LearnSpell(51328, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_SHAMAN:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(2062, false);
                    player->LearnSpell(8737, false);
                    player->LearnSpell(49273, false);
                    player->LearnSpell(49238, false);
                    player->LearnSpell(10399, false);
                    player->LearnSpell(49231, false);
                    player->LearnSpell(58753, false);
                    player->LearnSpell(2484, false);
                    player->LearnSpell(49281, false);
                    player->LearnSpell(58582, false);
                    player->LearnSpell(49233, false);
                    player->LearnSpell(58790, false);
                    player->LearnSpell(58704, false);
                    player->LearnSpell(58643, false);
                    player->LearnSpell(49277, false);
                    player->LearnSpell(61657, false);
                    player->LearnSpell(8012, false);
                    player->LearnSpell(526, false);
                    player->LearnSpell(2645, false);
                    player->LearnSpell(57994, false);
                    player->LearnSpell(8143, false);
                    player->LearnSpell(49236, false);
                    player->LearnSpell(58796, false);
                    player->LearnSpell(58757, false);
                    player->LearnSpell(49276, false);
                    player->LearnSpell(57960, false);
                    player->LearnSpell(131, false);
                    player->LearnSpell(58745, false);
                    player->LearnSpell(6196, false);
                    player->LearnSpell(58734, false);
                    player->LearnSpell(58774, false);
                    player->LearnSpell(58739, false);
                    player->LearnSpell(58656, false);
                    player->LearnSpell(546, false);
                    player->LearnSpell(556, false);
                    player->LearnSpell(66842, false);
                    player->LearnSpell(51994, false);
                    player->LearnSpell(8177, false);
                    player->LearnSpell(58749, false);
                    player->LearnSpell(20608, false);
                    player->LearnSpell(36936, false);
                    player->LearnSpell(36936, false);
                    player->LearnSpell(58804, false);
                    player->LearnSpell(49271, false);
                    player->LearnSpell(8512, false);
                    player->LearnSpell(6495, false);
                    player->LearnSpell(8170, false);
                    player->LearnSpell(66843, false);
                    player->LearnSpell(55459, false);
                    player->LearnSpell(66844, false);
                    player->LearnSpell(3738, false);
                    player->LearnSpell(2894, false);
                    player->LearnSpell(60043, false);
                    player->LearnSpell(51514, false);
                    if (player->GetTeam() == ALLIANCE)
                    player->LearnSpell(32182, false);
                    if (player->GetTeam() == HORDE)
                    player->LearnSpell(2825, false);
                    if (player->HasSpell(61295))
                    player->LearnSpell(61301, false);
                    if (player->HasSpell(974))
                    player->LearnSpell(49284, false);
                    if (player->HasSpell(30706))
                    player->LearnSpell(57722, false);
                    if (player->HasSpell(51490))
                    player->LearnSpell(59159, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                    break;

                case CLASS_DRUID:
                    player->RemoveSpell(28880);
                    player->RemoveSpell(26297);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(7744);
                    player->RemoveSpell(59752);
                    player->RemoveSpell(20594);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(58984);
                    player->RemoveSpell(20549);
                    player->RemoveSpell(20589);
                    player->RemoveSpell(33697);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(59545);
                    player->RemoveSpell(59544);
                    player->RemoveSpell(59547);
                    player->RemoveSpell(59543);
                    player->RemoveSpell(59542);
                    player->RemoveSpell(59548);
                    player->RemoveSpell(50613);
                    player->RemoveSpell(28730);
                    player->RemoveSpell(25046);
                    player->RemoveSpell(20572);
                    player->RemoveSpell(33702);
                    player->RemoveSpell(33697);
                    player->LearnSpell(48378, false);
                    player->LearnSpell(48469, false);
                    player->LearnSpell(48461, false);
                    player->LearnSpell(48463, false);
                    player->LearnSpell(48441, false);
                    player->LearnSpell(53307, false);
                    player->LearnSpell(53308, false);
                    player->LearnSpell(5487, false);
                    player->LearnSpell(48560, false);
                    player->LearnSpell(6795, false);
                    player->LearnSpell(48480, false);
                    player->LearnSpell(53312, false);
                    player->LearnSpell(18960, false);
                    player->LearnSpell(5229, false);
                    player->LearnSpell(48443, false);
                    player->LearnSpell(50763, false);
                    player->LearnSpell(8983, false);
                    player->LearnSpell(8946, false);
                    player->LearnSpell(1066, false);
                    player->LearnSpell(48562, false);
                    player->LearnSpell(783, false);
                    player->LearnSpell(770, false);
                    player->LearnSpell(16857, false);
                    player->LearnSpell(18658, false);
                    player->LearnSpell(768, false);
                    player->LearnSpell(1082, false);
                    player->LearnSpell(5215, false);
                    player->LearnSpell(48477, false);
                    player->LearnSpell(49800, false);
                    player->LearnSpell(48465, false);
                    player->LearnSpell(48572, false);
                    player->LearnSpell(26995, false);
                    player->LearnSpell(48574, false);
                    player->LearnSpell(2782, false);
                    player->LearnSpell(50213, false);
                    player->LearnSpell(2893, false);
                    player->LearnSpell(33357, false);
                    player->LearnSpell(5209, false);
                    player->LearnSpell(48575, false);
                    player->LearnSpell(48447, false);
                    player->LearnSpell(48577, false);
                    player->LearnSpell(48579, false);
                    player->LearnSpell(5225, false);
                    player->LearnSpell(22842, false);
                    player->LearnSpell(49803, false);
                    player->LearnSpell(9634, false);
                    player->LearnSpell(20719, false);
                    player->LearnSpell(48467, false);
                    player->LearnSpell(29166, false);
                    player->LearnSpell(62600, false);
                    player->LearnSpell(22812, false);
                    player->LearnSpell(48470, false);
                    player->LearnSpell(33943, false);
                    player->LearnSpell(49802, false);
                    player->LearnSpell(48451, false);
                    player->LearnSpell(48568, false);
                    player->LearnSpell(33786, false);
                    player->LearnSpell(40120, false);
                    player->LearnSpell(62078, false);
                    player->LearnSpell(52610, false);
                    player->LearnSpell(50464, false);
                    player->LearnSpell(48570, false);
                    if (player->HasSpell(50516))
                    player->LearnSpell(61384, false);
                    if (player->HasSpell(48505))
                    player->LearnSpell(53201, false);
                    if (player->HasSpell(48438))
                    player->LearnSpell(53251, false);
                    if (player->HasSpell(5570))
                    player->LearnSpell(48468, false);
                    player->SaveToDB();
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                switch (player->getRace())
                {
                case RACE_HUMAN:
                    player->LearnSpell(20598, false);
                    player->LearnSpell(20864, false);
                    player->LearnSpell(20597, false);
                    player->LearnSpell(59752, false);
                    player->LearnSpell(20599, false);
                    player->LearnSpell(58985, false);
                    break;
                case RACE_DRAENEI:
                    player->LearnSpell(6562, false);
                    player->LearnSpell(28875, false);
                    player->LearnSpell(59221, false);
                    if (player->getClass() == CLASS_DEATH_KNIGHT)
                    {
                        player->LearnSpell(59545, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_WARRIOR)
                    {
                        player->LearnSpell(28880, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_PRIEST)
                    {
                        player->LearnSpell(59544, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_SHAMAN)
                    {
                        player->LearnSpell(59547, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_HUNTER)
                    {
                        player->LearnSpell(59543, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_PALADIN)
                    {
                        player->LearnSpell(59542, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_MAGE)
                    {
                        player->LearnSpell(59548, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    break;
                case RACE_DWARF:
                    player->LearnSpell(20595, false);
                    player->LearnSpell(59224, false);
                    player->LearnSpell(20596, false);
                    player->LearnSpell(2481, false);
                    player->LearnSpell(20594, false);
                    break;
                case RACE_NIGHTELF:
                    player->LearnSpell(20583, false);
                    player->LearnSpell(58984, false);
                    player->LearnSpell(20582, false);
                    player->LearnSpell(20585, false);
                    break;
                case RACE_GNOME:
                    player->LearnSpell(20589, false);
                    player->LearnSpell(20591, false);
                    player->LearnSpell(20592, false);
                    player->LearnSpell(20593, false);
                    break;  
                case RACE_ORC:
                    player->LearnSpell(20573, false);
                    player->LearnSpell(20574, false);
                    player->LearnSpell(21563, false);
                    if (player->getClass() == CLASS_DEATH_KNIGHT)
                    {
                        player->LearnSpell(20572, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_WARRIOR || player->getClass() == CLASS_ROGUE)
                    {
                        player->LearnSpell(20572, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_PRIEST)
                    {
                        player->LearnSpell(33702, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_SHAMAN || player->getClass() == CLASS_DRUID)
                    {
                        player->LearnSpell(33697, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_HUNTER)
                    {
                        player->LearnSpell(20572, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_PALADIN)
                    {
                        player->LearnSpell(33697, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_MAGE || player->getClass() == CLASS_WARLOCK)
                    {
                        player->LearnSpell(33702, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    break;
                case RACE_UNDEAD_PLAYER:
                    player->LearnSpell(7744, false);
                    player->LearnSpell(20577, false);
                    player->LearnSpell(5227, false);
                    player->LearnSpell(20579, false);
                    break; 
                case RACE_TAUREN:
                    player->LearnSpell(20551, false);
                    player->LearnSpell(20552, false);
                    player->LearnSpell(20550, false);
                    player->LearnSpell(20549, false);
                    break;        
                case RACE_TROLL:
                    player->LearnSpell(20557, false);
                    player->LearnSpell(20558, false);
                    player->LearnSpell(26290, false);
                    player->LearnSpell(20555, false);
                    player->LearnSpell(58943, false);
                    player->LearnSpell(26297, false);
                    break;    
                case RACE_BLOODELF:             
                    player->LearnSpell(822, false);
                    player->LearnSpell(28877, false);
                    if (player->getClass() == CLASS_DEATH_KNIGHT)
                    {
                        player->LearnSpell(50613, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_WARRIOR || player->getClass() == CLASS_ROGUE)
                    {
                        player->LearnSpell(25046, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_PRIEST)
                    {
                        player->LearnSpell(28730, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_SHAMAN || player->getClass() == CLASS_DRUID)
                    {
                        player->LearnSpell(28730, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_HUNTER)
                    {
                        player->LearnSpell(28730, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_PALADIN)
                    {
                        player->LearnSpell(28730, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    else if (player->getClass() == CLASS_MAGE || player->getClass() == CLASS_WARLOCK)
                    {
                        player->LearnSpell(28730, false);
                        player->SaveToDB();
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                    break;                 
                }
                break;
                

            case 2: // Двойная специализация
                if (player->GetSpecsCount() == 1 && !(player->getLevel() < sWorld->getIntConfig(CONFIG_MIN_DUALSPEC_LEVEL)))
                player->CastSpell(player, 63680, true, NULL, NULL, player->GetGUID());
                player->CastSpell(player, 63624, true, NULL, NULL, player->GetGUID());
                player->CLOSE_GOSSIP_MENU();
                break;
            case 3: // Сброс талантов
                //Добавление меню Да / Нет
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_RESET_YES, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_RESET_NO, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                player->PlayerTalkClass->SendGossipMenu(68, creature->GetGUID());
                break;

                // Yes Menu
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->ResetTalents(true);
                player->SendTalentsInfoData(false);
                player->CLOSE_GOSSIP_MENU();
                //creature->Whisper(MSG_RESET_TALENTS, LANG_UNIVERSAL, player, false);
                break;

                // No Menu
            case GOSSIP_ACTION_INFO_DEF + 2:
                OnGossipHello(player, creature);
                break;
            case 4: // Профессии
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/Trade_Alchemy:30|t Алхимия.|r", GOSSIP_SENDER_MAIN, 100);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/Trade_BlackSmithing:30|t Кузнечное дело.|r", GOSSIP_SENDER_MAIN, 101);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/INV_Misc_ArmorKit_17:30|t Кожевничество.|r", GOSSIP_SENDER_MAIN, 102);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/Trade_Tailoring:30|t Портняжное дело.|r", GOSSIP_SENDER_MAIN, 103);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/Trade_Engineering:30|t Инженерное дело.|r", GOSSIP_SENDER_MAIN, 104);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/Trade_Engraving:30|t Наложение чар.|r", GOSSIP_SENDER_MAIN, 105);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/INV_Misc_Gem_01:30|t Ювелирное дело.|r", GOSSIP_SENDER_MAIN, 106);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/INV_Inscription_Tradeskill01:30|t Начертание.|r", GOSSIP_SENDER_MAIN, 107);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/Spell_Nature_NatureTouchGrow:30|t Травничество.|r", GOSSIP_SENDER_MAIN, 108);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/INV_Misc_Pelt_Wolf_01:30|t Снятие шкур.|r", GOSSIP_SENDER_MAIN, 109);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "|TInterface/ICONS/Trade_Mining:30|t Горное дело.|r", GOSSIP_SENDER_MAIN, 110);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|TInterface/ICONS/INV_Misc_Food_15:30|t Кулинария.|r", GOSSIP_SENDER_MAIN, 111);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|TInterface/ICONS/Spell_Holy_SealOfSacrifice:30|t Первая помощь.|r", GOSSIP_SENDER_MAIN, 112);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "|TInterface/ICONS/Trade_Fishing:30|t Рыбная ловля.|r", GOSSIP_SENDER_MAIN, 113);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "|TInterface/icons/Spell_ChargeNegative:30|t Закрыть окно!|r", GOSSIP_SENDER_MAIN, 114);
                player->PlayerTalkClass->SendGossipMenu(68, creature->GetGUID());
                break;
            case 100:
                if (player->HasSkill(SKILL_ALCHEMY))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }

                CompleteLearnProfession(player, creature, SKILL_ALCHEMY);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 101:
                if (player->HasSkill(SKILL_BLACKSMITHING))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                CompleteLearnProfession(player, creature, SKILL_BLACKSMITHING);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 102:
                if (player->HasSkill(SKILL_LEATHERWORKING))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                CompleteLearnProfession(player, creature,  SKILL_LEATHERWORKING);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 103:
                if (player->HasSkill(SKILL_TAILORING))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                CompleteLearnProfession(player, creature,  SKILL_TAILORING);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 104:
                if (player->HasSkill(SKILL_ENGINEERING))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                CompleteLearnProfession(player, creature,  SKILL_ENGINEERING);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 105:
                if (player->HasSkill(SKILL_ENCHANTING))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                CompleteLearnProfession(player, creature,  SKILL_ENCHANTING);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 106:
                if (player->HasSkill(SKILL_JEWELCRAFTING))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                CompleteLearnProfession(player, creature,  SKILL_JEWELCRAFTING);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 107:
                if (player->HasSkill(SKILL_INSCRIPTION))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }
                CompleteLearnProfession(player, creature,  SKILL_INSCRIPTION);

                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 108:
                if (player->HasSkill(SKILL_HERBALISM))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }

                CompleteLearnProfession(player, creature,  SKILL_HERBALISM);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 109:
                if (player->HasSkill(SKILL_SKINNING))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }

                CompleteLearnProfession(player, creature,  SKILL_SKINNING);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 110:
                if (player->HasSkill(SKILL_MINING))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }

                CompleteLearnProfession(player, creature,  SKILL_MINING);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 111:
                if (player->HasSkill(SKILL_COOKING))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }

                CompleteLearnProfession(player, creature,  SKILL_COOKING);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 112:
                if (player->HasSkill(SKILL_FIRST_AID))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }

                CompleteLearnProfession(player, creature,  SKILL_FIRST_AID);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 113:
                if (player->HasSkill(SKILL_FISHING))
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    break;
                }

                CompleteLearnProfession(player, creature,  SKILL_FISHING);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            case 8:
                player->CombatStop();
                player->CastSpell(player, 24222);
                player->CLOSE_GOSSIP_MENU();
                break;
            case 5: // Верховая езда
                player->CLOSE_GOSSIP_MENU();
                //creature->Whisper(MSG_RIDING_COMPLETE, LANG_UNIVERSAL, player, false);
                player->CastSpell(player, 33389, false); // Apprentice Riding
                player->CastSpell(player, 33392, false); // Journeyman Riding
                player->CastSpell(player, 34092, false); // Expert Riding
                player->CastSpell(player, 34093, false); // Artisan Riding
                player->LearnSpell(54197, false); // Allows you to ride flying mounts in Northrend.
                break;
            case 6: // Оружейные навыки
                switch (player->getClass())
                {
                case CLASS_WARRIOR:
                    player->CLOSE_GOSSIP_MENU();
                    //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                    player->LearnSpell(5011, false); // Арбалеты
                    player->LearnSpell(200, false); // Древковое оружие
                    player->LearnSpell(15590, false); // Кулачное оружие
                    player->LearnSpell(264, false); // Луки
                    player->LearnSpell(266, false); // Ружья
                    player->LearnSpell(227, false); // Посохи
                    break;
                case CLASS_PALADIN:
                    player->CLOSE_GOSSIP_MENU();
                    //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                    player->LearnSpell(197, false); // Двуручные топоры
                    player->LearnSpell(200, false); // Древковое оружие
                    player->LearnSpell(196, false); // Одноручные топоры
                    break;
                case CLASS_WARLOCK:
                    player->CLOSE_GOSSIP_MENU();
                    //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                    player->LearnSpell(201, false); // Одноручные мечи
                    break;
                case CLASS_PRIEST:
                    player->CLOSE_GOSSIP_MENU();
                    //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                    player->LearnSpell(1180, false); // Кинжалы
                    break;
                case CLASS_HUNTER:
                    player->CLOSE_GOSSIP_MENU();
                    //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                    player->LearnSpell(5011, false); // Арбалеты
                    player->LearnSpell(202, false); // Двуручные мечи
                    player->LearnSpell(200, false); // Древковое оружие
                    player->LearnSpell(15590, false); // Кулачное оружие
                    player->LearnSpell(264, false); // Луки
                    player->LearnSpell(2567, false); // Метательное оружие
                    player->LearnSpell(227, false); // Посохи
                    break;
                case CLASS_MAGE:
                    player->CLOSE_GOSSIP_MENU();
                    ////creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                    player->LearnSpell(1180, false); // Кинжалы
                    player->LearnSpell(201, false); // Одноручные мечи
                    break;
                case CLASS_SHAMAN:
                    player->CLOSE_GOSSIP_MENU();
                    ////creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                    player->LearnSpell(199, false); // Двуручное дробящее оружие
                    player->LearnSpell(197, false); // Двуручные топоры
                    player->LearnSpell(1180, false); // Кинжалы
                    player->LearnSpell(15590, false); // Кулачное оружие
                    player->LearnSpell(196, false); // Одноручные топоры
                    break;
                case CLASS_ROGUE:
                    player->CLOSE_GOSSIP_MENU();
                    //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                    player->LearnSpell(5011, false); // Арбалеты
                    player->LearnSpell(198, false); // Одноручное дробящее оружие
                    player->LearnSpell(15590, false); // Кулачное оружие
                    player->LearnSpell(264, false); // Луки
                    player->LearnSpell(201, false); // Одноручные мечи
                    player->LearnSpell(266, false); // Ружья
                    player->LearnSpell(196, false); // Одноручные топоры
                    break;
                case CLASS_DEATH_KNIGHT:
                    player->CLOSE_GOSSIP_MENU();
                    //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                    player->LearnSpell(199, false); // Двуручное дробящее оружие
                    player->LearnSpell(198, false); // Одноручное дробящее оружие
                    break;
                case CLASS_DRUID:
                    player->CLOSE_GOSSIP_MENU();
                    //creature->Whisper(MSG_WEAPON_SKILLS_COMPLETE, LANG_UNIVERSAL, player, false);
                    player->LearnSpell(199, false); // Двуручное дробящее оружие
                    player->LearnSpell(200, false); // Древковое оружие
                    player->LearnSpell(15590, false); // Кулачное оружие
                    break;
                }
            case 7: // Улучшить навыки защиты и владения оружием до максимума
                player->CLOSE_GOSSIP_MENU();
                //creature->Whisper(MSG_MAX_SKILL, LANG_UNIVERSAL, player, false);
                player->UpdateSkillsToMaxSkillsForLevel();
                break;			
            }
        }
        return true;
    }
};

/*#####################*\
## tele_gossip_codebox ##
\*#####################*/

// Teleport data

struct Teleport
{
    uint32 menu_id, next_menu_id;
    uint8 icon;
    std::string name;
    uint32 cost;
    uint8 level, faction; // 0 Both, 1 Horde, 2 Ally
    uint32 map;
    float x, y, z, o;
};
typedef struct Teleport Data;
Data Tele[] = // key = i & Key
{
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportStormWind:25|t Штормград", 0, 0, 2, 0, -8842.09f, 626.358f, 94.0867f, 3.61363f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportOrgrimmar:25|t Оргриммар", 0, 0, 1, 1, 1601.08f, -4378.69f, 9.9846f, 2.14362f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportDarnassus:25|t Дарнас", 0, 0, 2, 1, 9869.91f, 2493.58f, 1315.88f, 2.78897f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportIronForge:25|t Стальгорн", 0, 0, 2, 0, -4900.47f, -962.585f, 501.455f, 5.40538f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportExodar:25|t Эксодар", 0, 0, 2, 530, -3864.92f, -11643.7f, -137.644f, 5.50862f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportThunderBluff:25|t Громовой утес", 0, 0, 1, 1, -1274.45f, 71.8601f, 128.159f, 2.80623f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportUnderCity:25|t Подгород", 0, 0, 1, 0, 1633.75f, 240.167f, -43.1034f, 6.26128f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportSilvermoon:25|t Луносвет", 0, 0, 1, 530, 9738.28f, -7454.19f, 13.5605f, 0.043914f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportDalaran:25|t Даларан", 0, 67, 0, 571, 5809.55f, 503.975f, 657.526f, 2.38338f },
    { 1, 0, 2, "|TInterface/ICONS/Spell_Arcane_TeleportShattrath:25|t Шаттрат", 0, 57, 0, 530, -1887.62f, 5359.09f, -12.4279f, 4.40435f },
    { 1, 0, 2, "|TInterface/ICONS/Achievement_BG_killingblow_most:25|t Прибамбасск", 0, 40, 0, 1, -7177.15f, -3785.34f, 8.36981f, 6.10237f },
    { 1, 0, 2, "|TInterface/ICONS/Achievement_BG_killingblow_most:25|t Кабестан", 0, 10, 0, 1, -956.664f, -3754.709f, 5.33239f, 0.99663f },
    { 1, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Дуэль зона", 0, 80, 0, 37, 1194.983887f, 186.221924f, 356.908295f, 5.434835f },
    { 1, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Арена Гурубаши", 0, 30, 0, 0, -13181.8f, 339.356f, 42.9805f, 1.18013f },
    { 1, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Арена в Круге Крови", 0, 57, 0, 530, 2839.44f, 5930.17f, 11.1002f, 3.16284f },
    { 1, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Круг Испытаний", 0, 57, 0, 530, -1999.94f, 6581.71f, 11.32f, 2.36528f },
    { 1, 6, 3, "|TInterface/ICONS/INV_BannerPVP_02:25|t Восточные королевства", 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 7, 3, "|TInterface/ICONS/INV_BannerPVP_01:25|t Калимдор", 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 8, 3, "|TInterface/ICONS/Achievement_Dungeon_Outland_DungeonMaster:25|t Запределье", 0, 58, 0, 0, 0, 0, 0, 0 },
    { 1, 9, 3, "|TInterface/ICONS/Achievement_Zone_BoreanTundra_03:25|t Нордскол", 0, 68, 0, 0, 0, 0, 0, 0 },
    { 1, 2, 9, "|TInterface/ICONS/Achievement_Dungeon_Naxxramas_Heroic:25|t Классические подземелья", 0, 15, 0, 0, 0, 0, 0, 0 },
    { 1, 3, 9, "|TInterface/ICONS/Achievement_Dungeon_Naxxramas_10man:25|t Подземелья BC", 0, 59, 0, 0, 0, 0, 0, 0 },
    { 1, 4, 9, "|TInterface/ICONS/Achievement_Dungeon_Naxxramas_Normal:25|t Подземелья WotLK", 0, 69, 0, 0, 0, 0, 0, 0 },
    { 1, 5, 9, "|TInterface/ICONS/Achievement_Dungeon_Naxxramas_25man:25|t Рейдовые телепоры", 0, 57, 0, 0, 0, 0, 0, 0 },
    { 1, 10, 9, "|TInterface/ICONS/Spell_fire_felfireward:25|t Прочие телепорты", 0, 80, 0, 0, 0, 0, 0, 0 },
    
    { 2, 0, 2, "|TInterface/ICONS/Achievement_Boss_Mekgineer_Thermaplugg .png:25|t Гномреган |cffff0000(24-34)|r", 0, 25, 2, 0, -5163.54f, 925.423f, 257.181f, 1.57423f },
    { 2, 0, 2, "|TInterface/ICONS/Achievement_Boss_EdwinVancleef:25|t Мертвые копи |cffff0000(15-21)|r", 0, 17, 2, 0, -11209.6f, 1666.54f, 24.6974f, 1.42053f },
    { 2, 0, 2, "|TInterface/ICONS/Achievement_Boss_Bazil_Thredd:25|t Тюрьма |cffff0000(20-30)|r", 0, 22, 2, 0, -8799.15f, 832.718f, 97.6348f, 6.04085f },
    { 2, 0, 2, "|TInterface/ICONS/Spell_Shadow_SummonFelGuard:25|t Огненная пропасть |cffff0000(15-21)|r", 0, 15, 1, 1, 1811.78f, -4410.5f, -18.4704f, 5.20165f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_charlgarazorflank:25|t Лабиринты Иглошкурых |cffff0000(35-43)|r", 0, 34, 1, 1, -4657.3f, -2519.35f, 81.0529f, 4.54808f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_amnennar_the_coldbringer:25|t Курганы Иглошкурых |cffff0000(30-40)|r", 0, 24, 1, 1, -4470.28f, -1677.77f, 81.3925f, 1.16302f },
    { 2, 0, 2, "|TInterface/ICONS/inv_helmet_52:25|t Монастырь Алого ордена |cffff0000(26-42)|r", 0, 32, 1, 0, 2873.15f, -764.523f, 160.332f, 5.10447f },
    { 2, 0, 2, "|TInterface/ICONS/Achievement_Boss_ArchmageArugal:25|t Крепость Темного клыка |cffff0000(16-26)|r", 0, 18, 1, 0, -234.675f, 1561.63f, 76.8921f, 1.24031f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_mutanus_the_devourer:25|t Пещеры стенаний |cffff0000(15-25)|r", 0, 17, 1, 1, -731.607f, -2218.39f, 17.0281f, 2.78486f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_bazil_akumai:25|t Непроглядная пучина |cffff0000(20-30)|r", 0, 21, 0, 1, 4249.99f, 740.102f, -25.671f, 1.34062f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_emperordagranthaurissan:25|t Глубины Черной горы |cffff0000(50-60)|r", 0, 53, 0, 0, -7179.34f, -921.212f, 165.821f, 5.09599f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_generaldrakkisath:25|t Пик Черной горы |cffff0000(50-60)|r", 0, 57, 0, 0, -7527.05f, -1226.77f, 285.732f, 5.29626f },
    { 2, 0, 2, "|TInterface/ICONS/ability_warrior_decisivestrike:25|t Забытый Город |cffff0000(50-60)|r", 0, 55, 0, 1, -3520.14f, 1119.38f, 161.025f, 4.70454f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_princesstheradras:25|t Мародон |cffff0000(30-46)|r", 0, 45, 0, 1, -1421.42f, 2907.83f, 137.415f, 1.70718f },
    { 2, 0, 2, "|TInterface/ICONS/spell_holy_senseundead:25|t Некроситет |cffff0000(50-60)|r", 0, 55, 0, 0, 1269.64f, -2556.21f, 93.6088f, 0.620623f },
    { 2, 0, 2, "|TInterface/ICONS/spell_deathknight_armyofthedead:25|t Стратхольм |cffff0000(50-60)|r", 0, 55, 0, 0, 3352.92f, -3379.03f, 144.782f, 6.25978f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_shadeoferanikus:25|t Храм Атал'Хаккара |cffff0000(45-55)|r", 0, 47, 0, 0, -10177.9f, -3994.9f, -111.239f, 6.01885f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_archaedas:25|t Ульдаман |cffff0000(35-45)|r", 0, 37, 0, 0, -6071.37f, -2955.16f, 209.782f, 0.015708f },
    { 2, 0, 2, "|TInterface/ICONS/achievement_boss_chiefukorzsandscalp:25|t Зул'Фаррак |cffff0000(40-48)|r", 0, 35, 0, 1, -6801.19f, -2893.02f, 9.00388f, 0.158639f },
    { 2, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [Назад]", 0, 0, 0, 0, 0, 0, 0, 0 },

    { 3, 0, 2, "|TInterface/ICONS/achievement_boss_exarch_maladaar:25|t Аукиндон |cffff0000(60-70)|r", 0, 64, 0, 530, -3324.49f, 4943.45f, -101.239f, 4.63901f },
    { 3, 0, 2, "|TInterface/ICONS/achievement_boss_epochhunter:25|t Пещеры Времени |cffff0000(60-70)|r", 0, 66, 0, 1, -8369.65f, -4253.11f, -204.272f, -2.70526f },
    { 3, 0, 2, "|TInterface/ICONS/achievement_boss_warlord_kalithresh:25|t Резервуар Кривого Клыка |cffff0000(60-69)|r", 0, 62, 0, 530, 738.865f, 6865.77f, -69.4659f, 6.27655f },
    { 3, 0, 2, "|TInterface/ICONS/achievement_boss_kelidanthebreaker:25|t Цитадель Адского Пламени |cffff0000(60-68)|r", 0, 59, 0, 530, -347.29f, 3089.82f, 21.394f, 5.68114f },
    { 3, 0, 2, "|TInterface/ICONS/Achievement_Boss_Kael\'thasSunstrider_01.png:25|t Терраса Магистров |cffff0000(68-72)|r", 0, 70, 0, 530, 12884.6f, -7317.69f, 65.5023f, 4.799f },
    { 3, 0, 2, "|TInterface/ICONS/achievement_boss_pathaleonthecalculator:25|t Подземелья Крепости Бурь |cffff0000(60-70)|r", 0, 70, 0, 530, 3100.48f, 1536.49f, 190.3f, 4.62226f },
    { 3, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [Назад]", 0, 0, 0, 0, 0, 0, 0, 0 },

    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_azjoluppercity_heroic:25|t Азжол-Неруб |cffff0000(72-74)|r", 0, 73, 0, 571, 3707.86f, 2150.23f, 36.76f, 3.22f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_azjollowercity_heroic:25|t Ан'кахет: Старое Королевство |cffff0000(73-75)|r", 0, 74, 0, 571, 3707.86f, 2150.23f, 36.76f, 3.22f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_cotstratholme_heroic:25|t Очищение Стратхольма |cffff0000(80)|r", 0, 79, 0, 1, -8756.39f, -4440.68f, -199.489f, 4.66289f },
    { 4, 0, 2, "|TInterface/ICONS/inv_spear_05:25|t Испытание чемпиона |cffff0000(80)|r", 0, 79, 0, 571, 8590.95f, 791.792f, 558.235f, 3.13127f },
    { 4, 0, 2, "|TInterface/ICONS/Achievement_Dungeon_Drak\'Tharon_Heroic.png:25|t Крепость Драк'Тарон |cffff0000(74-76)|r", 0, 74, 0, 571, 4765.59f, -2038.24f, 229.363f, 0.887627f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_gundrak_heroic:25|t Гундрак |cffff0000(76-78)|r", 0, 71, 0, 571, 6722.44f, -4640.67f, 450.632f, 3.91123f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_boss_lichking:25|t Подземелья Ледяной Короны |cffff0000(80)|r", 0, 79, 0, 571, 5643.16f, 2028.81f, 798.274f, 4.60242f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_icecrown_pitofsaron:25|t Яма Сарона |cffff0000(80)|r", 0, 79, 0, 571, 5643.16f, 2028.81f, 798.274f, 4.60242f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_icecrown_forgeofsouls:25|t Кузня Душ |cffff0000(80)|r", 0, 79, 0, 571, 5643.16f, 2028.81f, 798.274f, 4.60242f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_icecrown_hallsofreflection:25|t Залы Отражений |cffff0000(80)|r", 0, 79, 0, 571, 5643.16f, 2028.81f, 798.274f, 4.60242f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_nexus80_25man:25|t Подземелья Нексуса |cffff0000(71-80)|r", 0, 71, 0, 571, 3782.89f, 6965.23f, 105.088f, 6.14194f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_nexus70_heroic:25|t Нексус |cffff0000(71-73)|r", 0, 71, 0, 571, 3782.89f, 6965.23f, 105.088f, 6.14194f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_nexus80_heroic:25|t Окулус |cffff0000(80)|r", 0, 79, 0, 571, 3782.89f, 6965.23f, 105.088f, 6.14194f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_theviolethold_heroic:25|t Аметистовая крепость |cffff0000(75-77)|r", 0, 75, 0, 571, 5693.08f, 502.588f, 652.672f, 4.0229f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_ulduar80_heroic:25|t Чертоги Молний |cffff0000(80)|r", 0, 79, 0, 571, 9136.52f, -1311.81f, 1066.29f, 5.19113f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_ulduar77_heroic:25|t Чертоги Камня |cffff0000(77-79)|r", 0, 77, 0, 571, 8922.12f, -1009.16f, 1039.56f, 1.57044f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_utgardekeep_heroic:25|t Крепость Утгард |cffff0000(70-72)|r", 0, 69, 0, 571, 1203.41f, -4868.59f, 41.2486f, 0.283237f },
    { 4, 0, 2, "|TInterface/ICONS/achievement_dungeon_utgardepinnacle_heroic:25|t Вершина Утгард |cffff0000(80)|r", 0, 75, 0, 571, 1267.24f, -4857.3f, 215.764f, 3.22768f },
    { 4, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [Назад]", 0, 0, 0, 0, 0, 0, 0, 0 },

    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_illidan:25|t Черный храм", 0, 70, 0, 530, -3649.92f, 317.469f, 35.2827f, 2.94285f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_nefarion:25|t Логово Крыла Тьмы", 0, 60, 0, 229, 152.451f, -474.881f, 116.84f, 0.001073f },
    { 5, 0, 2, "|TInterface/ICONS/Achievement_Boss_Archimonde .png:25|t Битва за гору Хиджал", 0, 70, 0, 1, -8177.89f, -4181.23f, -167.552f, 0.913338f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_ladyvashj:25|t Змеиное святилище", 0, 70, 0, 530, 797.855f, 6865.77f, -65.4165f, 0.005938f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_reputation_argentchampion:25|t Испытание крестоносца", 0, 80, 0, 571, 8515.61f, 714.153f, 558.248f, 1.57753f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_gruulthedragonkiller:25|t Логово Груула", 0, 70, 0, 530, 3530.06f, 5104.08f, 3.50861f, 5.51117f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_magtheridon:25|t Логово Магтеридона", 0, 70, 0, 530, -336.411f, 3130.46f, -102.928f, 5.20322f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_dungeon_icecrown_icecrownentrance:25|t Цитадель Ледяной Короны", 0, 80, 0, 571, 5855.22f, 2102.03f, 635.991f, 3.57899f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_princemalchezaar_02:25|t Каражан", 0, 70, 0, 0, -11118.9f, -2010.33f, 47.0819f, 0.649895f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_ragnaros:25|t Огненные Недра", 0, 60, 0, 230, 1126.64f, -459.94f, -102.535f, 3.46095f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_dungeon_naxxramas_10man:25|t Наксрамас", 0, 80, 0, 571, 3668.72f, -1262.46f, 243.622f, 4.785f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_onyxia:25|t Логово Ониксии", 0, 80, 0, 1, -4708.27f, -3727.64f, 54.5589f, 3.72786f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_ossiriantheunscarred:25|t Руины Ан'Киража", 0, 60, 0, 1, -8409.82f, 1499.06f, 27.7179f, 2.51868f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_kiljaedan:25|t Плато Солнечного Колодца", 0, 70, 0, 530, 12574.1f, -6774.81f, 15.0904f, 3.13788f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_character_bloodelf_male:25|t Око", 0, 67, 0, 530, 3088.49f, 1381.57f, 184.863f, 4.61973f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_cthun:25|t Храм Ан'Киража", 0, 60, 0, 1, -8240.09f, 1991.32f, 129.072f, 0.941603f },
    { 5, 0, 2, "|TInterface/ICONS/Achievement_Boss_Malygos_01:25|t Око Вечности", 0, 80, 0, 571, 3784.17f, 7028.84f, 161.258f, 5.79993f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_dungeon_coablackdragonflight_10man:25|t Обсидиановое святилище", 0, 80, 0, 571, 3472.43f, 264.923f, -120.146f, 3.27923f },
    { 5, 0, 2, "|TInterface/ICONS/spell_shadow_twilight:25|t Рубиновое святилище", 0, 80, 0, 571, 3472.43f, 264.923f, -120.146f, 3.27923f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_dungeon_ulduarraid_misc_01:25|t Ульдуар", 0, 80, 0, 571, 9222.88f, -1113.59f, 1216.12f, 6.27549f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_kologarn_01:25|t Склеп Аркавона", 0, 80, 0, 571, 5453.72f, 2840.79f, 421.28f, 0.0f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_hakkar:25|t Зул'Гуруб", 0, 57, 0, 0, -11916.7f, -1215.72f, 92.289f, 4.72454f },
    { 5, 0, 2, "|TInterface/ICONS/achievement_boss_zuljin:25|t Зул'Аман", 0, 70, 1, 530, 6851.78f, -7972.57f, 179.242f, 4.64691f },
    { 5, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [Назад]", 0, 0, 0, 0, 0, 0, 0, 0 },

    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_elwynnforest:25|t Элвиннский Лес |cffff0000(1-10)|r", 0, 0, 2, 0, -9449.06f, 64.8392f, 56.3581f, 3.07047f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_eversongwoods:25|t Леса Вечной Песни |cffff0000(1-10)|r", 0, 0, 1, 530, 9024.37f, -6682.55f, 16.8973f, 3.14131f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_dunmorogh:25|t Дун Морог |cffff0000(1-10)|r", 0, 0, 2, 0, -5603.76f, -482.704f, 396.98f, 5.23499f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_tirisfalglades_01:25|t Тирисфальские Леса |cffff0000(1-10)|r", 0, 0, 1, 0, 2274.95f, 323.918f, 34.1137f, 4.24367f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_ghostlands:25|t Призрачные Земли |cffff0000(10-20)|r", 0, 10, 1, 530, 7595.73f, -6819.6f, 84.3718f, 2.56561f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_lochmodan:25|t Лок Модан |cffff0000(10-20)|r", 0, 10, 2, 0, -5405.85f, -2894.15f, 341.972f, 5.48238f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_silverpine_01:25|t Серебрянный Бор |cffff0000(10-20)|r", 0, 10, 1, 0, 505.126f, 1504.63f, 124.808f, 1.77987f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_westfall_01:25|t Западный край |cffff0000(10-20)|r", 0, 10, 2, 0, -10684.9f, 1033.63f, 32.5389f, 6.07384f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_redridgemountains:25|t Красногорье |cffff0000(15-25)|r", 0, 15, 2, 0, -9447.8f, -2270.85f, 71.8224f, 0.283853f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_duskwood:25|t Сумеречный лес |cffff0000(18-30)|r", 0, 18, 2, 0, -10531.7f, -1281.91f, 38.8647f, 1.56959f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_hillsbradfoothills:25|t Предгорья Хилсбрада |cffff0000(20-30)|r", 0, 20, 0, 0, -385.805f, -787.954f, 54.6655f, 1.03926f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_wetlands_01:25|t Болотина |cffff0000(20-30)|r", 0, 20, 2, 0, -3517.75f, -913.401f, 8.86625f, 2.60705f },
    { 6, 0, 2, "|TInterface/ICONS/Achievement_Zone_AlteracMountains_01:25|t Альтеракские горы |cffff0000(30-40)|r", 0, 30, 0, 0, 275.049f, -652.044f, 130.296f, 0.502032f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_arathihighlands_01:25|t Нагорье Арати |cffff0000(30-40)|r", 0, 30, 0, 0, -1581.45f, -2704.06f, 35.4168f, 0.490373f },
    { 6, 0, 2, "|TInterface/ICONS/Achievement_Zone_Stranglethorn_01:25|t Тернистая долина |cffff0000(30-45)|r", 0, 30, 0, 0, -11921.7f, -59.544f, 39.7262f, 3.73574f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_badlands_01:25|t Бесплодные земли |cffff0000(35-45)|r", 0, 35, 0, 0, -6782.56f, -3128.14f, 240.48f, 5.65912f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_swampsorrows_01:25|t Болото печали |cffff0000(35-45)|r", 0, 35, 0, 0, -10368.6f, -2731.3f, 21.6537f, 5.29238f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_hinterlands_01:25|t Внутренние земли |cffff0000(40-50)|r", 0, 40, 0, 0, 112.406f, -3929.74f, 136.358f, 0.981903f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_searinggorge_01:25|t Тлеющее ущелье |cffff0000(43-50)|r", 0, 43, 0, 0, -6686.33f, -1198.55f, 240.027f, 0.916887f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_blastedlands_01:25|t Выжженные земли |cffff0000(45-55)|r", 0, 45, 0, 0, -11184.7f, -3019.31f, 7.29238f, 3.20542f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_burningsteppes_01:25|t Пылающие степи |cffff0000(50-58)|r", 0, 50, 0, 0, -7979.78f, -2105.72f, 127.919f, 5.10148f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_westernplaguelands_01:25|t Западные Чумные земли |cffff0000(51-58)|r", 0, 51, 0, 0, 1743.69f, -1723.86f, 59.6648f, 5.23722f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_easternplaguelands:25|t Восточные Чумные земли |cffff0000(53-60)|r", 0, 53, 0, 0, 2280.64f, -5275.05f, 82.0166f, 4.7479f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_deadwindpass:25|t Перевал Мертвого Ветра |cffff0000(55-60)|r", 0, 55, 0, 0, -10438.8f, -1932.75f, 104.617f, 4.77402f },
    { 6, 0, 2, "|TInterface/ICONS/achievement_zone_isleofqueldanas:25|t Остров Кель'Данас |cffff0000(70)|r", 0, 70, 0, 530, 12806.5f, -6911.11f, 41.1156f, 2.22935f },
    { 6, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [Назад]", 0, 0, 0, 0, 0, 0, 0, 0 },

    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_azuremystisle_01:25|t Остров лазурной дымки |cffff0000(1-10)|r", 0, 0, 2, 530, -4192.62f, -12576.7f, 36.7598f, 1.62813f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_darnassus:25|t Тельдрассил |cffff0000(1-10)|r", 0, 0, 2, 1, 9889.03f, 915.869f, 1307.43f, 1.9336f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_durotar:25|t Дуротар |cffff0000(1-10)|r", 0, 0, 1, 1, 228.978f, -4741.87f, 10.1027f, 0.416883f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_mulgore_01:25|t Мулгор |cffff0000(1-10)|r", 0, 0, 1, 1, -2473.87f, -501.225f, -9.42465f, 0.6525f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_bloodmystisle_01:25|t Остров кровавой дымки |cffff0000(10-20)|r", 0, 15, 2, 530, -2095.7f, -11841.1f, 51.1557f, 6.19288f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_darkshore_01:25|t Темные берега |cffff0000(10-20)|r", 0, 10, 2, 1, 6463.25f, 683.986f, 8.92792f, 4.33534f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_barrens_01:25|t Степи |cffff0000(10-25)|r", 0, 10, 1, 1, -575.772f, -2652.45f, 95.6384f, 0.006469f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_stonetalon_01:25|t Когтистые горы |cffff0000(15-27)|r", 0, 15, 0, 1, 1574.89f, 1031.57f, 137.442f, 3.8013f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_ashenvale_01:25|t Ясеневый лес |cffff0000(18-30)|r", 0, 18, 0, 1, 1919.77f, -2169.68f, 94.6729f, 6.14177f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_thousandneedles_01:25|t Тысяча игл |cffff0000(25-35)|r", 0, 25, 1, 1, -5375.53f, -2509.2f, -40.432f, 2.41885f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_desolace:25|t Пустоши |cffff0000(30-40)|r", 0, 30, 0, 1, -656.056f, 1510.12f, 88.3746f, 3.29553f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_dustwallowmarsh:25|t Пылевые топи |cffff0000(35-45)|r", 0, 35, 0, 1, -3350.12f, -3064.85f, 33.0364f, 5.12666f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_feralas:25|t Фералас |cffff0000(40-50)|r", 0, 40, 0, 1, -4808.31f, 1040.51f, 103.769f, 2.90655f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_tanaris_01:25|t Танарис |cffff0000(40-50)|r", 0, 40, 0, 1, -6940.91f, -3725.7f, 48.9381f, 3.11174f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_azshara_01:25|t Азшара |cffff0000(45-55)|r", 0, 45, 0, 1, 3117.12f, -4387.97f, 91.9059f, 5.49897f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_felwood:25|t Оскверненный лес |cffff0000(48-55)|r", 0, 48, 0, 1, 3898.8f, -1283.33f, 220.519f, 6.24307f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_ungorocrater_01:25|t Кратер Ун'Горо |cffff0000(45-55)|r", 0, 48, 0, 1, -6291.55f, -1158.62f, -258.138f, 0.457099f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_silithus_01:25|t Силитус |cffff0000(55-60)|r", 0, 55, 0, 1, -6815.25f, 730.015f, 40.9483f, 2.39066f },
    { 7, 0, 2, "|TInterface/ICONS/achievement_zone_winterspring:25|t Зимние ключи |cffff0000(55-60)|r", 0, 55, 0, 1, 6658.57f, -4553.48f, 718.019f, 5.18088f },
    { 7, 0, 2, "|TInterface/ICONS/spell_arcane_teleportmoonglade:25|t Лунная поляна |cffff0000(1-80)|r", 0, 0, 0, 1, 7654.3f, -2232.87f, 462.107f, 5.96786f },
    { 7, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [Назад]", 0, 0, 0, 0, 0, 0, 0, 0 },

    { 8, 0, 2, "|TInterface/ICONS/achievement_zone_hellfirepeninsula_01:25|t Полуостров Адского пламени |cffff0000(58-63)|r", 0, 58, 0, 530, -207.335f, 2035.92f, 96.464f, 1.59676f },
    { 8, 0, 2, "|TInterface/ICONS/achievement_zone_zangarmarsh:25|t Зангартопь |cffff0000(60-64)|r", 0, 60, 0, 530, -220.297f, 5378.58f, 23.3223f, 1.61718f },
    { 8, 0, 2, "|TInterface/ICONS/achievement_zone_terrokar:25|t Лес Тероккар |cffff0000(62-65)|r", 0, 62, 0, 530, -2266.23f, 4244.73f, 1.47728f, 3.68426f },
    { 8, 0, 2, "|TInterface/ICONS/achievement_zone_nagrand_01:25|t Награнд |cffff0000(64-67)|r", 0, 64, 0, 530, -1610.85f, 7733.62f, -17.2773f, 1.33522f },
    { 8, 0, 2, "|TInterface/ICONS/achievement_zone_bladesedgemtns_01:25|t Острогорье |cffff0000(65-68)|r", 0, 65, 0, 530, 2029.75f, 6232.07f, 133.495f, 1.30395f },
    { 8, 0, 2, "|TInterface/ICONS/achievement_zone_netherstorm_01:25|t Пустоверть |cffff0000(67-70)|r", 0, 67, 0, 530, 3271.2f, 3811.61f, 143.153f, 3.44101f },
    { 8, 0, 2, "|TInterface/ICONS/achievement_zone_shadowmoon:25|t Долина призрачной луны |cffff0000(67-70)|r", 0, 67, 0, 530, -3681.01f, 2350.76f, 76.587f, 4.25995f },
    { 8, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [Назад]", 0, 0, 0, 0, 0, 0, 0, 0 },

    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_boreantundra_01:25|t Борейская тундра |cffff0000(68-72)|r", 0, 68, 0, 571, 2954.24f, 5379.13f, 60.4538f, 2.55544f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_howlingfjord_01:25|t Ревущий фьорд |cffff0000(68-72)|r", 0, 68, 0, 571, 682.848f, -3978.3f, 230.161f, 1.54207f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_dragonblight_01:25|t Драконий погост |cffff0000(71-74)|r", 0, 71, 0, 571, 2678.17f, 891.826f, 4.37494f, 0.101121f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_grizzlyhills_01:25|t Седые холмы |cffff0000(73-75)|r", 0, 73, 0, 571, 4017.35f, -3403.85f, 290.0f, 5.35431f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_zuldrak_03:25|t Зул'Драк |cffff0000(74-77)|r", 0, 74, 0, 571, 5560.23f, -3211.66f, 371.709f, 5.55055f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_sholazar_01:25|t Низина Шолазар |cffff0000(76-78)|r", 0, 76, 0, 571, 5614.67f, 5818.86f, -69.722f, 3.60807f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_crystalsong_01:25|t Хрустальная песня |cffff0000(77-80)|r", 0, 77, 0, 571, 5411.17f, -966.37f, 167.082f, 1.57167f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_stormpeaks_01:25|t Грозовая гряда |cffff0000(77-80)|r", 0, 77, 0, 571, 6120.46f, -1013.89f, 408.39f, 5.12322f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_icecrown_01:25|t Ледяная корона |cffff0000(77-80)|r", 0, 77, 0, 571, 8323.28f, 2763.5f, 655.093f, 2.87223f },
    { 9, 0, 2, "|TInterface/ICONS/achievement_zone_stormpeaks_03:25|t Озеро ледяных оков |cffff0000(PvP)|r", 0, 77, 0, 571, 4522.23f, 2828.01f, 389.975f, 0.215009f },
    { 9, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [Назад]", 0, 0, 0, 0, 0, 0, 0, 0 },

    { 10, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Джамп", 0, 80, 0, 169, -1641.657104f, -576.635803f, 118.556992f, 0.141738f },			
    { 10, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Бессмертные Души", 0, 80, 0, 1, -11808.95f, -4737.97f, 5.9716f, 3.47454f },
    { 10, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Акула", 0, 80, 0, 1, 5078.207520f, -6816.839355f, 4.579811f, 0.342080f },
    { 10, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Злая Панда", 0, 80, 0, 1, 4794.006348f, -6384.777344f, 107.372040f, 4.068978f },
    { 10, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Череп и Кости", 0, 80, 0, 1, 6139.717773f, -1293.815430f, 379.230225f, 5.263196f },
    { 10, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Чума", 0, 80, 0, 1, -7847.695801f, 695.177734f, -29.928991f, 4.018467f },
    { 10, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Ледяной Дракон", 0, 80, 0, 1, -5804.846191f, -3855.156006f, -88.647224f, 4.953506f },
    { 10, 0, 2, "|TInterface/ICONS/Achievement_Arena_2v2_7:25|t Тень Короля Лича", 0, 80, 0, 0, -3358.708008f, -3562.596191f, 53.180382f, 0.812192f },
    { 10, 1, 7, "|TInterface/PaperDollInfoFrame/UI-GearManager-Undo:25|t [Назад]", 0, 0, 0, 0, 0, 0, 0, 0 }

};

// TextIDs from npc_text
enum eEnums
{
    TEXT_MAIN_H = 300000,			//Horde main menu text               "За Орду!"
    TEXT_MAIN_A = 300001,			//Alliance main menu text            "За Альянс!"
    TEXT_DUNGEON = 300002,			//Dungeon teleport menu texts        "Какие подземелья вы хотите исследовать?"
    TEXT_RAID = 300003,			//Raid teleport menu text            "Будьте осторожны при выборе рейдового подземелья. Меня не будет рядом, когда вы начнете погибать."
    TEXT_AREA = 300004,			//Area teleport location menu texts  "Куда бы вы хотели отправиться?"

    TELEPORT_COUNT = sizeof Tele / sizeof(*Tele),
};

#define ARE_YOU_SURE	"Вы уверены, что вы хотите попасть в "
#define ERROR_COMBAT	"|cffff0000Вы в бою!|r"

bool Custom_FactCheck(uint32 Fact, unsigned char Key)
{
    bool Show = false;
    switch (Tele[Key].faction)
    {
    case 0:
        Show = true;
        break;
    case 1:
        if (Fact == HORDE)
        Show = true;
        break;
    case 2:
        if (Fact == ALLIANCE)
        Show = true;
        break;
    }
    return (Show);
}

uint32 Custom_GetText(unsigned int menu, Player* pPlayer)
{
    uint32 TEXT = TEXT_AREA;
    switch (menu)
    {
    case 0:
    case 1:
        switch (pPlayer->GetTeam())
        {
        case ALLIANCE:
            TEXT = TEXT_MAIN_A;
            break;
        case HORDE:
            TEXT = TEXT_MAIN_H;
            break;
        }
        break;
    case 2:
    case 3:
    case 4:
        TEXT = TEXT_DUNGEON;
        break;
    case 5:
        TEXT = TEXT_RAID;
        break;
    }
    return (TEXT);
}

void Custom_GetMenu(Player* pPlayer, Creature* pCreature, uint32 Key)
{
    bool ENDMENU = false;
    for (uint32 i = 0; i < TELEPORT_COUNT; i++)
    {
        if (ENDMENU && Tele[i].menu_id != Key)
        break;
        if (Tele[i].menu_id == Key && pPlayer->getLevel() >= Tele[i].level && Custom_FactCheck(pPlayer->GetTeam(), i))
        {
            if (Tele[i].next_menu_id != 0)
            pPlayer->ADD_GOSSIP_ITEM_EXTENDED(Tele[i].icon, Tele[i].name, GOSSIP_SENDER_MAIN, i, "", Tele[i].cost, false);
            else
            pPlayer->ADD_GOSSIP_ITEM_EXTENDED(Tele[i].icon, Tele[i].name, GOSSIP_SENDER_MAIN, i, ARE_YOU_SURE + Tele[i].name + " |r?", Tele[i].cost, false);
            ENDMENU = true;
        }
    }
    pPlayer->PlayerTalkClass->SendGossipMenu(Custom_GetText(Key, pPlayer), pCreature->GetGUID());
}

class npc_teleportation : public CreatureScript
{
public:
    npc_teleportation()
    : CreatureScript("npc_teleportation") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_teleportationAI(creature);
    }

    struct npc_teleportationAI : public ScriptedAI
    {
        npc_teleportationAI(Creature* creature) : ScriptedAI(creature)
        {
            me->CastSpell(me, 72523, true);
            SetEquipmentSlots(false, 40348);
        }
    };

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
        Custom_GetMenu(pPlayer, pCreature, 1);
        return true;
    }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
    {
        pPlayer->PlayerTalkClass->ClearMenus(); // Clear the menu

        pPlayer->ModifyMoney(-1 * Tele[uiAction].cost); // Take cash
        uint32 Key = Tele[uiAction].next_menu_id;
        if (Key == 0) // If no next_menu_id, then teleport to coords
        {
            if (!pPlayer->IsInCombat())
            {
                pPlayer->CLOSE_GOSSIP_MENU();
                pPlayer->TeleportTo(Tele[uiAction].map, Tele[uiAction].x, Tele[uiAction].y, Tele[uiAction].z, Tele[uiAction].o);
                return true;
            }
            pPlayer->GetSession()->SendAreaTriggerMessage(ERROR_COMBAT);
            Key = Tele[uiAction].menu_id;
        }

        Custom_GetMenu(pPlayer, pCreature, Key);
        return true;
    }
};

class npc_reagent : public CreatureScript
{
public:
    npc_reagent() : CreatureScript("npc_reagent") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_elemental_eternal_shadow:25:25:-15:0|tБытовые товары", GOSSIP_SENDER_MAIN, 1);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\trade_alchemy:25:25:-15:0|tРеагенты для Алхимии", GOSSIP_SENDER_MAIN, 3);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\trade_mining:25:25:-15:0|tРеагенты для Кузнечного дела", GOSSIP_SENDER_MAIN, 4);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_misc_armorkit_17:25:25:-15:0|tРеагенты для Кожевника", GOSSIP_SENDER_MAIN, 5);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\trade_engineering:25:25:-15:0|tРеагенты для Инженерии", GOSSIP_SENDER_MAIN, 7);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\trade_engraving:25:25:-15:0|tРеагенты для Наложения чар", GOSSIP_SENDER_MAIN, 8);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_inscription_tradeskill01:25:25:-15:0|tРеагенты для Начертания", GOSSIP_SENDER_MAIN, 9);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\trade_tailoring:25:25:-15:0|tРеагенты для Портняжного дела", GOSSIP_SENDER_MAIN, 10);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_misc_herb_icethorn:25:25:-15:0|tРеагенты для Кулинарии", GOSSIP_SENDER_MAIN, 11);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_misc_gem_01:25:25:-15:0|tРеагенты для Ювелира", GOSSIP_SENDER_MAIN, 12);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_elemental_eternal_shadow:25:25:-15:0|tРазные Товары", GOSSIP_SENDER_MAIN, 13);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_hammer_20:25:25:-15:0|tУстройства | Приспособления", GOSSIP_SENDER_MAIN, 6);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\Spell_chargenegative:25:25:-15:0|tЗакрыть", GOSSIP_SENDER_MAIN, 100);
        player->PlayerTalkClass->SendGossipMenu(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player *player, Creature *creature, uint32 sender, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {

        case 100:
            player->PlayerTalkClass->SendCloseGossip();
            break;
        case 1:
            player->GetSession()->SendListInventory(creature->GetGUID(), 190015);
            break;
        case 2:
            player->GetSession()->SendListInventory(creature->GetGUID(), 190016);
            break;
        case 3:
            player->GetSession()->SendListInventory(creature->GetGUID(), 190017);
            break;
        case 4:
            player->GetSession()->SendListInventory(creature->GetGUID(), 190018);
            break;
        case 5:
            player->GetSession()->SendListInventory(creature->GetGUID(), 190019);
            break;
        case 6:
            player->GetSession()->SendListInventory(creature->GetGUID(), 190020);
            break;
        case 7:
            player->GetSession()->SendListInventory(creature->GetGUID(), 190021);
            break;
        case 8:
            player->GetSession()->SendListInventory(creature->GetGUID(), 190022);
            break;
        case 9:
            player->GetSession()->SendListInventory(creature->GetGUID(), 190023);
            break;
        case 10:
            player->GetSession()->SendListInventory(creature->GetGUID(), 190024);
            break;
        case 11:
            player->GetSession()->SendListInventory(creature->GetGUID(), 190025);
            break;
        case 12:
            player->GetSession()->SendListInventory(creature->GetGUID(), 190026);
            break;
        case 13:
            player->GetSession()->SendListInventory(creature->GetGUID(), 190027);
            break;
        }
        return true;
    }
};
class npc_clk : public CreatureScript
{
public:
    npc_clk() : CreatureScript("npc_clk") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_chest_plate_26:25:25:-15:0|tГрудь", GOSSIP_SENDER_MAIN, 3000);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_helmet_158:25:25:-15:0|tГолова", GOSSIP_SENDER_MAIN, 3002);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_pants_plate_31:25:25:-15:0|tНоги", GOSSIP_SENDER_MAIN, 3003);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_boots_leather_07:25:25:-15:0|tСтупни", GOSSIP_SENDER_MAIN, 3004);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_gauntlets_85:25:25:-15:0|tПерчатки", GOSSIP_SENDER_MAIN, 3005);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_belt_70:25:25:-15:0|tПояс", GOSSIP_SENDER_MAIN, 3006);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_jewelry_ring_86:25:25:-15:0|tКольца", GOSSIP_SENDER_MAIN, 3007);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\item_icecrownnecklaced:25:25:-15:0|tОжерелья", GOSSIP_SENDER_MAIN, 3008);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_jewelry_trinket_04:25:25:-15:0|tТринкеты", GOSSIP_SENDER_MAIN, 3009);
        //player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_axe_113:25:25:-15:0|tОружие", GOSSIP_SENDER_MAIN, 3010);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_bracer_41:25:25:-15:0|tЗапястья", GOSSIP_SENDER_MAIN, 3011);
		player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_shoulder_113:25:25:-15:0|tПлечи", GOSSIP_SENDER_MAIN, 3047);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\Spell_chargenegative:25:25:-15:0|tЗакрыть", GOSSIP_SENDER_MAIN, 100);
        player->PlayerTalkClass->SendGossipMenu(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player *player, Creature *creature, uint32 sender, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {

        case 100:
            player->PlayerTalkClass->SendCloseGossip();
            break;
        case 3000:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400043);
            break;
        case 3002:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400044);
            break;
        case 3003:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400045);
            break;
        case 3004:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400046);
            break;
        case 3005:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400047);
            break;
        case 3006:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400048);
            break;
        case 3007:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400049);
            break;
        case 3008:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400050);
            break;
        case 3009:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400051);
            break;
        case 3010:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400052);
            break;
        case 3011:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400053);
            break;
		case 3047:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400092);
            break;
        }
        return true;
    }
};
class npc_rubik : public CreatureScript
{
public:
    npc_rubik() : CreatureScript("npc_rubik") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_chest_plate_26:25:25:-15:0|tГрудь", GOSSIP_SENDER_MAIN, 3049);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_helmet_158:25:25:-15:0|tГолова", GOSSIP_SENDER_MAIN, 3050);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_pants_plate_31:25:25:-15:0|tНоги", GOSSIP_SENDER_MAIN, 3051);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_boots_leather_07:25:25:-15:0|tСтупни", GOSSIP_SENDER_MAIN, 3052);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_gauntlets_85:25:25:-15:0|tПерчатки", GOSSIP_SENDER_MAIN, 3053);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_belt_70:25:25:-15:0|tПояс", GOSSIP_SENDER_MAIN, 3054);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_jewelry_ring_86:25:25:-15:0|tКольца", GOSSIP_SENDER_MAIN, 3055);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\item_icecrownnecklaced:25:25:-15:0|tОжерелья", GOSSIP_SENDER_MAIN, 3056);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_jewelry_trinket_04:25:25:-15:0|tТринкеты", GOSSIP_SENDER_MAIN, 3057);
        //player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_axe_113:25:25:-15:0|tОружие", GOSSIP_SENDER_MAIN, 3010);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_bracer_41:25:25:-15:0|tЗапястья", GOSSIP_SENDER_MAIN, 3058);
		player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_shoulder_113:25:25:-15:0|tПлечи", GOSSIP_SENDER_MAIN, 3059);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\Spell_chargenegative:25:25:-15:0|tЗакрыть", GOSSIP_SENDER_MAIN, 100);
        player->PlayerTalkClass->SendGossipMenu(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player *player, Creature *creature, uint32 sender, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {

        case 100:
            player->PlayerTalkClass->SendCloseGossip();
            break;
        case 3049:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400094);
            break;
        case 3050:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400095);
            break;
        case 3051:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400096);
            break;
        case 3052:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400097);
            break;
        case 3053:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400098);
            break;
        case 3054:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400099);
            break;
        case 3055:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400100);
            break;
        case 3056:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400101);
            break;
        case 3057:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400102);
            break;
        case 3058:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400103);
            break;
        case 3059:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400104);
            break;
        }
        return true;
    }
};
class npc_xazam : public CreatureScript
{
public:
    npc_xazam() : CreatureScript("npc_xazam") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_chest_plate_26:25:25:-15:0|tГрудь", GOSSIP_SENDER_MAIN, 3012);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_helmet_158:25:25:-15:0|tГолова", GOSSIP_SENDER_MAIN, 3013);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_pants_plate_31:25:25:-15:0|tНоги", GOSSIP_SENDER_MAIN, 3014);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_boots_leather_07:25:25:-15:0|tСтупни", GOSSIP_SENDER_MAIN, 3015);
		player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_shoulder_113:25:25:-15:0|tПлечи", GOSSIP_SENDER_MAIN, 3048);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_gauntlets_85:25:25:-15:0|tПерчатки", GOSSIP_SENDER_MAIN, 3016);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_belt_70:25:25:-15:0|tПояс", GOSSIP_SENDER_MAIN, 3017);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_jewelry_ring_86:25:25:-15:0|tКольца", GOSSIP_SENDER_MAIN, 3018);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\item_icecrownnecklaced:25:25:-15:0|tОжерелья", GOSSIP_SENDER_MAIN, 3019);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_jewelry_trinket_04:25:25:-15:0|tТринкеты", GOSSIP_SENDER_MAIN, 3020);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_bracer_41:25:25:-15:0|tЗапястья", GOSSIP_SENDER_MAIN, 3022);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\Spell_chargenegative:25:25:-15:0|tЗакрыть", GOSSIP_SENDER_MAIN, 100);
        player->PlayerTalkClass->SendGossipMenu(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player *player, Creature *creature, uint32 sender, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {

        case 100:
            player->PlayerTalkClass->SendCloseGossip();
            break;
        case 3012:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400054);
            break;
        case 3013:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400055);
            break;
        case 3014:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400056);
            break;
        case 3015:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400057);
            break;
        case 3016:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400058);
            break;
        case 3017:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400059);
            break;
        case 3018:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400060);
            break;
        case 3019:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400061);
            break;
        case 3020:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400062);
            break;
        case 3022:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400063);
            break;
		case 3048:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400093);
            break;
        }
        return true;
    }
};
class npc_puhi : public CreatureScript
{
public:
    npc_puhi() : CreatureScript("npc_puhi") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_weapon_shortblade_106:25:25:-15:0|tКистевое", GOSSIP_SENDER_MAIN, 3023);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_weapon_shortblade_101:25:25:-15:0|tКинжал", GOSSIP_SENDER_MAIN, 3024);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_sword_154:25:25:-15:0|tОдноручный меч", GOSSIP_SENDER_MAIN, 3025);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_sword_153:25:25:-15:0|tДвуручный меч", GOSSIP_SENDER_MAIN, 3026);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_axe_113:25:25:-15:0|tОдноручный топор", GOSSIP_SENDER_MAIN, 3027);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_axe_120:25:25:-15:0|tДвуручный топор", GOSSIP_SENDER_MAIN, 3028);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_mace_115:25:25:-15:0|tДробящее", GOSSIP_SENDER_MAIN, 3029);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_weapon_staff_109:25:25:-15:0|tДревковое", GOSSIP_SENDER_MAIN, 3030);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_staff_109:25:25:-15:0|tПосох", GOSSIP_SENDER_MAIN, 3031);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_weapon_bow_55:25:25:-15:0|tЛук/Арбалет/Огнестрел", GOSSIP_SENDER_MAIN, 3032);
		player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_shield_72:25:25:-15:0|tЩиты", GOSSIP_SENDER_MAIN, 3033);
		player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_misc_thread_01:25:25:-15:0|tЛевая рука", GOSSIP_SENDER_MAIN, 3034);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\Spell_chargenegative:25:25:-15:0|tЗакрыть", GOSSIP_SENDER_MAIN, 100);
        player->PlayerTalkClass->SendGossipMenu(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player *player, Creature *creature, uint32 sender, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {

        case 100:
            player->PlayerTalkClass->SendCloseGossip();
            break;
        case 3023:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400066);
            break;
        case 3024:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400067);
            break;
        case 3025:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400068);
            break;
        case 3026:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400069);
            break;
        case 3027:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400070);
            break;
        case 3028:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400071);
            break;
        case 3029:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400072);
            break;
        case 3030:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400073);
            break;
        case 3031:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400074);
            break;
        case 3032:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400075);
            break;
		case 3033:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400076);
            break;
        case 3034:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400077);
            break;
        }
        return true;
    }
};
class npc_puhipve : public CreatureScript
{
public:
    npc_puhipve() : CreatureScript("npc_puhipve") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_weapon_shortblade_106:25:25:-15:0|tКистевое", GOSSIP_SENDER_MAIN, 3035);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_weapon_shortblade_101:25:25:-15:0|tКинжал", GOSSIP_SENDER_MAIN, 3036);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_sword_154:25:25:-15:0|tОдноручный меч", GOSSIP_SENDER_MAIN, 3037);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_sword_153:25:25:-15:0|tДвуручный меч", GOSSIP_SENDER_MAIN, 3038);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_axe_113:25:25:-15:0|tОдноручный топор", GOSSIP_SENDER_MAIN, 3039);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_axe_120:25:25:-15:0|tДвуручный топор", GOSSIP_SENDER_MAIN, 3040);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_mace_115:25:25:-15:0|tДробящее", GOSSIP_SENDER_MAIN, 3041);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_weapon_staff_109:25:25:-15:0|tДревковое", GOSSIP_SENDER_MAIN, 3042);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_staff_109:25:25:-15:0|tПосох", GOSSIP_SENDER_MAIN, 3043);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_weapon_bow_55:25:25:-15:0|tЛук/Арбалет/Огнестрел", GOSSIP_SENDER_MAIN, 3044);
		player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_shield_72:25:25:-15:0|tЩиты", GOSSIP_SENDER_MAIN, 3045);
		player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\inv_misc_thread_01:25:25:-15:0|tЛевая рука", GOSSIP_SENDER_MAIN, 3046);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface\\icons\\Spell_chargenegative:25:25:-15:0|tЗакрыть", GOSSIP_SENDER_MAIN, 100);
        player->PlayerTalkClass->SendGossipMenu(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player *player, Creature *creature, uint32 sender, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {

        case 100:
            player->PlayerTalkClass->SendCloseGossip();
            break;
        case 3035:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400079);
            break;
        case 3036:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400080);
            break;
        case 3037:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400081);
            break;
        case 3038:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400082);
            break;
        case 3039:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400083);
            break;
        case 3040:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400084);
            break;
        case 3041:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400085);
            break;
        case 3042:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400086);
            break;
        case 3043:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400087);
            break;
        case 3044:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400088);
            break;
		case 3045:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400089);
            break;
        case 3046:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400090);
            break;
        }
        return true;
    }
};

enum Enchants
{
    ENCHANT_WEP_BERSERKING = 3789,
    ENCHANT_WEP_BLADE_WARD = 3869,
    ENCHANT_WEP_BLOOD_DRAINING = 3870,
    ENCHANT_WEP_ACCURACY = 3788,
    ENCHANT_WEP_AGILITY_1H = 1103,
    ENCHANT_WEP_SPIRIT = 3844,
    ENCHANT_WEP_BATTLEMASTER = 2675,
    ENCHANT_WEP_BLACK_MAGIC = 3790,
    ENCHANT_WEP_ICEBREAKER = 3239,
    ENCHANT_WEP_LIFEWARD = 3247,
    ENCHANT_WEP_MIGHTY_SPELL_POWER = 3834, // One-hand
    ENCHANT_WEP_EXECUTIONER = 3225,
    ENCHANT_WEP_STRYJA = 1894,
    ENCHANT_WEP_POTENCY = 3833,
    ENCHANT_WEP_TITANGUARD = 3851,
    ENCHANT_2WEP_MASSACRE = 3827,
    ENCHANT_2WEP_SCOURGEBANE = 3247,
    ENCHANT_2WEP_GIANT_SLAYER = 3257,
    ENCHANT_2WEP_GREATER_SPELL_POWER = 3854,
    ENCHANT_2WEP_AGILITY = 2670,
    ENCHANT_2WEP_MONGOOSE = 2673,

    ENCHANT_SHIELD_DEFENSE = 1952,
    ENCHANT_SHIELD_INTELLECT = 1128,
    ENCHANT_SHIELD_RESILIENCE = 3229,
    ENCHANT_SHIELD_BLOCK = 2655,
    ENCHANT_SHIELD_STAMINA = 1077,
    ENCHANT_SHIELD_TOUGHSHIELD = 2653,
    ENCHANT_SHIELD_TITANIUM_PLATING = 3849,

    ENCHANT_HEAD_BLISSFUL_MENDING = 3819,
    ENCHANT_HEAD_BURNING_MYSTERIES = 3820,
    ENCHANT_HEAD_DOMINANCE = 3796,
    ENCHANT_HEAD_SAVAGE_GLADIATOR = 3842,
    ENCHANT_HEAD_STALWART_PROTECTOR = 3818,
    ENCHANT_HEAD_TORMENT = 3817,
    ENCHANT_HEAD_TRIUMPH = 3795,
    ENCHANT_HEAD_ECLIPSED_MOON = 3815,
    ENCHANT_HEAD_FLAME_SOUL = 3816,
    ENCHANT_HEAD_FLEEING_SHADOW = 3814,
    ENCHANT_HEAD_FROSTY_SOUL = 3812,
    ENCHANT_HEAD_TOXIC_WARDING = 3813,

    ENCHANT_SHOULDER_MASTERS_AXE = 3835,
    ENCHANT_SHOULDER_MASTERS_CRAG = 3836,
    ENCHANT_SHOULDER_KRIT = 3838,
    ENCHANT_SHOULDER_MASTERS_PINNACLE = 3837,
    ENCHANT_SHOULDER_MASTERS_STORM = 3813,
    ENCHANT_SHOULDER_GREATER_AXE = 3808,
    ENCHANT_SHOULDER_GREATER_CRAG = 3809,
    ENCHANT_SHOULDER_GREATER_GLADIATOR = 3852,
    ENCHANT_SHOULDER_GREATER_PINNACLE = 3811,
    ENCHANT_SHOULDER_GREATER_STORM = 3810,
    ENCHANT_SHOULDER_DOMINANCE = 3794,
    ENCHANT_SHOULDER_TRIUMPH = 3793,

    ENCHANT_CLOAK_DARKGLOW_EMBROIDERY = 3728,
    ENCHANT_CLOAK_SWORDGUARD_EMBROIDERY = 3730,
    ENCHANT_CLOAK_LIGHTWEAVE_EMBROIDERY = 3722,
    ENCHANT_CLOAK_SPRINGY_ARACHNOWEAVE = 3859,
    ENCHANT_CLOAK_WISDOM = 3296,
    ENCHANT_CLOAK_TITANWEAVE = 1957,
    ENCHANT_CLOAK_SPELL_PIERCING = 3243,
    ENCHANT_CLOAK_SHADOW_ARMOR = 3256,
    ENCHANT_CLOAK_MIGHTY_ARMOR = 3294,
    ENCHANT_CLOAK_MAJOR_AGILITY = 1099,
    ENCHANT_CLOAK_GREATER_SPEED = 3831,

    ENCHANT_LEG_EARTHEN = 3853,
    ENCHANT_LEG_FROSTHIDE = 3822,
    ENCHANT_LEG_ICESCALE = 3823,
    ENCHANT_LEG_BRILLIANT_SPELLTHREAD = 3719,
    ENCHANT_LEG_SAPPHIRE_SPELLTHREAD = 3721,
    ENCHANT_LEG_DRAGONSCALE = 3331,
    ENCHANT_LEG_WYRMSCALE = 3332,

    ENCHANT_GLOVES_GREATER_BLASTING = 3249,
    ENCHANT_GLOVES_ARMSMAN = 3253,
    ENCHANT_GLOVES_CRUSHER = 1603,
    ENCHANT_GLOVES_AGILITY = 3222,
    ENCHANT_GLOVES_PRECISION = 3234,
    ENCHANT_GLOVES_SPD = 3246,
    ENCHANT_GLOVES_EXPERTISE = 3237,
    ENCHANT_GLOVES_HAST = 3604,
    ENCHANT_GLOVES_RAKETA = 3603,

    ENCHANT_BRACERS_MAJOR_STAMINA = 3850,
    ENCHANT_BRACERS_SUPERIOR_SP = 2332,
    ENCHANT_BRACERS_GREATER_ASSUALT = 3845,
    ENCHANT_BRACERS_MAJOR_SPIRT = 1147,
    ENCHANT_BRACERS_EXPERTISE = 3231,
    ENCHANT_BRACERS_INTELLECT = 1119,
    ENCHANT_BRACERS_FURL_ARCANE = 3763,
    ENCHANT_BRACERS_FURL_FIRE = 3759,
    ENCHANT_BRACERS_FURL_FROST = 3760,
    ENCHANT_BRACERS_FURL_NATURE = 3762,
    ENCHANT_BRACERS_FURL_SHADOW = 3761,
    ENCHANT_BRACERS_FURL_ATTACK = 3756,
    ENCHANT_BRACERS_FURL_STAMINA = 3757,
    ENCHANT_BRACERS_FURL_SPELLPOWER = 3758,

    ENCHANT_CHEST_POWERFUL_STATS = 3832,
    ENCHANT_CHEST_SUPER_HEALTH = 3297,
    ENCHANT_CHEST_GREATER_MAINA_REST = 2387,
    ENCHANT_CHEST_EXCEPTIONAL_RESIL = 3245,
    ENCHANT_CHEST_GREATER_DEFENSE = 1953,

    ENCHANT_BOOTS_GREATER_ASSULT = 1597,
    ENCHANT_BOOTS_TUSKARS_VITLIATY = 3232,
    ENCHANT_BOOTS_SUPERIOR_AGILITY = 983,
    ENCHANT_BOOTS_GREATER_SPIRIT = 1147,
    ENCHANT_BOOTS_GREATER_VITALITY = 3244,
    ENCHANT_BOOTS_ICEWALKER = 3826,
    ENCHANT_BOOTS_GREATER_FORTITUDE = 1075,
    ENCHANT_BOOTS_NITRO_BOOTS = 3606,

    ENCHANT_RING_ASSULT = 3839,
    ENCHANT_RING_GREATER_SP = 3840,
    ENCHANT_RING_STAMINA = 3791,

    ENCHANT_VZRIV = 3599,
    ENCHANT_BOMBA = 3601,
    ENCHANT_WAIST_SOCKET = 3729,
    ENCHANT_KISTIRYK_SOCKET = 3723, 
    ENCHANT_ZAPYASTIE_SOCKET = 3717,
    
    ENCHANT_SPINA = 3859, // Пружинистая паутинка
    ENCHANT_SPINADVA = 3605 // Эластичная подкладка
    
};


void AdvancedEnchant(Player* player, Item* item, EnchantmentSlot slot, uint32 socketGem)
{
    if (!item)
    {
        player->GetSession()->SendNotification("Вы должны одеть предмет для зачарование!");
        return;
    }

    if (!socketGem)
    {
        player->GetSession()->SendNotification("Something went wrong in the code. It has been logged for developers and will be looked into, sorry for the inconvenience.");
        return;
    }

    player->ApplyEnchantment(item, slot, false);
    item->SetEnchantment(slot, socketGem, 0, 0);
    player->ApplyEnchantment(item, slot, true);
    player->GetSession()->SendNotification("|cff800080%s |cffFF0000 успешно произошло", item->GetTemplate()->Name1.c_str());
}

void Enchant(Player* player, Item* item, uint32 enchantid)
{
    if (!item)
    {
        player->GetSession()->SendNotification("Вы должны одеть предмет для зачарование!");
        return;
    }

    if (!enchantid)
    {
        player->GetSession()->SendNotification("Something went wrong in the code. It has been logged for developers and will be looked into, sorry for the inconvenience.");
        //sLog->("enchant_vendor::Enchant: Enchant NPC 'enchantid' is NULL, something went wrong here!");
        return;
    }

    player->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, false);
    item->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchantid, 0, 0);
    player->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, true);
    player->GetSession()->SendNotification("|cff800080%s |cffFF0000 успешно произошло!", item->GetTemplate()->Name1.c_str());
}

void RemoveEnchant(Player* player, Item* item)
{
    if (!item)
    {
        player->GetSession()->SendNotification("У вас нету предмета");
        return;
    }

    item->ClearEnchantment(PERM_ENCHANTMENT_SLOT);
    player->GetSession()->SendNotification("|cff800080%s |cffFF0000 успешно снята!", item->GetTemplate()->Name1.c_str());
}

class npc_enchantment : public CreatureScript
{
public:
    npc_enchantment() : CreatureScript("npc_enchantment") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->ADD_GOSSIP_ITEM(7, "|TInterface\\icons\\Inv_misc_dust_infinite:20:20:-15:0|t[Наложение чар]", GOSSIP_SENDER_MAIN, 1001);
        player->ADD_GOSSIP_ITEM(7, "|TInterface\\icons\\Inv_jewelcrafting_starofelune_01:20:20:-15:0|t[Сокеты]", GOSSIP_SENDER_MAIN, 1002);
        player->ADD_GOSSIP_ITEM(7, "|TInterface\\icons\\Inv_inscription_armorscroll01:20:20:-15:0|t[Символы]", GOSSIP_SENDER_MAIN, 1003);
        player->PlayerTalkClass->SendGossipMenu(100007, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        Item * item;

        switch (action)
        {
        case 1002:
            // Сокеты
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Inv_jewelcrafting_shadowspirit_01:20:20:-15:0|t[Особые]", GOSSIP_SENDER_MAIN, 1101);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Inv_jewelcrafting_gem_38:20:20:-15:0|t[Желтые]", GOSSIP_SENDER_MAIN, 1102);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Inv_jewelcrafting_gem_37:20:20:-15:0|t[Красные]", GOSSIP_SENDER_MAIN, 1103);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Inv_jewelcrafting_gem_42:20:20:-15:0|t[Синие]", GOSSIP_SENDER_MAIN, 1104);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Inv_jewelcrafting_gem_40:20:20:-15:0|t[Фиолетовые]", GOSSIP_SENDER_MAIN, 1105);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Inv_jewelcrafting_gem_39:20:20:-15:0|t[Оранжевые]", GOSSIP_SENDER_MAIN, 1106);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Inv_jewelcrafting_gem_41:20:20:-15:0|t[Зеленые]", GOSSIP_SENDER_MAIN, 1107);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Inv_jewelcrafting_dragonseye02:20:20:-15:0|t[Радужные]", GOSSIP_SENDER_MAIN, 1108);
            player->PlayerTalkClass->SendGossipMenu(100007, creature->GetGUID());
            return true;
            break;
        case 1101:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400021);
            break;
        case 1102:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400023);
            break;
        case 1103:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400024);
            break;
        case 1104:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400025);
            break;
        case 1105:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400026);
            break;
        case 1106:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400027);
            break;
        case 1107:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400028);
            break;
        case 1108:
            player->GetSession()->SendListInventory(creature->GetGUID(), 400029);
            break;
        case 1003:
            // Символы
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_holy_avenginewrath:20:20:-15:0|t[Паладин]", GOSSIP_SENDER_MAIN, 1121);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_nature_thunderclap:20:20:-15:0|t[Воин]", GOSSIP_SENDER_MAIN, 1122);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_shadow_painspike:20:20:-15:0|t[Охотник]", GOSSIP_SENDER_MAIN, 1123);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_shadow_nethercloak:20:20:-15:0|t[Разбойник]", GOSSIP_SENDER_MAIN, 1124);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_holy_prayerofshadowprotection:20:20:-15:0|t[Жрец]", GOSSIP_SENDER_MAIN, 1125);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_deathknight_icetouch:20:20:-15:0|t[Рыцарь смерти]", GOSSIP_SENDER_MAIN, 1126);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_nature_earthshock:20:20:-15:0|t[Шаман]", GOSSIP_SENDER_MAIN, 1127);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_nature_polymorph:20:20:-15:0|t[Маг]", GOSSIP_SENDER_MAIN, 1128);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_shadow_demoniccircleteleport:20:20:-15:0|t[Чернокнижник]", GOSSIP_SENDER_MAIN, 1129);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_nature_forceofnature:20:20:-15:0|t[Друид]", GOSSIP_SENDER_MAIN, 1130);
            player->PlayerTalkClass->SendGossipMenu(100007, creature->GetGUID());
            return true;
            break;
        case 1121:
            player->GetSession()->SendListInventory(creature->GetGUID(), 250021);
            break;
        case 1122:
            player->GetSession()->SendListInventory(creature->GetGUID(), 250022);
            break;
        case 1123:
            player->GetSession()->SendListInventory(creature->GetGUID(), 250023);
            break;
        case 1124:
            player->GetSession()->SendListInventory(creature->GetGUID(), 250024);
            break;
        case 1125:
            player->GetSession()->SendListInventory(creature->GetGUID(), 250025);
            break;
        case 1126:
            player->GetSession()->SendListInventory(creature->GetGUID(), 250026);
            break;
        case 1127:
            player->GetSession()->SendListInventory(creature->GetGUID(), 250027);
            break;
        case 1128:
            player->GetSession()->SendListInventory(creature->GetGUID(), 250028);
            break;
        case 1129:
            player->GetSession()->SendListInventory(creature->GetGUID(), 250029);
            break;
        case 1130:
            player->GetSession()->SendListInventory(creature->GetGUID(), 250030);
            break;

        case 1001:
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_helmet_164:20:20:-15:0|t[Зачарить: Голову]", GOSSIP_SENDER_MAIN, 4);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_shoulder_126:20:20:-15:0|t[Зачарить: Плечи]", GOSSIP_SENDER_MAIN, 5);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_chest_plate26:20:20:-15:0|t[Зачарить: Грудь]", GOSSIP_SENDER_MAIN, 7);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_pants_plate_37:20:20:-15:0|t[Зачарить: Ноги]", GOSSIP_SENDER_MAIN, 10);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_gauntlets_91:20:20:-15:0|t[Зачарить: Кисти рук]", GOSSIP_SENDER_MAIN, 9);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_bracer_43:20:20:-15:0|t[Зачарить: Запястья]", GOSSIP_SENDER_MAIN, 8);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_belt_48c:20:20:-15:0|t[Зачарить: Пояс]", GOSSIP_SENDER_MAIN, 15);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_boots_plate_13:20:20:-15:0|t[Зачарить: Ступни]", GOSSIP_SENDER_MAIN, 11);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_misc_cape_18:20:20:-15:0|t[Зачарить: Спину]", GOSSIP_SENDER_MAIN, 6);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_axe_113:20:20:-15:0|t[Зачарить: Оружие]", GOSSIP_SENDER_MAIN, 1);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_axe_116:20:20:-15:0|t[Зачарить: Оружие в левой руке]", GOSSIP_SENDER_MAIN, 13);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_axe_115:20:20:-15:0|t[Зачарить: Двуручное оружие]", GOSSIP_SENDER_MAIN, 2);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_shield_75:20:20:-15:0|t[Зачарить: Щит]", GOSSIP_SENDER_MAIN, 3);

            if (player->HasSkill(SKILL_ENCHANTING) && player->GetSkillValue(SKILL_ENCHANTING) == 450)
            {
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_jewelry_ring_83:20:20:-15:0|t[Зачарить: Кольца]", GOSSIP_SENDER_MAIN, 12);
            }

            player->PlayerTalkClass->SendGossipMenu(100007, creature->GetGUID());
            return true;

            break;

        case 1: // Enchant Weapon

            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tОтведение удара", GOSSIP_SENDER_MAIN, 102);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tВытягивание Крови", GOSSIP_SENDER_MAIN, 103);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+26 к ловкости", GOSSIP_SENDER_MAIN, 100);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+45 к духу", GOSSIP_SENDER_MAIN, 101);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tБоевое исступление", GOSSIP_SENDER_MAIN, 104);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 к рейтингу меткости, +25 к рейтингу критического удара", GOSSIP_SENDER_MAIN, 105);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tЧерная магия", GOSSIP_SENDER_MAIN, 106);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tВоеначальник", GOSSIP_SENDER_MAIN, 107);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tОружие Ледолома", GOSSIP_SENDER_MAIN, 108);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tОберег", GOSSIP_SENDER_MAIN, 109);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+50 к выносливости", GOSSIP_SENDER_MAIN, 110);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+65 к силе атаки", GOSSIP_SENDER_MAIN, 111);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+63 к силе заклинаний", GOSSIP_SENDER_MAIN, 112);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tМангуст", GOSSIP_SENDER_MAIN, 113);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tПалач", GOSSIP_SENDER_MAIN, 114);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tЛеденящая стужа", GOSSIP_SENDER_MAIN, 701);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tНазад", GOSSIP_SENDER_MAIN, 300);
            player->PlayerTalkClass->SendGossipMenu(100002, creature->GetGUID());
            return true;
            break;
        case 2: // Enchant 2H Weapon
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                if (!item)
                {
                    player->GetSession()->SendNotification("Вы должны оборудовать двуручным оружием.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
                {
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tБоевое исступление", GOSSIP_SENDER_MAIN, 104);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+81 к силе заклинаний", GOSSIP_SENDER_MAIN, 115);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+35 к ловкости", GOSSIP_SENDER_MAIN, 116);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+110 к силе атаки", GOSSIP_SENDER_MAIN, 117);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tМангуст", GOSSIP_SENDER_MAIN, 113);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tПалач", GOSSIP_SENDER_MAIN, 114);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tЛеденящая стужа", GOSSIP_SENDER_MAIN, 701);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tНазад", GOSSIP_SENDER_MAIN, 300);
                    player->PlayerTalkClass->SendGossipMenu(100003, creature->GetGUID());
                    return true;
                }
                else
                {
                    player->GetSession()->SendNotification("Вы должны двуручным оружием оборудованным.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 3: // Enchant Shield
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendNotification("Вы должны оборудовать щитом.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_SHIELD)
                {
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+20 к рейтингу защиты", GOSSIP_SENDER_MAIN, 118);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 к интелекту", GOSSIP_SENDER_MAIN, 119);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+12 к рейтингу устойчивости", GOSSIP_SENDER_MAIN, 120);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tТитановая обшивка", GOSSIP_SENDER_MAIN, 121);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+18 к выносливости", GOSSIP_SENDER_MAIN, 122);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+36 к показателю блокирования", GOSSIP_SENDER_MAIN, 123);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tНазад", GOSSIP_SENDER_MAIN, 300);
                    player->PlayerTalkClass->SendGossipMenu(100004, creature->GetGUID());
                    return true;
                }
                else
                {
                    player->GetSession()->SendNotification("У вас нету щита.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 4: // Enchant Head
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+30 к силе заклинаний и + 10 ед.маны каждые 5 секунд", GOSSIP_SENDER_MAIN, 124);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+30 к силе заклинаний и 20 к рейтингу критического удара", GOSSIP_SENDER_MAIN, 125);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+29 к силе заклинаний и +20 к рейтингу устойчивости", GOSSIP_SENDER_MAIN, 126);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+30 к выносливости и + 25 к рейтингу устойчивости", GOSSIP_SENDER_MAIN, 127);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+37 к выносливости и + 20 к рейтингу защиты", GOSSIP_SENDER_MAIN, 128);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+50 к силе атаки и + 20 к рейтингу критического удара", GOSSIP_SENDER_MAIN, 129);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+50 к силе атаки и +20 к рейтингу устойчивости", GOSSIP_SENDER_MAIN, 130);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 к сопротивлению тайной магии и 30 к выносливости", GOSSIP_SENDER_MAIN, 131);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 к сопротивлению огню и 30 к выносливости", GOSSIP_SENDER_MAIN, 132);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 к сопротивлению темной магии и 30 к выносливости", GOSSIP_SENDER_MAIN, 133);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 к сопротивлению магии льда и 30 к выносливости", GOSSIP_SENDER_MAIN, 134);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 к сопротивлению силам природы и 30 к выносливости", GOSSIP_SENDER_MAIN, 135);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tНазад", GOSSIP_SENDER_MAIN, 300);
            player->PlayerTalkClass->SendGossipMenu(100005, creature->GetGUID());
            return true;
            break;
        case 5: // Enchant Shoulders
            if (player->HasSkill(SKILL_INSCRIPTION) && player->GetSkillValue(SKILL_INSCRIPTION) == 450)
            {
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+120 к силе атаки и + 15 к рейтингу критического эффекта [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 136);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+70 к силе заклинаний и +8 к мане каждые 5 секунд [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 137);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+70 к силе заклинаний и +15 к рейтингу критического эффекта [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 411);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+60 к рейтингу уклонения и + 15 к защите [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 138);
                // player->ADD_GOSSIP_ITEM(7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 к сопротивлению силам природы и 30 к выносливости", GOSSIP_SENDER_MAIN, 139);
            }
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+40 к силе атаки и +15 к рейтингу критического удара", GOSSIP_SENDER_MAIN, 140);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+24 к силе заклинаний и +8 ед.маны каждые 5 секунд", GOSSIP_SENDER_MAIN, 141);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+30 к выносливости и +15 к рейтингу устойчивости", GOSSIP_SENDER_MAIN, 142);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+20 к рейтингу уклонения и +15 к рейтингу защиты", GOSSIP_SENDER_MAIN, 143);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+24 к силе заклинаний и +15 к рейтингу критического удара", GOSSIP_SENDER_MAIN, 144);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+23 к силе заклинаний и +15 к рейтингу устойчивости", GOSSIP_SENDER_MAIN, 145);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+40 к силе атаки и + 15 крейтингу устойчивости", GOSSIP_SENDER_MAIN, 146);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tНазад", GOSSIP_SENDER_MAIN, 300);
            player->PlayerTalkClass->SendGossipMenu(100006, creature->GetGUID());
            return true;
            break;
        case 6: // Enchant Cloak
            if (player->HasSkill(SKILL_TAILORING) && player->GetSkillValue(SKILL_TAILORING) == 450)
            {
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tПружинистая паутинка [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 147);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tВышивка темного сияния [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 149);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tСветлотканая вышивка [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 150);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tВышивка в виде рукояти меча [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 151);
            }
            if (player->HasSkill(SKILL_ENGINEERING) && player->GetSkillValue(SKILL_ENGINEERING) == 450)
            {
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tПружинистая паутинка +27 к силе заклинаний + парашут [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 350);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tЭластичная подкладка +23 к ловкости + парашут [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 351);
            }
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+10 к духу и снижение угрозы на 2%", GOSSIP_SENDER_MAIN, 152);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+16 к рейтингу защиты", GOSSIP_SENDER_MAIN, 153);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+35 к проникающей способности заклинаний", GOSSIP_SENDER_MAIN, 154);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+225 к броне", GOSSIP_SENDER_MAIN, 155);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+22 к ловкости", GOSSIP_SENDER_MAIN, 156);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+23 к рейтингу скорости", GOSSIP_SENDER_MAIN, 157);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tУлучшение незаметности и +10 к ловкости", GOSSIP_SENDER_MAIN, 148);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tНазад", GOSSIP_SENDER_MAIN, 300);
            player->PlayerTalkClass->SendGossipMenu(100007, creature->GetGUID());
            return true;
            break;
        case 7: //Enchant chest
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+10 ко всем характеристикам", GOSSIP_SENDER_MAIN, 158);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+275 к здоровью", GOSSIP_SENDER_MAIN, 159);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+10 ед.маны каждые 5 секунд", GOSSIP_SENDER_MAIN, 160);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+20 к рейтингу устойчивости", GOSSIP_SENDER_MAIN, 161);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+22 к рейтингу защиты", GOSSIP_SENDER_MAIN, 162);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tНазад", GOSSIP_SENDER_MAIN, 300);
            player->PlayerTalkClass->SendGossipMenu(100008, creature->GetGUID());
            return true;
            break;
        case 8: //Enchant Bracers            
            if (player->HasSkill(SKILL_LEATHERWORKING) && player->GetSkillValue(SKILL_LEATHERWORKING) == 450)
            {
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+70 к сопротивлению тайной магии [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 170);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+70 к сопротивлению огню [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 171);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+70 к сопротивлению магии льда [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 172);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+70 к сопротивлению силам природы [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 173);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+70 к сопротивлению темной магии [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 174);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+130 к силе атаки [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 175);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+102 к выносливости [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 176);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+76 к силе заклинаний [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 177);
            }
            if (player->HasSkill(SKILL_BLACKSMITHING) && player->GetSkillValue(SKILL_BLACKSMITHING) == 450)
            {
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tДобавить место для сокета [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 601);
            }
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+40 к выносливости", GOSSIP_SENDER_MAIN, 163);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+30 к силе заклинаний", GOSSIP_SENDER_MAIN, 164);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+50 к силе атаки", GOSSIP_SENDER_MAIN, 165);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+18 к духу", GOSSIP_SENDER_MAIN, 166);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+15 к рейтингу мастерства", GOSSIP_SENDER_MAIN, 167);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+16 к интелекту", GOSSIP_SENDER_MAIN, 169);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tНазад", GOSSIP_SENDER_MAIN, 300);
            player->PlayerTalkClass->SendGossipMenu(100009, creature->GetGUID());
            return true;
            break;
        case 9: //Enchant Gloves		
            if (player->HasSkill(SKILL_ENGINEERING) && player->GetSkillValue(SKILL_ENGINEERING) == 450)
            {

                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tГиперскоростные ускорители [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 222);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tНарукавная зажигательная ракетница [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 223);

            }
            if (player->HasSkill(SKILL_BLACKSMITHING) && player->GetSkillValue(SKILL_BLACKSMITHING) == 450)
            {
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tДобавить место для сокета [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 600);
            }
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+16 к рейтингу критического удара", GOSSIP_SENDER_MAIN, 178);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+2% угрозы и + 10 к рейтингу парирования", GOSSIP_SENDER_MAIN, 179);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+44 к силе атаки", GOSSIP_SENDER_MAIN, 180);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+20 к ловкости", GOSSIP_SENDER_MAIN, 181);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+20 к рейтингу меткости", GOSSIP_SENDER_MAIN, 182);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+28 к силе заклинаний", GOSSIP_SENDER_MAIN, 168);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tНазад", GOSSIP_SENDER_MAIN, 300);
            player->PlayerTalkClass->SendGossipMenu(100010, creature->GetGUID());
            return true;
            break;
        case 10: //Enchant legs
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+40 к устойчивости и + 28 к выносливости", GOSSIP_SENDER_MAIN, 184);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+55 к выносливости и +22 к ловкости", GOSSIP_SENDER_MAIN, 185);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+75 к силе атаки и +22 к рейтингу критического удара", GOSSIP_SENDER_MAIN, 186);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+50 к силе заклинаний и +20 к духу", GOSSIP_SENDER_MAIN, 187);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+50 к силе заклинаний и +30 к выносливости", GOSSIP_SENDER_MAIN, 188);
            // player->ADD_GOSSIP_ITEM(7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+72 к выносливости и +35 к ловкости", GOSSIP_SENDER_MAIN, 189);
            // player->ADD_GOSSIP_ITEM(7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+100 к силе атаки и +36 к рейтингу критического удара", GOSSIP_SENDER_MAIN, 190);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tНазад", GOSSIP_SENDER_MAIN, 300);
            player->PlayerTalkClass->SendGossipMenu(100017, creature->GetGUID());
            return true;
            break;
        case 11: //Enchant feet
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+32 к силе атаки", GOSSIP_SENDER_MAIN, 191);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+15 к выносливости и небольшое увеличение скорости", GOSSIP_SENDER_MAIN, 192);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+16 к ловкости", GOSSIP_SENDER_MAIN, 193);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+18 к духу", GOSSIP_SENDER_MAIN, 194);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+7 ед. здоровья и маны каждые 5 сек", GOSSIP_SENDER_MAIN, 195);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+12 к рейтингу меткости, и + 12 к рейтингу критического удара", GOSSIP_SENDER_MAIN, 196);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+22 к выносливости", GOSSIP_SENDER_MAIN, 197);
            if (player->HasSkill(SKILL_ENGINEERING) && player->GetSkillValue(SKILL_ENGINEERING) == 450)
            {
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tНитро сапоги [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 198);
            }
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tНазад", GOSSIP_SENDER_MAIN, 300);
            player->PlayerTalkClass->SendGossipMenu(100012, creature->GetGUID());
            return true;
            break;
        case 12: //Enchant rings
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+40 к силе атаки [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 202);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+23 к силе заклинаний [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 203);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+30 к выносливости [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 204);
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tНазад", GOSSIP_SENDER_MAIN, 300);
            player->PlayerTalkClass->SendGossipMenu(100013, creature->GetGUID());
            return true;
            break;
        case 13: //Enchant Off-Hand weapons
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendNotification("Это заточки нужен одной рукой оружие оборудованная в левой руке.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tОтведение удара", GOSSIP_SENDER_MAIN, 205);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tВытягивание Крови", GOSSIP_SENDER_MAIN, 219);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+26 к ловкости", GOSSIP_SENDER_MAIN, 206);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+45 к духу", GOSSIP_SENDER_MAIN, 207);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tБоевое исступление", GOSSIP_SENDER_MAIN, 208);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+25 к рейтингу меткости, +25 к рейтингу критического удара", GOSSIP_SENDER_MAIN, 209);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tЧерная магия", GOSSIP_SENDER_MAIN, 210);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tВоеначальник", GOSSIP_SENDER_MAIN, 211);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tОружие Ледолома", GOSSIP_SENDER_MAIN, 212);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tОберег", GOSSIP_SENDER_MAIN, 213);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+50 к выносливости", GOSSIP_SENDER_MAIN, 214);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+65 к силе атаки", GOSSIP_SENDER_MAIN, 215);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|t+63 к силе заклинаний", GOSSIP_SENDER_MAIN, 216);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tМангуст", GOSSIP_SENDER_MAIN, 217);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tПалач", GOSSIP_SENDER_MAIN, 218);
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tНазад", GOSSIP_SENDER_MAIN, 300);
                    player->PlayerTalkClass->SendGossipMenu(100002, creature->GetGUID());
                    return true;
                }
                else
                {
                    player->GetSession()->SendNotification("В левой руке не является оружием.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 15: //Enchant poyas                        
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tДобавить место для сокета", GOSSIP_SENDER_MAIN, 220);
            if (player->HasSkill(SKILL_ENGINEERING) && player->GetSkillValue(SKILL_ENGINEERING) == 450)
            {
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tНаременные осколочные гранаты [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 221);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargepositive:20:20:-15:0|tГенератор ЭМИ [|cffD80000+|r]", GOSSIP_SENDER_MAIN, 299);
            }
            player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\Spell_chargenegative:20:20:-15:0|tНазад", GOSSIP_SENDER_MAIN, 300);
            player->PlayerTalkClass->SendGossipMenu(100513, creature->GetGUID());
            return true;
            break;
        case 100:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_AGILITY_1H);
            OnGossipHello(player, creature);
            break;
        case 101:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_SPIRIT);
            OnGossipHello(player, creature);
            break;
        case 102:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_BLADE_WARD);
            OnGossipHello(player, creature);
            break;
        case 103:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_BLOOD_DRAINING);
            OnGossipHello(player, creature);
            break;
        case 104:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_BERSERKING);
            OnGossipHello(player, creature);
            break;
        case 105:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_ACCURACY);
            OnGossipHello(player, creature);
            break;
        case 106:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_BLACK_MAGIC);
            OnGossipHello(player, creature);
            break;
        case 107:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_BATTLEMASTER);
            OnGossipHello(player, creature);
            break;
        case 108:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_ICEBREAKER);
            OnGossipHello(player, creature);
            break;
        case 109:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_LIFEWARD);
            OnGossipHello(player, creature);
            break;
        case 110:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_TITANGUARD);
            OnGossipHello(player, creature);
            break;
        case 111:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_POTENCY);
            OnGossipHello(player, creature);
            break;
        case 112:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_MIGHTY_SPELL_POWER);
            OnGossipHello(player, creature);
            break;
        case 113:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_2WEP_MONGOOSE);
            OnGossipHello(player, creature);
            break;
        case 114:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_EXECUTIONER);
            OnGossipHello(player, creature);
            break;
        case 701:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_WEP_STRYJA);
            OnGossipHello(player, creature);
            break;
        case 115:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a 2H weapon equipped.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_2WEP_GREATER_SPELL_POWER);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("You don't have a Two-Handed weapon equipped.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 116:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a 2H weapon equipped.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_2WEP_AGILITY);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("You don't have a Two-Handed weapon equipped.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 117:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a 2H weapon equipped.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND), ENCHANT_2WEP_MASSACRE);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("You don't have a Two-Handed weapon equipped.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 118:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a shield equipped.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_SHIELD)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_DEFENSE);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("You don't have a shield equipped.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 119:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a shield equipped.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_SHIELD)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_INTELLECT);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("You don't have a shield equipped.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 120:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a shield equipped.");
                    OnGossipHello(player, creature);
                    return false;
                }

                if (item->GetTemplate()->InventoryType == INVTYPE_SHIELD)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_RESILIENCE);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("You don't have a shield equipped.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 121:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a shield equipped.");
                    OnGossipHello(player, creature);
                    return false;
                }

                if (item->GetTemplate()->InventoryType == INVTYPE_SHIELD)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_TITANIUM_PLATING);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("You don't have a shield equipped.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 122:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a shield equipped.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_SHIELD)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_STAMINA);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("You don't have a shield equipped.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 123:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a shield equipped.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_SHIELD)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_SHIELD_TOUGHSHIELD);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("You don't have a shield equipped.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 124:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_BLISSFUL_MENDING);
            OnGossipHello(player, creature);
            break;
        case 125:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_BURNING_MYSTERIES);
            OnGossipHello(player, creature);
            break;
        case 126:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_DOMINANCE);
            OnGossipHello(player, creature);
            break;
        case 127:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_SAVAGE_GLADIATOR);
            OnGossipHello(player, creature);
            break;
        case 128:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_STALWART_PROTECTOR);
            OnGossipHello(player, creature);
            break;
        case 129:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_TORMENT);
            OnGossipHello(player, creature);
            break;
        case 130:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_TRIUMPH);
            OnGossipHello(player, creature);
            break;
        case 131:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_ECLIPSED_MOON);
            OnGossipHello(player, creature);
            break;
        case 132:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_FLAME_SOUL);
            OnGossipHello(player, creature);
            break;
        case 133:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_FLEEING_SHADOW);
            OnGossipHello(player, creature);
            break;
        case 134:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_FROSTY_SOUL);
            OnGossipHello(player, creature);
            break;
        case 135:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD), ENCHANT_HEAD_TOXIC_WARDING);
            OnGossipHello(player, creature);
            break;
        case 136:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_MASTERS_AXE);
            OnGossipHello(player, creature);
            break;
        case 137:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_MASTERS_CRAG);
            OnGossipHello(player, creature);
            break;
        case 411:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_KRIT);
            OnGossipHello(player, creature);
            break;
        case 138:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_MASTERS_PINNACLE);
            OnGossipHello(player, creature);
            break;
        case 139:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_MASTERS_STORM);
            OnGossipHello(player, creature);
            break;
        case 140:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_GREATER_AXE);
            OnGossipHello(player, creature);
            break;
        case 141:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_GREATER_CRAG);
            OnGossipHello(player, creature);
            break;
        case 142:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_GREATER_GLADIATOR);
            OnGossipHello(player, creature);
            break;
        case 143:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_GREATER_PINNACLE);
            OnGossipHello(player, creature);
            break;
        case 144:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_GREATER_STORM);
            OnGossipHello(player, creature);
            break;
        case 145:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_DOMINANCE);
            OnGossipHello(player, creature);
            break;
        case 146:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS), ENCHANT_SHOULDER_TRIUMPH);
            OnGossipHello(player, creature);
            break;
        case 147:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_SPRINGY_ARACHNOWEAVE);
            OnGossipHello(player, creature);
            break;
        case 148:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_SHADOW_ARMOR);
            OnGossipHello(player, creature);
            break;
        case 149:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_DARKGLOW_EMBROIDERY);
            OnGossipHello(player, creature);
            break;
        case 150:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_LIGHTWEAVE_EMBROIDERY);
            OnGossipHello(player, creature);
            break;
        case 151:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_SWORDGUARD_EMBROIDERY);
            OnGossipHello(player, creature);
            break;
        case 351:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_SPINADVA);
            OnGossipHello(player, creature);
            break;
        case 350:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_SPINA);
            OnGossipHello(player, creature);
            break;
        case 152:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_WISDOM);
            OnGossipHello(player, creature);
            break;
        case 153:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_TITANWEAVE);
            OnGossipHello(player, creature);
            break;
        case 154:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_SPELL_PIERCING);
            OnGossipHello(player, creature);
            break;
        case 155:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_MIGHTY_ARMOR);
            OnGossipHello(player, creature);
            break;
        case 156:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_MAJOR_AGILITY);
            OnGossipHello(player, creature);
            break;
        case 157:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK), ENCHANT_CLOAK_GREATER_SPEED);
            OnGossipHello(player, creature);
            break;
        case 158:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST), ENCHANT_CHEST_POWERFUL_STATS);
            OnGossipHello(player, creature);
            break;
        case 159:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST), ENCHANT_CHEST_SUPER_HEALTH);
            OnGossipHello(player, creature);
            break;
        case 160:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST), ENCHANT_CHEST_GREATER_MAINA_REST);
            OnGossipHello(player, creature);
            break;
        case 161:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST), ENCHANT_CHEST_EXCEPTIONAL_RESIL);
            OnGossipHello(player, creature);
            break;
        case 162:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST), ENCHANT_CHEST_GREATER_DEFENSE);
            OnGossipHello(player, creature);
            break;
        case 163:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_MAJOR_STAMINA);
            OnGossipHello(player, creature);
            break;
        case 601:
            AdvancedEnchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), PRISMATIC_ENCHANTMENT_SLOT, ENCHANT_ZAPYASTIE_SOCKET); // Socket Belt		
            OnGossipHello(player, creature);
            break;
        case 164:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_SUPERIOR_SP);
            OnGossipHello(player, creature);
            break;
        case 165:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_GREATER_ASSUALT);
            OnGossipHello(player, creature);
            break;
        case 166:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_MAJOR_SPIRT);
            OnGossipHello(player, creature);
            break;
        case 167:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_EXPERTISE);
            OnGossipHello(player, creature);
            break;
        case 169:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_INTELLECT);
            OnGossipHello(player, creature);
            break;
        case 170:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_ARCANE);
            OnGossipHello(player, creature);
            break;
        case 171:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_FIRE);
            OnGossipHello(player, creature);
            break;
        case 172:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_FROST);
            OnGossipHello(player, creature);
            break;
        case 173:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_NATURE);
            OnGossipHello(player, creature);
            break;
        case 174:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_SHADOW);
            OnGossipHello(player, creature);
            break;
        case 175:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_ATTACK);
            OnGossipHello(player, creature);
            break;
        case 176:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_STAMINA);
            OnGossipHello(player, creature);
            break;
        case 177:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS), ENCHANT_BRACERS_FURL_SPELLPOWER);
            OnGossipHello(player, creature);
            break;
        case 178:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_GREATER_BLASTING);
            OnGossipHello(player, creature);
            break;
        case 179:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_ARMSMAN);
            OnGossipHello(player, creature);
            break;
        case 180:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_CRUSHER);
            OnGossipHello(player, creature);
            break;
        case 181:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_AGILITY);
            OnGossipHello(player, creature);
            break;
        case 182:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_PRECISION);
            OnGossipHello(player, creature);
            break;
        case 600:
            AdvancedEnchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), PRISMATIC_ENCHANTMENT_SLOT, ENCHANT_KISTIRYK_SOCKET); // Socket Belt
            OnGossipHello(player, creature);
            break;
        case 168:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_SPD);
            OnGossipHello(player, creature);
            break;
        case 184:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_EARTHEN);
            OnGossipHello(player, creature);
            break;
        case 185:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_FROSTHIDE);
            OnGossipHello(player, creature);
            break;
        case 186:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_ICESCALE);
            OnGossipHello(player, creature);
            break;
        case 187:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_BRILLIANT_SPELLTHREAD);
            OnGossipHello(player, creature);
            break;
        case 188:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_SAPPHIRE_SPELLTHREAD);
            OnGossipHello(player, creature);
            break;
        case 189:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_DRAGONSCALE);
            OnGossipHello(player, creature);
            break;
        case 190:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS), ENCHANT_LEG_WYRMSCALE);
            OnGossipHello(player, creature);
            break;
        case 191:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_GREATER_ASSULT);
            OnGossipHello(player, creature);
            break;
        case 192:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_TUSKARS_VITLIATY);
            OnGossipHello(player, creature);
            break;
        case 193:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_SUPERIOR_AGILITY);
            OnGossipHello(player, creature);
            break;
        case 194:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_GREATER_SPIRIT);
            OnGossipHello(player, creature);
            break;
        case 195:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_GREATER_VITALITY);
            OnGossipHello(player, creature);
            break;
        case 196:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_ICEWALKER);
            OnGossipHello(player, creature);
            break;
        case 197:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_GREATER_FORTITUDE);
            OnGossipHello(player, creature);
            break;
        case 198:
            item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET);
            if (!item)
            {
                player->GetSession()->SendAreaTriggerMessage("Вы должны одеть боты.");
                player->PlayerTalkClass->SendCloseGossip();
                return false;
            }
            if (item->GetTemplate()->ItemLevel>150)
            {
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET), ENCHANT_BOOTS_NITRO_BOOTS);
                OnGossipHello(player, creature);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage("Вы не можете зачарить данный итем");
                OnGossipHello(player, creature);
            }
            break;
        case 202:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER1), ENCHANT_RING_ASSULT);
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER2), ENCHANT_RING_ASSULT);
            OnGossipHello(player, creature);
            break;
        case 203:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER1), ENCHANT_RING_GREATER_SP);
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER2), ENCHANT_RING_GREATER_SP);
            OnGossipHello(player, creature);
            break;
        case 204:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER1), ENCHANT_RING_STAMINA);
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER2), ENCHANT_RING_STAMINA);
            OnGossipHello(player, creature);
            break;
        case 220:
            AdvancedEnchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WAIST), PRISMATIC_ENCHANTMENT_SLOT, ENCHANT_WAIST_SOCKET);
            OnGossipHello(player, creature);
            break;
        case 221:
            item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WAIST);
            if (!item)
            {
                player->GetSession()->SendAreaTriggerMessage("Вы должны одеть пояс.");
                player->PlayerTalkClass->SendCloseGossip();
                return false;
            }
            if (item->GetTemplate()->ItemLevel>150)
            {
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WAIST), ENCHANT_BOMBA);
                OnGossipHello(player, creature);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage("Вы не можете зачарить данный итем");
                OnGossipHello(player, creature);
            }
            break;
        case 299: // 
            item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WAIST);
            if (!item)
            {
                player->GetSession()->SendAreaTriggerMessage("Вы должны одеть пояс.");
                player->PlayerTalkClass->SendCloseGossip();
                return false;
            }
            if (item->GetTemplate()->ItemLevel>150)
            {
                Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WAIST), ENCHANT_VZRIV);
                OnGossipHello(player, creature);
            }
            else
            {
                player->GetSession()->SendAreaTriggerMessage("Вы не можете зачарить данный итем");
                OnGossipHello(player, creature);
            }
            break;
        case 222:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_HAST);
            OnGossipHello(player, creature);
            break;
        case 223:
            Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS), ENCHANT_GLOVES_RAKETA);
            OnGossipHello(player, creature);
            break;
        case 205:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_BLADE_WARD);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                }
            }
            break;

        case 206:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_AGILITY_1H);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                }
            }
            break;

        case 207:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_SPIRIT);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                }
            }
            break;

        case 208:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_BERSERKING);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 209:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_ACCURACY);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 210:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_BLACK_MAGIC);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 211:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_BATTLEMASTER);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                }
            }
            break;

        case 212:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_ICEBREAKER);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 213:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_LIFEWARD);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 214:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_TITANGUARD);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 215:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_POTENCY);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                }
            }
            break;

        case 216:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_MIGHTY_SPELL_POWER);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                }
            }
            break;

        case 217:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_2WEP_MONGOOSE);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                }
            }
            break;

        case 218:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_EXECUTIONER);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 219:
            {
                item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                if (!item)
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                    return false;
                }
                if (item->GetTemplate()->InventoryType == INVTYPE_WEAPON)
                {
                    Enchant(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND), ENCHANT_WEP_BLOOD_DRAINING);
                    OnGossipHello(player, creature);
                }
                else
                {
                    player->GetSession()->SendAreaTriggerMessage("This enchant needs a one-hand weapon equipped in the off-hand.");
                    OnGossipHello(player, creature);
                }
            }
            break;
        case 300: //<-Back menu
            {
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_helmet_164:20:20:-15:0|t[Зачарить: Голову]", GOSSIP_SENDER_MAIN, 4);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_shoulder_126:20:20:-15:0|t[Зачарить: Плечи]", GOSSIP_SENDER_MAIN, 5);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_chest_plate26:20:20:-15:0|t[Зачарить: Грудь]", GOSSIP_SENDER_MAIN, 7);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_pants_plate_37:20:20:-15:0|t[Зачарить: Ноги]", GOSSIP_SENDER_MAIN, 10);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_gauntlets_91:20:20:-15:0|t[Зачарить: Кисти рук]", GOSSIP_SENDER_MAIN, 9);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_bracer_43:20:20:-15:0|t[Зачарить: Запястья]", GOSSIP_SENDER_MAIN, 8);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_belt_48c:20:20:-15:0|t[Зачарить: Пояс]", GOSSIP_SENDER_MAIN, 15);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_boots_plate_13:20:20:-15:0|t[Зачарить: Ступни]", GOSSIP_SENDER_MAIN, 11);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_misc_cape_18:20:20:-15:0|t[Зачарить: Спину]", GOSSIP_SENDER_MAIN, 6);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_axe_113:20:20:-15:0|t[Зачарить: Оружие]", GOSSIP_SENDER_MAIN, 1);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_axe_116:20:20:-15:0|t[Зачарить: Оружие в левой руке]", GOSSIP_SENDER_MAIN, 13);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_axe_115:20:20:-15:0|t[Зачарить: Двуручное оружие]", GOSSIP_SENDER_MAIN, 2);
                player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_shield_75:20:20:-15:0|t[Зачарить: Щит]", GOSSIP_SENDER_MAIN, 3);
                if (player->HasSkill(SKILL_ENCHANTING) && player->GetSkillValue(SKILL_ENCHANTING) == 450)
                {
                    player->ADD_GOSSIP_ITEM( 7, "|TInterface\\icons\\inv_jewelry_ring_83:20:20:-15:0|t[Зачарить: Кольца]", GOSSIP_SENDER_MAIN, 12);
                }
                player->PlayerTalkClass->SendGossipMenu(100007, creature->GetGUID());
                return true;
            }
            break;
        }
        return true;
    }
};

#define EMOTE_ALREADY_ACHIEVEMENT "|cff00ffffУ вас уже имеется данное достижение!|r"
#define EMOTE_NO_ETHEREAL_CREDITS "|cffff0000У вас недостаточно Очков Арены! Регайте аренку чаще)|r"

class npc_eferial : public CreatureScript
{
public:
    npc_eferial() : CreatureScript("npc_eferial") { }

    bool OnGossipHello(Player* player, Creature* m_creature)
    {
        player->ADD_GOSSIP_ITEM( 0, "|TInterface/ICONS/ability_mount_drake_red:25:25:-15:0|t Покупка достижений (АП)", GOSSIP_SENDER_MAIN, 1);
        player->ADD_GOSSIP_ITEM( 0, "|TInterface/ICONS/Inv_mask_01:25:25:-15:0|t Плюшки (PvP Token)", GOSSIP_SENDER_MAIN, 27);
        player->PlayerTalkClass->SendGossipMenu(DEFAULT_GOSSIP_MESSAGE, m_creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* m_creature, uint32 sender, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {

        case 27:
            player->GetSession()->SendListInventory(m_creature->GetGUID(), 100018);
            break;

        case 1:
            {
                player->PlayerTalkClass->ClearMenus();
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/ability_mount_drake_red:25:25:-15:0|t Лииииирооооой (100 АП)", GOSSIP_SENDER_MAIN, 100);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/achievement_bg_masterofallbgs:25:25:-15:0|t Долгое и странное путешествие(100 АП)", GOSSIP_SENDER_MAIN, 200);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/achievement_worldevent_merrymaker:25:25:-15:0|t Весельчак(100 АП)", GOSSIP_SENDER_MAIN, 300);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/inv_thanksgiving_turkey:25:25:-15:0|t Странник(100 АП)", GOSSIP_SENDER_MAIN, 400);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/inv_misc_bone_humanskull_02:25:25:-15:0|t Вечеринка мертвецов(100 АП)", GOSSIP_SENDER_MAIN, 500);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/achievement_halloween_witch_01:25:25:-15:0|t Да тыквится имя твое(100 АП)", GOSSIP_SENDER_MAIN, 600);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/achievement_worldevent_brewmaster:25:25:-15:0|t Хмелевар(100 АП)", GOSSIP_SENDER_MAIN, 700);
                player->ADD_GOSSIP_ITEM( 9, "|TInterface/ICONS/inv_helmet_66:25:25:-15:0|t Сундук капитана(100 АП)", GOSSIP_SENDER_MAIN, 800);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/inv_summerfest_symbol_high:25:25:-15:0|t Страж огня(100 АП)", GOSSIP_SENDER_MAIN, 900);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/inv_misc_toy_04:25:25:-15:0|t Все лучшее – детям(100 АП)", GOSSIP_SENDER_MAIN, 1000);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/inv_egg_09:25:25:-15:0|t Чудесный садовник(100 АП)", GOSSIP_SENDER_MAIN, 1100);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/achievement_worldevent_valentine:25:25:-15:0|t Безумие любви(100 АП)", GOSSIP_SENDER_MAIN, 1200);
                player->ADD_GOSSIP_ITEM( 5, "|TInterface/ICONS/spell_holy_holyguidance:25:25:-15:0|t Далее ->", GOSSIP_SENDER_MAIN, 1300);
                player->PlayerTalkClass->SendGossipMenu(DEFAULT_GOSSIP_MESSAGE, m_creature->GetGUID());
            }
            break;
        case 101:
            player->CLOSE_GOSSIP_MENU();
            break;
        case 100: // Лироой
            if (player->HasAchieved(2188))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/ability_mount_drake_red:25:25:-15:0|t Лииииирооооой'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(2188));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 200: // Долгое и странное путешествие
            if (player->HasAchieved(2144))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/achievement_bg_masterofallbgs:25:25:-15:0|t Долгое и странное путешествие'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(2144));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 300: // Весельчак
            if (player->HasAchieved(1691))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/achievement_worldevent_merrymaker:25:25:-15:0|t Весельчак'", LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
                player->CompletedAchievement(sAchievementMgr->GetAchievement(1691));
            }
            break;
        case 400: // Странник
            if (player->HasAchieved(3478))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/inv_thanksgiving_turkey:25:25:-15:0|t Странник'", LANG_UNIVERSAL, player, false);

                player->CompletedAchievement(sAchievementMgr->GetAchievement(3478));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 500: // Вечеринка мертвецов
            if (player->HasAchieved(3456))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/inv_misc_bone_humanskull_02:25:25:-15:0|t Вечеринка мертвецов'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(3456));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 600: // Да тыквится имя твое
            if (player->HasAchieved(1656))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/achievement_halloween_witch_01:25:25:-15:0|t Да тыквится имя твое'", LANG_UNIVERSAL, player, false);

                player->CompletedAchievement(sAchievementMgr->GetAchievement(1656));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 700: // Хмелевар
            if (player->HasAchieved(1683))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/achievement_worldevent_brewmaster:25:25:-15:0|t Хмелевар'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(1683));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 800: // Сундук капитана
            if (player->HasAchieved(3457))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();

            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/inv_helmet_66:25:25:-15:0|t Сундук капитана'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(3457));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 900: // Страж огня
            if (player->HasAchieved(1038))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/inv_summerfest_symbol_high:25:25:-15:0|t Страж огня'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(1038));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 1000: // Все лучшее – детям
            if (player->HasAchieved(1793))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/inv_misc_toy_04:25:25:-15:0|t Все лучшее – детям'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(1793));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 1100: // Чудесный садовник
            if (player->HasAchieved(2798))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/inv_egg_09:25:25:-15:0|t Чудесный садовник'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(2798));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 1200: // Безумие любви
            if (player->HasAchieved(1693))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/achievement_worldevent_valentine:25:25:-15:0|t Безумие любви'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(1693));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 1300:
            {
                player->PlayerTalkClass->ClearMenus();
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/achievement_worldevent_lunar:25:25:-15:0|t Почтение к предкам(100 АП)", GOSSIP_SENDER_MAIN, 14);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/ability_thunderclap:25:25:-15:0|t Однокнопочный(100 АП)", GOSSIP_SENDER_MAIN, 15);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/inv_helmet_25:25:25:-15:0|t Слава герою(100 АП)", GOSSIP_SENDER_MAIN, 16);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/inv_misc_bomb_02:25:25:-15:0|t Сапер(100 АП)", GOSSIP_SENDER_MAIN, 17);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/achievement_dungeon_naxxramas_10man:25:25:-15:0|t Падение Наксрамаса (25 игроков)(100 АП)", GOSSIP_SENDER_MAIN, 18);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/achievement_quests_completed_08:25:25:-15:0|t 3000 заданий(100 АП)", GOSSIP_SENDER_MAIN, 19);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/achievement_worldevent_lunar:25:25:-15:0|t Почтение к предкам(100 АП)", GOSSIP_SENDER_MAIN, 20);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/achievement_reputation_tuskarr:25:25:-15:0|t Клыкаррмагеддон(100 АП)", GOSSIP_SENDER_MAIN, 21);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/ability_mount_whitedirewolf:25:25:-15:0|t Племя Бешеного Сердца(100 АП)", GOSSIP_SENDER_MAIN, 22);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/achievement_boss_princemalchezaar_02:25:25:-15:0|t Каражан(100 АП)", GOSSIP_SENDER_MAIN, 23);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/inv_misc_food_15:25:25:-15:0|t Друг или индюк? (100 АП)", GOSSIP_SENDER_MAIN, 24);
                player->ADD_GOSSIP_ITEM( 6, "|TInterface/ICONS/spell_holy_weaponmastery:25:25:-15:0|t Бессмертный (100 АП)", GOSSIP_SENDER_MAIN, 25);
                player->SEND_GOSSIP_MENU( 68, m_creature->GetGUID());
            }
            break;
        case 14: // Почтение к предкам
            if (player->HasAchieved(913))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/achievement_worldevent_lunar:25:25:-15:0|t Почтение к предкам'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(913));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 15: // Однокнопочный
            if (player->HasAchieved(3803))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/ability_thunderclap:25:25:-15:0|t Однокнопочный'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(3803));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 16: // Слава герою
            if (player->HasAchieved(2136))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/inv_helmet_25:25:25:-15:0|t Слава герою'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(2136));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 17: // Сапер
            if (player->HasAchieved(1428))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/inv_misc_bomb_02:25:25:-15:0|t Сапер'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(1428));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 18: // Падение Наксрамаса 
            if (player->HasAchieved(577))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/achievement_dungeon_naxxramas_10man:25:25:-15:0|t Падение Наксрамаса (25 игроков)'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(577));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 19: // 3000 заданий
            if (player->HasAchieved(978))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/achievement_quests_completed_08:25:25:-15:0|t 3000 заданий'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(978));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 20: // Почтение к предкам
            if (player->HasAchieved(913))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/achievement_worldevent_lunar:25:25:-15:0|t Почтение к предкам'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(913));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 21: // Клыкаррмагеддон
            if (player->HasAchieved(949))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/achievement_reputation_tuskarr:25:25:-15:0|t Клыкаррмагеддон'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(949));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 22: // Племя Бешеного Сердца
            if (player->HasAchieved(950))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/ability_mount_whitedirewolf:25:25:-15:0|t Племя Бешеного Сердца'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(950));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 23: // Каражан
            if (player->HasAchieved(690))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/achievement_boss_princemalchezaar_02:25:25:-15:0|t Каражан'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(690));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 24: // Друг или индюк?
            if (player->HasAchieved(1254))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/inv_misc_food_15:25:25:-15:0|t Друг или индюк?'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(1254));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        case 25: // Бессмертный
            if (player->HasAchieved(2186))
            {
                m_creature->Whisper(EMOTE_ALREADY_ACHIEVEMENT, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else if (player->GetArenaPoints() < 100)
            {
                m_creature->Whisper(EMOTE_NO_ETHEREAL_CREDITS, LANG_UNIVERSAL, player, false);
                player->CLOSE_GOSSIP_MENU();
            }
            else
            {
                player->ModifyArenaPoints(-100);
                m_creature->Whisper("|cff00ff00Вы успешно купили достижение|r '|TInterface/ICONS/spell_holy_weaponmastery:25:25:-15:0|t Бессмертный'", LANG_UNIVERSAL, player, false);
                player->CompletedAchievement(sAchievementMgr->GetAchievement(2186));
                player->CLOSE_GOSSIP_MENU();
            }
            break;
        }
        return true;
    }
};

class npc_template : public CreatureScript
{
public:
    npc_template() : CreatureScript("npc_template") { }

    void ApplyGlyph(Player* player, uint8 slot, uint32 glyphID)
    {
        if (GlyphPropertiesEntry const* gp = sGlyphPropertiesStore.LookupEntry(glyphID))
        {
            if (uint32 oldGlyph = player->GetGlyph(slot))
            {
                player->RemoveAurasDueToSpell(sGlyphPropertiesStore.LookupEntry(oldGlyph)->SpellId);
                player->SetGlyph(slot, 0);
            }

            player->CastSpell(player, gp->SpellId, true);
            player->SetGlyph(slot, glyphID);
        }
    }

    void SetTemplateGlyphs(Player* player, std::string& playerSpecStr)
    {
        for (uint8 slot = 0; slot < MAX_GLYPH_SLOT_INDEX; ++slot)
        {
            QueryResult result = CharacterDatabase.PQuery("SELECT glyph FROM template_npc_glyphs "
            "WHERE playerClass = '%s' AND playerSpec = '%s' AND slot = '%u';", GetClassString(player).c_str(), playerSpecStr.c_str(), slot);

            if (!result)
            continue;

            Field* fields = result->Fetch();
            uint32 glyph = fields[0].GetUInt32();

            ApplyGlyph(player, slot, glyph);
        }
        player->SendTalentsInfoData(false);
    }

    void SetTemplateTalents(Player* player, std::string& playerSpecStr)
    {
        QueryResult select = CharacterDatabase.PQuery("SELECT talentId FROM template_npc_talents WHERE playerClass = '%s' AND "
        "playerSpec = '%s';", GetClassString(player).c_str(), playerSpecStr.c_str());

        if (!select)
        { 
            return;
        }
        else
        {
            do
            {
                Field* fields = select->Fetch(); 
                uint32 talentId = fields[0].GetUInt32(); 

                player->LearnSpell(talentId, false);
                player->AddTalent(talentId, player->GetActiveSpec(), true);
            }
            while (select->NextRow());
            player->SetFreeTalentPoints(0);
            player->SendTalentsInfoData(false);
        }
    }

    void ApplyBonus(Player* player, Item* item, EnchantmentSlot slot, uint32 bonusEntry)
    {
        if (!item)
        return;

        if (!bonusEntry || bonusEntry == 0)
        return;

        player->ApplyEnchantment(item, slot, false);
        item->SetEnchantment(slot, bonusEntry, 0, 0);
        player->ApplyEnchantment(item, slot, true);
    }

    void EquipTmpGear(Player* player, std::string& playerSpecStr)
    {
        if (player->getRace() == RACE_HUMAN)
        {
            for (uint8 equipmentSlot = EQUIPMENT_SLOT_START; equipmentSlot < EQUIPMENT_SLOT_END; ++equipmentSlot)
            {
                QueryResult result = CharacterDatabase.PQuery("SELECT itemEntry, enchant, socket1, socket2, socket3, bonusEnchant, prismaticEnchant "
                "FROM template_npc_human WHERE playerClass = '%s' AND playerSpec = '%s' AND pos = '%u';", GetClassString(player).c_str(), playerSpecStr.c_str(), equipmentSlot);

                if (!result)
                continue;

                Field* fields = result->Fetch(); 
                uint32 itemEntry = fields[0].GetUInt32(); 
                uint32 enchant = fields[1].GetUInt32(); 
                uint32 socket1 = fields[2].GetUInt32(); 
                uint32 socket2 = fields[3].GetUInt32(); 
                uint32 socket3 = fields[4].GetUInt32(); 
                uint32 bonusEnchant = fields[5].GetUInt32(); 
                uint32 prismaticEnchant = fields[6].GetUInt32();

                // Equip Gear
                player->EquipNewItem(equipmentSlot, itemEntry, true);

                // Apply Enchants and Gems
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), PERM_ENCHANTMENT_SLOT, enchant);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), SOCK_ENCHANTMENT_SLOT, socket1);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), SOCK_ENCHANTMENT_SLOT_2, socket2);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), SOCK_ENCHANTMENT_SLOT_3, socket3);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), BONUS_ENCHANTMENT_SLOT, bonusEnchant);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), PRISMATIC_ENCHANTMENT_SLOT, prismaticEnchant);
            }
        }
        else if (player->GetTeam() == ALLIANCE && player->getRace() != RACE_HUMAN)
        {
            for (uint8 equipmentSlot = EQUIPMENT_SLOT_START; equipmentSlot < EQUIPMENT_SLOT_END; ++equipmentSlot)
            {
                QueryResult result = CharacterDatabase.PQuery("SELECT itemEntry, enchant, socket1, socket2, socket3, bonusEnchant, prismaticEnchant "
                "FROM template_npc_alliance WHERE playerClass = '%s' AND playerSpec = '%s' AND pos = '%u';", GetClassString(player).c_str(), playerSpecStr.c_str(), equipmentSlot);

                if (!result)
                continue;

                Field* fields = result->Fetch(); 
                uint32 itemEntry = fields[0].GetUInt32(); 
                uint32 enchant = fields[1].GetUInt32(); 
                uint32 socket1 = fields[2].GetUInt32(); 
                uint32 socket2 = fields[3].GetUInt32(); 
                uint32 socket3 = fields[4].GetUInt32(); 
                uint32 bonusEnchant = fields[5].GetUInt32(); 
                uint32 prismaticEnchant = fields[6].GetUInt32();

                // Equip Gear
                player->EquipNewItem(equipmentSlot, itemEntry, true);

                // Apply Enchants and Gems
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), PERM_ENCHANTMENT_SLOT, enchant);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), SOCK_ENCHANTMENT_SLOT, socket1);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), SOCK_ENCHANTMENT_SLOT_2, socket2);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), SOCK_ENCHANTMENT_SLOT_3, socket3);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), BONUS_ENCHANTMENT_SLOT, bonusEnchant);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), PRISMATIC_ENCHANTMENT_SLOT, prismaticEnchant);
            }
        }
        else if (player->GetTeam() == HORDE)
        {
            for (uint8 equipmentSlot = EQUIPMENT_SLOT_START; equipmentSlot < EQUIPMENT_SLOT_END; ++equipmentSlot)
            {
                QueryResult result = CharacterDatabase.PQuery("SELECT itemEntry, enchant, socket1, socket2, socket3, bonusEnchant, prismaticEnchant "
                "FROM template_npc_horde WHERE playerClass = '%s' AND playerSpec = '%s' AND pos = '%u';", GetClassString(player).c_str(), playerSpecStr.c_str(), equipmentSlot);

                if (!result)
                continue;

                Field* fields = result->Fetch(); 
                uint32 itemEntry = fields[0].GetUInt32(); 
                uint32 enchant = fields[1].GetUInt32(); 
                uint32 socket1 = fields[2].GetUInt32(); 
                uint32 socket2 = fields[3].GetUInt32(); 
                uint32 socket3 = fields[4].GetUInt32(); 
                uint32 bonusEnchant = fields[5].GetUInt32(); 
                uint32 prismaticEnchant = fields[6].GetUInt32();
                // Equip Gear
                player->EquipNewItem(equipmentSlot, itemEntry, true);
                // Apply Enchants and Gems
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), PERM_ENCHANTMENT_SLOT, enchant);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), SOCK_ENCHANTMENT_SLOT, socket1);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), SOCK_ENCHANTMENT_SLOT_2, socket2);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), SOCK_ENCHANTMENT_SLOT_3, socket3);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), BONUS_ENCHANTMENT_SLOT, bonusEnchant);
                ApplyBonus(player, player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot), PRISMATIC_ENCHANTMENT_SLOT, prismaticEnchant);
            }
        }
    }

    void LearnPlateMailSpells(Player* player)
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
            player->LearnSpell(MAIL, true);
            break;
        default:
            break;
        }
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        switch (player->getClass())
        {
        case CLASS_PRIEST:
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_holy_wordfortitude:25:25:-15:0|t|r Послушание", GOSSIP_SENDER_MAIN, 0);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_holy_holybolt:25:25:-15:0|t|r Свет", GOSSIP_SENDER_MAIN, 1);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_shadow_shadowwordpain:25:25:-15:0|t|r Тьма", GOSSIP_SENDER_MAIN, 2);
                player->ADD_GOSSIP_ITEM(0, "####### Стартовые вещи! #######", GOSSIP_SENDER_MAIN, 100);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_bg_captureflag_eos:25:25:-15:0|t PvP сеты", GOSSIP_SENDER_MAIN, 30);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_arena_2v2_1:25:25:-15:0|t PvP Оружия", GOSSIP_SENDER_MAIN, 31);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Ability_kick:25:25:-15:0|t PvP Оффсет", GOSSIP_SENDER_MAIN, 32);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_chest_chain_11:25:25:-15:0|t PvE сеты", GOSSIP_SENDER_MAIN, 33);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_weapon_halberd17:25:25:-15:0|t PvE Оружие", GOSSIP_SENDER_MAIN, 34);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_bracer_01:25:25:-15:0|t PvE Оффсет", GOSSIP_SENDER_MAIN, 35);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_misc_ahnqirajtrinket_01:25:25:-15:0|t Аксессуары", GOSSIP_SENDER_MAIN, 36);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_relics_libramofhope:25:25:-15:0|t Реликвии", GOSSIP_SENDER_MAIN, 37);                
            }
            break;
        case CLASS_PALADIN:
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_holy_holybolt:25:25:-15:0|t|r Свет", GOSSIP_SENDER_MAIN, 3);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_holy_devotionaura:25:25:-15:0|t|r Защита", GOSSIP_SENDER_MAIN, 4);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_holy_auraoflight:25:25:-15:0|t|r Воздаяние", GOSSIP_SENDER_MAIN, 5);
                player->ADD_GOSSIP_ITEM(0, "####### Стартовые вещи! #######", GOSSIP_SENDER_MAIN, 100);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_bg_captureflag_eos:25:25:-15:0|t PvP сеты", GOSSIP_SENDER_MAIN, 30);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_arena_2v2_1:25:25:-15:0|t PvP Оружия", GOSSIP_SENDER_MAIN, 31);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Ability_kick:25:25:-15:0|t PvP Оффсет", GOSSIP_SENDER_MAIN, 32);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_chest_chain_11:25:25:-15:0|t PvE сеты", GOSSIP_SENDER_MAIN, 33);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_weapon_halberd17:25:25:-15:0|t PvE Оружие", GOSSIP_SENDER_MAIN, 34);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_bracer_01:25:25:-15:0|t PvE Оффсет", GOSSIP_SENDER_MAIN, 35);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_misc_ahnqirajtrinket_01:25:25:-15:0|t Аксессуары", GOSSIP_SENDER_MAIN, 36);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_relics_libramofhope:25:25:-15:0|t Реликвии", GOSSIP_SENDER_MAIN, 37);                
            }
            break;
        case CLASS_WARRIOR:
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\ability_rogue_eviscerate:25:25:-15:0|t|r Оружие", GOSSIP_SENDER_MAIN, 7);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\ability_warrior_innerrage:25:25:-15:0|t|r Неистовство", GOSSIP_SENDER_MAIN, 6);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\ability_warrior_defensivestance:25:25:-15:0|t|r Защита", GOSSIP_SENDER_MAIN, 8);
                player->ADD_GOSSIP_ITEM(0, "####### Стартовые вещи! #######", GOSSIP_SENDER_MAIN, 100);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_bg_captureflag_eos:25:25:-15:0|t PvP сеты", GOSSIP_SENDER_MAIN, 30);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_arena_2v2_1:25:25:-15:0|t PvP Оружия", GOSSIP_SENDER_MAIN, 31);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Ability_kick:25:25:-15:0|t PvP Оффсет", GOSSIP_SENDER_MAIN, 32);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_chest_chain_11:25:25:-15:0|t PvE сеты", GOSSIP_SENDER_MAIN, 33);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_weapon_halberd17:25:25:-15:0|t PvE Оружие", GOSSIP_SENDER_MAIN, 34);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_bracer_01:25:25:-15:0|t PvE Оффсет", GOSSIP_SENDER_MAIN, 35);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_misc_ahnqirajtrinket_01:25:25:-15:0|t Аксессуары", GOSSIP_SENDER_MAIN, 36);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_relics_libramofhope:25:25:-15:0|t Реликвии", GOSSIP_SENDER_MAIN, 37);                
            }
            break;
        case CLASS_MAGE:
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_holy_magicalsentry:25:25:-15:0|t|r Тайная магия", GOSSIP_SENDER_MAIN, 9);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_fire_flamebolt:25:25:-15:0|t|r Огонь", GOSSIP_SENDER_MAIN, 10);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_frost_frostbolt02:25:25:-15:0|t|r Лёд", GOSSIP_SENDER_MAIN, 11);
                player->ADD_GOSSIP_ITEM(0, "####### Стартовые вещи! #######", GOSSIP_SENDER_MAIN, 100);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_bg_captureflag_eos:25:25:-15:0|t PvP сеты", GOSSIP_SENDER_MAIN, 30);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_arena_2v2_1:25:25:-15:0|t PvP Оружия", GOSSIP_SENDER_MAIN, 31);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Ability_kick:25:25:-15:0|t PvP Оффсет", GOSSIP_SENDER_MAIN, 32);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_chest_chain_11:25:25:-15:0|t PvE сеты", GOSSIP_SENDER_MAIN, 33);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_weapon_halberd17:25:25:-15:0|t PvE Оружие", GOSSIP_SENDER_MAIN, 34);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_bracer_01:25:25:-15:0|t PvE Оффсет", GOSSIP_SENDER_MAIN, 35);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_misc_ahnqirajtrinket_01:25:25:-15:0|t Аксессуары", GOSSIP_SENDER_MAIN, 36);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_relics_libramofhope:25:25:-15:0|t Реликвии", GOSSIP_SENDER_MAIN, 37);                
            }
            break;
        case CLASS_WARLOCK:
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_shadow_deathcoil:25:25:-15:0|t|r Колдовство", GOSSIP_SENDER_MAIN, 12);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_shadow_metamorphosis:25:25:-15:0|t|r Демонология", GOSSIP_SENDER_MAIN, 13);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_shadow_rainoffire:25:25:-15:0|t|r Разрушение", GOSSIP_SENDER_MAIN, 14);
                player->ADD_GOSSIP_ITEM(0, "####### Стартовые вещи! #######", GOSSIP_SENDER_MAIN, 100);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_bg_captureflag_eos:25:25:-15:0|t PvP сеты", GOSSIP_SENDER_MAIN, 30);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_arena_2v2_1:25:25:-15:0|t PvP Оружия", GOSSIP_SENDER_MAIN, 31);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Ability_kick:25:25:-15:0|t PvP Оффсет", GOSSIP_SENDER_MAIN, 32);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_chest_chain_11:25:25:-15:0|t PvE сеты", GOSSIP_SENDER_MAIN, 33);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_weapon_halberd17:25:25:-15:0|t PvE Оружие", GOSSIP_SENDER_MAIN, 34);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_bracer_01:25:25:-15:0|t PvE Оффсет", GOSSIP_SENDER_MAIN, 35);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_misc_ahnqirajtrinket_01:25:25:-15:0|t Аксессуары", GOSSIP_SENDER_MAIN, 36);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_relics_libramofhope:25:25:-15:0|t Реликвии", GOSSIP_SENDER_MAIN, 37);               
            }
            break;
        case CLASS_SHAMAN:
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_nature_lightning:25:25:-15:0|t|r Стихии", GOSSIP_SENDER_MAIN, 15);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_nature_lightningshield:25:25:-15:0|t|r Совершенствование", GOSSIP_SENDER_MAIN, 16);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_nature_magicimmunity:25:25:-15:0|t|r Исцеление", GOSSIP_SENDER_MAIN, 17);
                player->ADD_GOSSIP_ITEM(0, "####### Стартовые вещи! #######", GOSSIP_SENDER_MAIN, 100);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_bg_captureflag_eos:25:25:-15:0|t PvP сеты", GOSSIP_SENDER_MAIN, 30);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_arena_2v2_1:25:25:-15:0|t PvP Оружия", GOSSIP_SENDER_MAIN, 31);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Ability_kick:25:25:-15:0|t PvP Оффсет", GOSSIP_SENDER_MAIN, 32);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_chest_chain_11:25:25:-15:0|t PvE сеты", GOSSIP_SENDER_MAIN, 33);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_weapon_halberd17:25:25:-15:0|t PvE Оружие", GOSSIP_SENDER_MAIN, 34);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_bracer_01:25:25:-15:0|t PvE Оффсет", GOSSIP_SENDER_MAIN, 35);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_misc_ahnqirajtrinket_01:25:25:-15:0|t Аксессуары", GOSSIP_SENDER_MAIN, 36);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_relics_libramofhope:25:25:-15:0|t Реликвии", GOSSIP_SENDER_MAIN, 37);
            }
            break;
        case CLASS_DRUID:
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_nature_starfall:25:25:-15:0|t|r Баланс", GOSSIP_SENDER_MAIN, 18);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\ability_racial_bearform:25:25:-15:0|t|r Сила зверя", GOSSIP_SENDER_MAIN, 19);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_nature_healingtouch:25:25:-15:0|t|r Исцеление", GOSSIP_SENDER_MAIN, 20);
                player->ADD_GOSSIP_ITEM(0, "####### Стартовые вещи! #######", GOSSIP_SENDER_MAIN, 100);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_bg_captureflag_eos:25:25:-15:0|t PvP сеты", GOSSIP_SENDER_MAIN, 30);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_arena_2v2_1:25:25:-15:0|t PvP Оружия", GOSSIP_SENDER_MAIN, 31);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Ability_kick:25:25:-15:0|t PvP Оффсет", GOSSIP_SENDER_MAIN, 32);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_chest_chain_11:25:25:-15:0|t PvE сеты", GOSSIP_SENDER_MAIN, 33);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_weapon_halberd17:25:25:-15:0|t PvE Оружие", GOSSIP_SENDER_MAIN, 34);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_bracer_01:25:25:-15:0|t PvE Оффсет", GOSSIP_SENDER_MAIN, 35);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_misc_ahnqirajtrinket_01:25:25:-15:0|t Аксессуары", GOSSIP_SENDER_MAIN, 36);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_relics_libramofhope:25:25:-15:0|t Реликвии", GOSSIP_SENDER_MAIN, 37);
            }
            break;
        case CLASS_HUNTER:
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\ability_hunter_beasttaming:25:25:-15:0|t|r Повелитель зверей", GOSSIP_SENDER_MAIN, 22);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\ability_marksmanship:25:25:-15:0|t|r Стрельба", GOSSIP_SENDER_MAIN, 21);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\ability_Hunter_swiftstrike:25:25:-15:0|t|r Выживание", GOSSIP_SENDER_MAIN, 23);
                player->ADD_GOSSIP_ITEM(0, "####### Стартовые вещи! #######", GOSSIP_SENDER_MAIN, 100);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_bg_captureflag_eos:25:25:-15:0|t PvP сеты", GOSSIP_SENDER_MAIN, 30);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_arena_2v2_1:25:25:-15:0|t PvP Оружия", GOSSIP_SENDER_MAIN, 31);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Ability_kick:25:25:-15:0|t PvP Оффсет", GOSSIP_SENDER_MAIN, 32);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_chest_chain_11:25:25:-15:0|t PvE сеты", GOSSIP_SENDER_MAIN, 33);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_weapon_halberd17:25:25:-15:0|t PvE Оружие", GOSSIP_SENDER_MAIN, 34);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_bracer_01:25:25:-15:0|t PvE Оффсет", GOSSIP_SENDER_MAIN, 35);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_misc_ahnqirajtrinket_01:25:25:-15:0|t Аксессуары", GOSSIP_SENDER_MAIN, 36);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_relics_libramofhope:25:25:-15:0|t Реликвии", GOSSIP_SENDER_MAIN, 37);
            }
            break;
        case CLASS_ROGUE:
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\ability_rogue_eviscerate:25:25:-15:0|t|r Ликвидация", GOSSIP_SENDER_MAIN, 24);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\ability_backstab:25:25:-15:0|t|r Бой", GOSSIP_SENDER_MAIN, 25);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\ability_stealth:25:25:-15:0|t|r Скрытность", GOSSIP_SENDER_MAIN, 26);
                player->ADD_GOSSIP_ITEM(0, "####### Стартовые вещи! #######", GOSSIP_SENDER_MAIN, 100);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_bg_captureflag_eos:25:25:-15:0|t PvP сеты", GOSSIP_SENDER_MAIN, 30);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_arena_2v2_1:25:25:-15:0|t PvP Оружия", GOSSIP_SENDER_MAIN, 31);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Ability_kick:25:25:-15:0|t PvP Оффсет", GOSSIP_SENDER_MAIN, 32);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_chest_chain_11:25:25:-15:0|t PvE сеты", GOSSIP_SENDER_MAIN, 33);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_weapon_halberd17:25:25:-15:0|t PvE Оружие", GOSSIP_SENDER_MAIN, 34);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_bracer_01:25:25:-15:0|t PvE Оффсет", GOSSIP_SENDER_MAIN, 35);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_misc_ahnqirajtrinket_01:25:25:-15:0|t Аксессуары", GOSSIP_SENDER_MAIN, 36);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_relics_libramofhope:25:25:-15:0|t Реликвии", GOSSIP_SENDER_MAIN, 37);
            }
            break;
        case CLASS_DEATH_KNIGHT:
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_deathknight_bloodpresence:25:25:-15:0|t|r Кровь", GOSSIP_SENDER_MAIN, 27);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_deathknight_frostpresence:25:25:-15:0|t|r Лёд", GOSSIP_SENDER_MAIN, 28);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "|cff00ff00|TInterface\\icons\\spell_deathknight_unholypresence:25:25:-15:0|t|r Нечестивость", GOSSIP_SENDER_MAIN, 29);
                player->ADD_GOSSIP_ITEM(0, "####### Стартовые вещи! #######", GOSSIP_SENDER_MAIN, 100);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_bg_captureflag_eos:25:25:-15:0|t PvP сеты", GOSSIP_SENDER_MAIN, 30);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Achievement_arena_2v2_1:25:25:-15:0|t PvP Оружия", GOSSIP_SENDER_MAIN, 31);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Ability_kick:25:25:-15:0|t PvP Оффсет", GOSSIP_SENDER_MAIN, 32);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_chest_chain_11:25:25:-15:0|t PvE сеты", GOSSIP_SENDER_MAIN, 33);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_weapon_halberd17:25:25:-15:0|t PvE Оружие", GOSSIP_SENDER_MAIN, 34);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_bracer_01:25:25:-15:0|t PvE Оффсет", GOSSIP_SENDER_MAIN, 35);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_misc_ahnqirajtrinket_01:25:25:-15:0|t Аксессуары", GOSSIP_SENDER_MAIN, 36);
                player->ADD_GOSSIP_ITEM(0, "|TInterface/ICONS/Inv_relics_libramofhope:25:25:-15:0|t Реликвии", GOSSIP_SENDER_MAIN, 37);
            }
            break;
        }
        player->SEND_GOSSIP_MENU(60025, creature->GetGUID());
        return true;
    }

    void EquipFullTemplateGear(Player* player, std::string& playerSpecStr) // Merge
    {
        if (CanEquipTemplate(player, playerSpecStr) == false)
        {
            player->GetSession()->SendAreaTriggerMessage("Нет вещей для %s специализации.", playerSpecStr.c_str());
            return;
        }

        EquipTmpGear(player, playerSpecStr);
        SetTemplateGlyphs(player, playerSpecStr);

        LearnPlateMailSpells(player);
        LearnWeaponSkills(player);
        player->UpdateSkillsToMaxSkillsForLevel();
        player->GetSession()->SendAreaTriggerMessage("Успешно одеты %s %s вещи!", playerSpecStr.c_str(), GetClassString(player).c_str());
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        if (!player || !creature)
        return true;

        switch (action)
        {
        case 30:
            player->GetSession()->SendListInventory(creature->GetGUID(), 100029);
            break;
        case 31:
            player->GetSession()->SendListInventory(creature->GetGUID(), 100031);
            break;
        case 32:
            player->GetSession()->SendListInventory(creature->GetGUID(), 100030);
            break;        
        case 33:
            player->GetSession()->SendListInventory(creature->GetGUID(), 100026);
            break;
        case 34:            
            player->GetSession()->SendListInventory(creature->GetGUID(), 100028);
            break;
        case 35:
            player->GetSession()->SendListInventory(creature->GetGUID(), 100027);
            break;    
        case 36:            
            player->GetSession()->SendListInventory(creature->GetGUID(), 100023);
            break;      
        case 37:            
            player->GetSession()->SendListInventory(creature->GetGUID(), 100025);
            break;             
        case 100:
            player->CLOSE_GOSSIP_MENU();
            break;
        case 0:
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }           
            spec = "Discipline";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 1: // Use Holy Priest Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Holy";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 2: // Use Shadow Priest Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Shadow";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 3: // Use Holy Paladin Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Holy";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 4: // Use Protection Paladin Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Protection";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 5: // Use Retribution Paladin Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Retribution";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 6: // Use Fury Warrior Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }           

            spec = "Fury";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 7: // Use Arms Warrior Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }           
            // Use Discipline Priest Spec
            spec = "Arms";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 8: // Use Protection Warrior Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }           
            // Use Discipline Priest Spec
            spec = "Protection";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 9: // Use Arcane Mage Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Arcane";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 10: // Use Fire Mage Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Fire";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 11: // Use Frost Mage Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Frost";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 12: // Use Affliction Warlock Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Affliction";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 13: // Use Demonology Warlock Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Demonology";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 14: // Use Destruction Warlock Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Destruction";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 15: // Use Elemental Shaman Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Elemental";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 16: // Use Enhancement Shaman Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Enhancement";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 17: // Use Restoration Shaman Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Restoration";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 18: // Use Ballance Druid Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Ballance";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 19: // Use Feral Druid Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Feral";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 20: // Use Restoration Druid Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Restoration";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 21: // Use Marksmanship Hunter Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Marksmanship";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 22: // Use Beastmastery Hunter Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Beastmastery";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 23: // Use Survival Hunter Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Survival";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 24: // Use Assassination Rogue Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Assassination";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 25: // Use Combat Rogue Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Combat";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 26: // Use Subtlety Rogue Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Subtlety";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 27: // Use Blood DK Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Blood";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 28: // Use Frost DK Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Frost";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        case 29: // Use Unholy DK Spec
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                if (Item* haveItemEquipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    if (haveItemEquipped)
                    {
                        player->GetSession()->SendAreaTriggerMessage("Вам нужно снять имеющиеся вещи!");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    } 
                }    
            }  
            spec = "Unholy";
            EquipFullTemplateGear(player, spec);
            player->CLOSE_GOSSIP_MENU();
            break;
        default: // Just in case
            player->GetSession()->SendAreaTriggerMessage("Нашли ошибку?Обратитесь к администрации.");
            break;
        }
        return true;
    }
};

enum SteelforgedDefender
 
{
    SPELL_DEFENDERS_SLAIN = 65387
 
};
 
class npc_steelforged_defender : public CreatureScript
{
    public:
        npc_steelforged_defender() : CreatureScript("npc_steelforged_defender") { }

        struct npc_steelforged_defenderAI : public BossAI
        {
            npc_steelforged_defenderAI(Creature* creature) : BossAI(creature, 0)
            {
            }

            void Reset() override
            {
                _Reset();
            }
			
            void EnterCombat(Unit* /*who*/) override
            {
            }
			
            void KilledUnit(Unit * /*victim*/) override
            {
            }
			
            void JustDied(Unit* killer/*victim*/) override
            {
				    killer->CastSpell(killer, SPELL_DEFENDERS_SLAIN, true);	
            }
            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;
					
                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
					
                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        default:
                            break;
                    }
                }
				
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_steelforged_defenderAI(creature);
        }
};

void AddSC_custom_scream()
{
    new ultimate_trainer_npc();
    new npc_teleportation();
    new npc_reagent();
    new npc_enchantment();
    new npc_eferial();
    new npc_template();
	new npc_clk;
	new npc_xazam;
	new npc_puhi;
	new npc_puhipve;
	new npc_rubik;
    new npc_steelforged_defender();
}