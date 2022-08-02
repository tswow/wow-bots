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
#include "BotMain.h"
#include "BotMgr.h"
#include "BotCommandMgr.h"
#include "BotProfile.h"
#include "BotLogging.h"
#include "Map/BotMapDataMgr.h"

#include "Config.h"

#include <filesystem>
#include <fstream>

void TC_BOT_API BotMain()
{
    std::string error;
    if (!std::filesystem::exists("bots.conf"))
    {
        std::ofstream conf("bots.conf");
    }
    if (!sConfigMgr->LoadInitial("bots.conf", {}, error))
    {
        BOT_LOG_ERROR("main","Loading configuration error: %s",error.c_str());
    }

    sBotMapDataMgr->Setup();
    sBotMgr->Initialize();
    sBotCommandMgr->Reload();
    if (sConfigMgr->GetBoolDefault("Console.Enable", true))
    {
        for (;;)
        {
            std::string input;
            std::getline(std::cin, input);
            if(input == "reload")
            {
                sBotCommandMgr->Reload();
                sBotMgr->Reload();
            }
            else
            {
                sBotCommandMgr->Fire(input);
            }
        }
    }
}