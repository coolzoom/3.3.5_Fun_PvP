/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
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

#ifndef _ARENATEAMMGR_H
#define _ARENATEAMMGR_H

#include "ArenaTeam.h"

class TC_GAME_API ArenaTeamMgr
{
private:
    ArenaTeamMgr();
    ~ArenaTeamMgr();

public:
    static ArenaTeamMgr* instance();

    typedef std::unordered_map<uint32, ArenaTeam*> ArenaTeamContainer;

    ArenaTeam* GetArenaTeamById(uint32 arenaTeamId) const;
    ArenaTeam* GetArenaTeamByName(std::string const& arenaTeamName) const;
    ArenaTeam* GetArenaTeamByCaptain(ObjectGuid guid) const;

    void LoadArenaTeams();
    void AddArenaTeam(ArenaTeam* arenaTeam);
    void RemoveArenaTeam(uint32 Id);

    ArenaTeamContainer::iterator GetArenaTeamMapBegin() { return ArenaTeamStore.begin(); }
    ArenaTeamContainer::iterator GetArenaTeamMapEnd()   { return ArenaTeamStore.end(); }

    void DistributeArenaPoints();

    uint32 GenerateArenaTeamId();
    uint32 GenerateTempArenaTeamId();
    void SetNextArenaTeamId(uint32 Id) { NextArenaTeamId = Id; }

protected:
    uint32 NextArenaTeamId;
    uint32 NextTempArenaTeamId;
    ArenaTeamContainer ArenaTeamStore;
};

#define sArenaTeamMgr ArenaTeamMgr::instance()

#endif
