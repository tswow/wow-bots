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
#include "BotLuaShared.h"
#include "BotLogging.h"
#include "Map/BotMapDataMgr.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

#define _LBOT_LOG(state,level) state.set_function("BOT_" #level,\
    [](std::string const& cat, std::string const& fmt, sol::variadic_args args) \
    {\
        LBotLog(LogLevel::level,cat,fmt,args);\
    })\

void RegisterSharedLua(sol::state & state)
{
    state.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os, sol::lib::table, sol::lib::string, sol::lib::io);
    _LBOT_LOG(state, LOG_TRACE);
    _LBOT_LOG(state, LOG_DEBUG);
    _LBOT_LOG(state, LOG_INFO);
    _LBOT_LOG(state, LOG_WARN);
    _LBOT_LOG(state, LOG_ERROR);
    state.set_function("print", LBotPrint);
    state.set_function("GetHeight", [](uint32_t map, float x, float y) {
        return sBotMapDataMgr->GetHeight(map, x, y);
    });

    state.set_function("GetVMapHeight", [](int map, float x, float y, float z, float maxSearchDist) {
        return sBotMapDataMgr->GetVMapHeight(map, x, y, z, maxSearchDist);
    });

    state.set_function("IsInLineOfSight", [](unsigned int mapId, float x1, float y1, float z1, float x2, float y2, float z2, uint32_t ignoreFlags) {
        return sBotMapDataMgr->IsInLineOfSight(mapId, x1, y1, z2, x2, y2, z2, static_cast<VMAP::ModelIgnoreFlags>(ignoreFlags));
    });

    state.set_function("ReadDir", 
        [&state](std::string const& dir) {
            sol::table table = state.create_table();
            for (const fs::directory_entry& dir_entry :
                fs::directory_iterator(dir))
            {
                table.add(dir_entry.path().string());
            }
            return table;
        }
    );

    state.set_function("ReadDirRecursive", 
        [&state](std::string const& dir) {
            sol::table table = state.create_table();
            for (const fs::directory_entry& dir_entry :
                fs::recursive_directory_iterator(dir))
            {
                table.add(dir_entry.path().string());
            }
            return table;
        }
    );

    state.set_function("IsFile", [](std::string const& path) {
        return fs::is_regular_file(path);
    });

    state.set_function("IsDirectory", [](std::string const& path) {
        return fs::is_directory(path);
    });

    state.set_function("WriteFile", [](std::string const& path, std::string const& value) {
        fs::path pPath = path;
        if (!fs::exists(pPath.parent_path())) {
            fs::create_directories(pPath.parent_path());
        }
        std::ofstream ofs(path);
        ofs << value;
    });

    state.set_function("ReadFile", [](std::string const& path) {
        fs::path pPath = path;
        if (!fs::exists(pPath.parent_path())) {
            return std::string("");
        }
        std::ifstream ifs(path);
        std::stringstream sstream;
        sstream << ifs.rdbuf();
        return sstream.str();
    });
}