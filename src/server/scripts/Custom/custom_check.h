#include "ScriptMgr.h"

class SpellDamage
{
public:
    static SpellDamage* instance()
    {
        static SpellDamage instance;
        return &instance;
    }

    void Damage(uint32& damage, uint32 spellId)
    {
        if (DamageContainer.find(spellId) == DamageContainer.end())
        return;


        float val = DamageContainer[spellId];

        if (!val || val == 100.0f)
        return;

        damage = (damage / 100.0f) * val;
    }

    void LoadFromDB()
    {
        DamageContainer.clear();
        uint32 msTime = getMSTime();
        QueryResult result = WorldDatabase.Query("SELECT * FROM spell_damage");

        if (!result)
        return;

        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            DamageContainer[fields[0].GetUInt32()] = fields[1].GetFloat();
            ++count;
        } 
        while (result->NextRow());
        TC_LOG_INFO("server.loading", "Loaded %u spells in %u ms", count, GetMSTimeDiffToNow(msTime));
    }

private:
    std::unordered_map<uint32, float> DamageContainer; // spellid, percentage
};

#define sSpellDamage SpellDamage::instance()

class DamageLoader : public WorldScript
{
public:
    DamageLoader() : WorldScript("SpellDamageLoader") {}

    void OnStartup() override
    {
        sSpellDamage->LoadFromDB();
    }
};
