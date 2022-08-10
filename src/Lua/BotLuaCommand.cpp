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
#include "BotLuaCommand.h"
#include "BotCommandMgr.h"
#include "BotMgr.h"
#include "BotLogging.h"
#include "BotLuaShared.h"
#include "Config.h"
#include "BotAccounts.h"

#include <filesystem>

namespace fs = std::filesystem;

BotLuaCommand::BotLuaCommand()
    : BotLuaState(fs::path(sConfigMgr->GetStringDefault("Lua.Path", "./")) / "commands")
{}

void BotLuaCommand::LoadLibraries()
{
    auto LBotAccount = m_state.new_usertype<BotAccount>("BotAccount");
    LBotAccount.set_function("GetUsername", &BotAccount::GetUsername);
    LBotAccount.set_function("GetPassword", &BotAccount::GetPassword);

    m_state.set_function("GetBotAccount", GetBotAccount);
    m_state.set_function("GetBotAccounts", [](sol::table table) {
        std::vector<uint32_t> vec;
        vec.reserve(table.size());
        for (auto& [key,value] : table)
        {
            vec.push_back(value.as<uint32_t>());
        }
        return sol::as_table(GetBotAccounts(vec));
    });

    auto LBotCommandBuilder = m_state.new_usertype<BotCommandBuilder>("BotCommandBuilder");
    LBotCommandBuilder.set_function("AddStringParam", sol::overload(
        [](BotCommandBuilder& builder, std::string const& name) { return builder.AddStringParam(name); },
        [](BotCommandBuilder& builder, std::string const& name, std::string const& def) { return builder.AddStringParam(name, def); }
    ));

    LBotCommandBuilder.set_function("AddNumberParam", sol::overload(
        [](BotCommandBuilder& builder, std::string const& name) { return builder.AddNumberParam(name); },
        [](BotCommandBuilder& builder, std::string const& name, double def) { return builder.AddNumberParam(name, def); }
    ));

    LBotCommandBuilder.set_function("AddBoolParam", sol::overload(
        [](BotCommandBuilder& builder, std::string const& name) { return builder.AddBoolParam(name); },
        [](BotCommandBuilder& builder, std::string const& name, bool def) { return builder.AddBoolParam(name, def); }
    ));

    LBotCommandBuilder.set_function("SetDescription", &BotCommandBuilder::SetDescription);

    LBotCommandBuilder.set_function("SetCallback", [](BotCommandBuilder& builder, sol::protected_function fn) {
        builder.SetCallback([=](BotCommandArguments const& args) {
            fn(args);
        });
    });

    m_state.set_function("CreateCommand", [](std::string const& name) {
        return sBotCommandMgr->CreateCommand(name);
    });

    auto LBotCommandArguments = m_state.new_usertype<BotCommandArguments>("BotCommandArguments");
    LBotCommandArguments.set_function("GetNumber", &BotCommandArguments::get_number);
    LBotCommandArguments.set_function("GetString", &BotCommandArguments::get_string);
    LBotCommandArguments.set_function("GetBool", &BotCommandArguments::get_bool);

    m_state.set_function("StartBot", sol::overload(
        [](std::string const& username, std::string const& password, std::string const& events, std::string const& authserver) {
            StartBot(username, password, events, authserver);
        },
        [](std::string const& username, std::string const& password, std::string const& events) {
            StartBot(username, password, events);
        },
        [](std::string const& username, std::string const& password) {
            StartBot(username, password);
        }
    ));
    m_state.set_function("StopBot", StopBot);
}
