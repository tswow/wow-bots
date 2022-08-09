/*
 * This file is part of the wotlk-bots project <https://github.com/tswow/wotlk-bots>.
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
#pragma once

#include "PacketTypes.h"

#include <map>
#include <memory>

class GridMap;
struct MapCoord
{
    uint32_t map;
    int x;
    int y;
    bool operator< (const MapCoord& rhs) const
    {
        if (rhs.map <= map) return false;
        if (rhs.x <= x) return false;
        return y > rhs.y;
    }
};

class BotMapDataMgr
{
    std::map<MapCoord, std::unique_ptr<GridMap>> m_maps;
    std::map<MapCoord, bool> m_vmaps;
private:
    GridMap* LoadGridMap(uint32 map, float x, float y);
    bool LoadVMap(uint32 map, float x, float y);
    MapCoord GetMapCoord(uint32 map, float x, float y);
public:
    void Setup();
    float GetHeight(uint32 map, float x, float y);
    float GetVMapHeight(uint32 map, float x, float y, float z, float maxSearchDist);
    bool IsInLineOfSight(uint32 map, float x1, float y1, float z1, float x2, float y2, float z2, uint32_t ignoreFlags);
    static BotMapDataMgr* instance();
};

#define sBotMapDataMgr BotMapDataMgr::instance()