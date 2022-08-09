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
#include "BotMapDataMgr.h"
#include "BotLogging.h"

#include "Config.h"
#include "Map.h"
#include "VMapFactory.h"
#include "VMapManager2.h"

#include <filesystem>
namespace fs = std::filesystem;

void BotMapDataMgr::Setup()
{

}

MapCoord BotMapDataMgr::GetMapCoord(uint32 map, float x, float y)
{
    int gx = (int)(CENTER_GRID_ID - x / SIZE_OF_GRIDS);                       //grid x
    int gy = (int)(CENTER_GRID_ID - y / SIZE_OF_GRIDS);                       //grid y
    return MapCoord { uint32_t(map),gx,gy };
}


GridMap* BotMapDataMgr::LoadGridMap(uint32 map, float x, float y)
{
    MapCoord c = GetMapCoord(map, x, y);
    auto itr = m_maps.find(c);
    if (itr == m_maps.end())
    {
        int len = strlen("maps/%03u%02u%02u.map") + 1;
        std::string tmp;
        tmp.resize(len);
        snprintf(tmp.data(), len, (char*)"maps/%03u%02u%02u.map", map, c.x, c.y);
        fs::path mapPath = fs::path(sConfigMgr->GetStringDefault("Data.Path", "./")) / tmp;
        if (fs::exists(mapPath))
        {
            GridMap* map = (m_maps[c] = std::make_unique<GridMap>()).get();
            map->loadData(mapPath.string().c_str());
            return map;
        }
        m_maps[c] = nullptr;
        return nullptr;
    }
    return itr->second.get();
}

bool BotMapDataMgr::LoadVMap(uint32 map, float x, float y)
{
    MapCoord c = GetMapCoord(map, x, y);
    auto itr = m_vmaps.find(c);
    if (itr != m_vmaps.end())
    {
        return itr->second;
    }

    fs::path vmapDir = fs::path(sConfigMgr->GetStringDefault("Data.Path", "./")) / "vmaps";
    int vmapLoadResult = VMAP::VMapFactory::createOrGetVMapManager()->loadMap(vmapDir.string().c_str(), map, c.x,c.y);
    switch (vmapLoadResult)
    {
    case VMAP::VMAP_LOAD_RESULT_OK:
        BOT_LOG_DEBUG("maps", "VMAP loaded id:%d, x:%d, y:%d", map, c.x,c.y);
        return (m_vmaps[c] = true);
    case VMAP::VMAP_LOAD_RESULT_ERROR:
        BOT_LOG_ERROR("maps", "Could not load VMAP, id:%d, x:%d, y:%d", map, c.x, c.y);
        return (m_vmaps[c] = false);
    case VMAP::VMAP_LOAD_RESULT_IGNORED:
        BOT_LOG_DEBUG("maps", "Ignored VMAP, id:%d, x:%d, y:%d", map, c.x, c.y);
        return (m_vmaps[c] = false);
    default:
        BOT_LOG_ERROR("maps", "Unknown VMapError %d: id:%d, x:%d, y:%d", vmapLoadResult, map, c.x,c.y);
        return (m_vmaps[c] = false);
    }
}

float BotMapDataMgr::GetHeight(uint32 map, float x, float y)
{
    GridMap* grid = LoadGridMap(map, x, y);
    return grid ? grid->getHeight(x, y) : 0;
}

float BotMapDataMgr::GetVMapHeight(uint32 map, float x, float y, float z, float maxSearchDist)
{
    return LoadVMap(map,x,y)
        ? VMAP::VMapFactory::createOrGetVMapManager()->getHeight(map,x,y,z,maxSearchDist)
        : 0;
}

bool BotMapDataMgr::IsInLineOfSight(uint32 mapId, float x1, float y1, float z1, float x2, float y2, float z2, uint32_t ignoreFlags)
{
    return (LoadVMap(mapId, x1, y1) && LoadVMap(mapId, x2,y2))
        ? VMAP::VMapFactory::createOrGetVMapManager()->isInLineOfSight(mapId, x1, y1, z1, x2, y2, z2, VMAP::ModelIgnoreFlags(ignoreFlags))
        : 0;
}

BotMapDataMgr* BotMapDataMgr::instance()
{
    static BotMapDataMgr mgr;
    return &mgr;
}
