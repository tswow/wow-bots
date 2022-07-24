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
#include "Config.h"
#include "BotLogging.h"

#include "VMapFactory.h"
#include "VMapManager2.h"

#include "thread_pool.hpp"
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

void BotMapDataMgr::Setup()
{
    fs::path dataDir = fs::path(sConfigMgr->GetStringDefault("DataDir", "./"));
    fs::path mapsDir = dataDir / "maps";
    fs::path vmapsDir = dataDir / "vmaps";

    thread_pool pool;
    if (fs::exists(mapsDir))
    {
        std::mutex map_mutex;
        for (const fs::directory_entry& entry :
            fs::directory_iterator(mapsDir))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".map")
            {
                continue;
            }
            //pool.push_task([=,&map_mutex]() {
            std::string name = entry.path().filename().string();
            uint32_t map = std::stoi(name.substr(0, 3));
            uint32_t x = std::stoi(name.substr(3, 2));
            uint32_t y = std::stoi(name.substr(5, 2));
            //BOT_LOG_TRACE("maps", "Loading tile %i %i %i", map, x, y);
            GridMap* gridMap;
            {
                std::scoped_lock lock(map_mutex);
                gridMap = (m_maps[MapCoord{ map, x, y }] = std::make_unique<GridMap>()).get();
            }
            gridMap->loadData(entry.path().string().c_str());
            //});
        }
    }
    pool.wait_for_tasks();

    VMAP::VMapManager2* mgr = VMAP::VMapFactory::createOrGetVMapManager();
    if (fs::exists(vmapsDir))
    {
        for (const fs::directory_entry& entry : fs::directory_iterator(vmapsDir))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".vmtile")
            {
                continue;
            }
            
            std::string name = entry.path().filename().string();
            uint32_t map = std::stoi(name.substr(0, 3));
            uint32_t x = std::stoi(name.substr(4, 2));
            uint32_t y = std::stoi(name.substr(7, 2));
            mgr->loadMap(vmapsDir.string().c_str(), map, x, y);
        }
    }
}

float BotMapDataMgr::GetVMapHeight(int map, float x, float y, float z, float maxSearchDist)
{
    return VMAP::VMapFactory::createOrGetVMapManager()->getHeight(map, x, y, z, maxSearchDist);
}

bool BotMapDataMgr::IsInLineOfSight(unsigned int mapId, float x1, float y1, float z1, float x2, float y2, float z2, VMAP::ModelIgnoreFlags ignoreFlags)
{
    return VMAP::VMapFactory::createOrGetVMapManager()->isInLineOfSight(mapId, x1, y1, z1, x2, y2, z2, ignoreFlags);
}


float BotMapDataMgr::GetHeight(int map, float x, float y)
{
    int gx = (int)(CENTER_GRID_ID - x / SIZE_OF_GRIDS);                       //grid x
    int gy = (int)(CENTER_GRID_ID - y / SIZE_OF_GRIDS);                       //grid y
    MapCoord c = { map,gx,gy };
    auto itr = m_maps.find(c);
    if (itr == m_maps.end())
    {
        return 0;
    }
    return itr->second->getHeight(x, y);
}

BotMapDataMgr* BotMapDataMgr::instance()
{
    static BotMapDataMgr mgr;
    return &mgr;
}

