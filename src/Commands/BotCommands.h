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

#include "BotCommandMgr.h"
#include "BotMgr.h"
#include "BotProfile.h"

void BotCommandMgr::RegisterBaseCommands()
{
    { // Login
        std::string USERNAME = "username";
        std::string PASSWORD = "password";
        std::string EVENTS = "events";
        std::string AUTHSERVER = "authserver";
    
        CreateCommand("start")
            .SetDescription("Starts a bot and logs in to the game")
            .AddStringParam(USERNAME)
            .AddStringParam(PASSWORD)
            .AddStringParam(EVENTS, ROOT_EVENT_NAME)
            .AddStringParam(AUTHSERVER, "127.0.0.1")
            .SetCallback([=](BotCommandArguments const& args) {
                std::string username = args.get_string(USERNAME);
                std::string password = args.get_string(PASSWORD);
                std::string events = args.get_string(EVENTS);
                std::string authserver = args.get_string(AUTHSERVER);
                StartBot(username, password, events, authserver);
            })
            ;
    }
}