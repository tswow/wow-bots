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

#include "BotOpcodes.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

#define _LBOT_LOG(state,level) state.set_function("BOT_" #level,\
    [](std::string const& cat, std::string const& fmt, sol::variadic_args args) \
    {\
        LBotLog(LogLevel::level,cat,fmt,args);\
    })\

static bool ends_with(std::string const& value, std::string const& ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

static std::filesystem::path search_from(std::filesystem::path const& root, std::string const& target)
{
    std::filesystem::path candidate = root / target;
    if (std::filesystem::exists(candidate))
    {
        return candidate;
    }

    for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(root))
    {
        std::string entry = dir_entry.path().string();
        std::replace(entry.begin(), entry.end(), '\\', '/');
        if (ends_with(entry, target))
        {
            return dir_entry.path();
        }
    }
    return "";
}

fs::path BotLuaState::FindLuaModule(std::string target)
{
    std::replace(target.begin(), target.end(), '.', '/');
    std::replace(target.begin(), target.end(), '\\', '/');
    target += ".lua";
    std::filesystem::path candidate = search_from(m_curPath, target);
    return candidate.empty() ? search_from(m_rootPath, target) : candidate;
}

BotLuaState::BotLuaState(fs::path const& rootPath)
    : m_rootPath(rootPath)
{}

void BotLuaState::ExecuteFile(fs::path file)
{
    file = std::filesystem::absolute(file);
    if (m_alreadyErrored)
    {
        return;
    }

    if (m_modules.find(file) != m_modules.end())
    {
        return;
    }

    m_fileStack.push_back(file);
    sol::protected_function_result res;

    res = m_state.safe_script_file(file.string(), &sol::script_pass_on_error);
    if (!res.valid())
    {
        if (!m_alreadyErrored)
        {
            sol::error err = res;
            BOT_LOG_ERROR("Lua", err.what());
        }
        m_alreadyErrored = true;
        return;
    }

    m_modules[file] = res.get_type() == sol::type::table
        ? res.get<sol::table>()
        : m_state.create_table();
    m_fileStack.pop_back();
}

sol::table BotLuaState::require(std::string const& mod)
{
    if (mod == "lualib_bundle")
    {
        return m_modules["lualib_bundle"];
    }

    std::filesystem::path path = std::filesystem::absolute(FindLuaModule(mod));
    if (path.empty())
    {
        throw std::runtime_error("Could not find module " + mod);
    }

    if (std::find(m_fileStack.begin(), m_fileStack.end(), path) != m_fileStack.end())
    {
        std::string error_str = "";
        for (std::filesystem::path const& file : m_fileStack)
        {
            error_str += "    " + file.string() + "->\n";
        }
        error_str += "    " + path.string();
        throw std::runtime_error("Circular dependency:" + error_str);
    }

    auto itr = m_modules.find(path);
    if (itr == m_modules.end())
    {
        ExecuteFile(path);
        itr = m_modules.find(path);
        return itr != m_modules.end() ? itr->second : m_state.create_table();
    }
    else
    {
        return itr->second;
    }
    return m_modules[path];
}

void BotLuaState::Start()
{
    m_modules.clear();
    m_state = sol::state();
    m_alreadyErrored = false;
    LoadBaseLibraries();
    LoadLibraries();
    
    fs::path lualibBundle = m_rootPath / "lualib_bundle.lua";
    if (fs::exists(lualibBundle))
    {
        m_modules["lualib_bundle"] = m_state.safe_script_file(lualibBundle.string());
    }
    else
    {
        m_modules["lualib_bundle"] = m_state.create_table();
    }

    for (auto const& file : std::filesystem::recursive_directory_iterator(m_rootPath))
    {
        if (file.is_regular_file() && file.path().extension() == ".lua")
        {
            m_curPath = file.path().parent_path();
            // don't load any accidental lualib_bundles
            if (file.path().filename() == "lualib_bundle.lua")
            {
                continue;
            }
            try
            {
                ExecuteFile(file);
            }
            catch (std::exception const& e)
            {
                std::cerr << e.what() << "\n";
            }
            catch (...)
            {
                std::cerr << "Unknown Lua exception\n";
            }
        }
    }
}

void BotLuaState::LoadBaseLibraries()
{
    m_state.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os, sol::lib::table, sol::lib::string, sol::lib::io);
    _LBOT_LOG(m_state, LOG_TRACE);
    _LBOT_LOG(m_state, LOG_DEBUG);
    _LBOT_LOG(m_state, LOG_INFO);
    _LBOT_LOG(m_state, LOG_WARN);
    _LBOT_LOG(m_state, LOG_ERROR);

    m_state.set_function("require", [this](std::string target) {
        return require(target);
    });

    m_state.set_function("print", LBotPrint);
    m_state.set_function("GetHeight", [](uint32_t map, float x, float y) {
        return sBotMapDataMgr->GetHeight(map, x, y);
    });

    m_state.set_function("GetVMapHeight", [](int map, float x, float y, float z, float maxSearchDist) {
        return sBotMapDataMgr->GetVMapHeight(map, x, y, z, maxSearchDist);
    });

    m_state.set_function("IsInLineOfSight", [](unsigned int mapId, float x1, float y1, float z1, float x2, float y2, float z2, uint32_t ignoreFlags) {
        return sBotMapDataMgr->IsInLineOfSight(mapId, x1, y1, z2, x2, y2, z2, ignoreFlags);
    });

    m_state.set_function("ReadDir", 
        [this](std::string const& dir) {
            sol::table table = m_state.create_table();
            for (const fs::directory_entry& dir_entry :
                fs::directory_iterator(dir))
            {
                table.add(dir_entry.path().string());
            }
            return table;
        }
    );

    m_state.set_function("ReadDirRecursive", 
        [this](std::string const& dir) {
            sol::table table = m_state.create_table();
            for (const fs::directory_entry& dir_entry :
                fs::recursive_directory_iterator(dir))
            {
                table.add(dir_entry.path().string());
            }
            return table;
        }
    );

    m_state.set_function("IsFile", [](std::string const& path) {
        return fs::is_regular_file(path);
    });

    m_state.set_function("IsDirectory", [](std::string const& path) {
        return fs::is_directory(path);
    });

    m_state.set_function("WriteFile", [](std::string const& path, std::string const& value) {
        fs::path pPath = path;
        if (!fs::exists(pPath.parent_path())) {
            fs::create_directories(pPath.parent_path());
        }
        std::ofstream ofs(path);
        ofs << value;
    });

    m_state.set_function("ReadFile", [](std::string const& path) {
        fs::path pPath = path;
        if (!fs::exists(pPath.parent_path())) {
            return std::string("");
        }
        std::ifstream ifs(path);
        std::stringstream sstream;
        sstream << ifs.rdbuf();
        return sstream.str();
    });

    m_state.set_function("OpcodeString", [](double opcode) {
        return OpcodeString(Opcodes(opcode));
    });
}