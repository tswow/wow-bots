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
#include <sol/sol.hpp>

#include <filesystem>
#include <map>
#include <vector>

namespace fs = std::filesystem;

class BotLuaState
{
public:
    BotLuaState(fs::path const& rootPath);
    void Start();
protected:
    virtual void LoadLibraries() = 0;
    sol::state m_state;
    fs::path m_rootPath;
    fs::path m_curPath;
    std::vector<fs::path> m_fileStack;
    bool m_alreadyErrored;
    std::map<fs::path, sol::table> m_modules;
    fs::path FindLuaModule(std::string target);
    sol::table require(std::string const& mod);
    void ExecuteFile(fs::path file);
    void LoadBaseLibraries();
};