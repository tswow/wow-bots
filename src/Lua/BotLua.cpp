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
#include "BotLua.h"
#include "Bot.h"
#include "BotEvents.h"
#include "BotMutable.h"
#include "BotLuaShared.h"
#include "BotLogging.h"
#include "Config.h"

#include <vector>

namespace fs = std::filesystem;

template <typename T>
void RegisterPacket(std::string const& name, sol::state& state)
{
    auto type = state.new_usertype<T>(name);
    type.set_function("WriteUInt8", &T::WriteUInt8);
    type.set_function("WriteInt8", &T::WriteInt8);
    type.set_function("WriteUInt16", &T::WriteUInt16);
    type.set_function("WriteInt16", &T::WriteInt16);
    type.set_function("WriteUInt32", &T::WriteUInt32);
    type.set_function("WriteInt32", &T::WriteInt32);
    type.set_function("WriteUInt64", &T::WriteUInt64);
    type.set_function("WriteInt64", &T::WriteInt64);
    type.set_function("WriteFloat", &T::WriteFloat);
    type.set_function("WriteDouble", &T::WriteDouble);
    type.set_function("WriteString", &T::WriteString);
    type.set_function("WriteCString", &T::WriteCString);
    type.set_function("WriteBytes", [](T& target, sol::table table) {
        std::vector<uint8_t> vec;
        vec.reserve(table.size());
        for (size_t i = 1; i <= table.size(); ++i)
        {
            vec.push_back(table.get<uint8_t>(i));
        }
        return target.WriteBytes<std::vector<uint8_t>>(vec);
    });
}

void BotLua::Reload(BotThread* thread)
{
    m_state = sol::state();
    RegisterSharedLua(m_state);
    auto LBotEvents = m_state.new_usertype<BotEvents>("BotEvents");
    LBotEvents.set_function("OnWorldPacket", sol::overload(&BotEvents::_LOnWorldPacket,&BotEvents::LidOnWorldPacket));
    LBotEvents.set_function("OnCreate", &BotEvents::LOnCreate);
    LBotEvents.set_function("OnAuthChallenge", &BotEvents::LOnAuthChallenge);
    LBotEvents.set_function("OnAuthProof", &BotEvents::LOnAuthProof);
    LBotEvents.set_function("OnRequestRealms", &BotEvents::LOnRequestRealms);
    LBotEvents.set_function("OnSelectRealm", &BotEvents::LOnSelectRealm);
    LBotEvents.set_function("OnCloseAuthConnection", &BotEvents::LOnCloseAuthConnection);
    LBotEvents.set_function("OnWorldAuthChallenge", &BotEvents::LOnWorldAuthChallenge);
    LBotEvents.set_function("OnWorldAuthResponse", &BotEvents::LOnWorldAuthResponse);
    LBotEvents.set_function("Register", &BotEvents::Register);

    RegisterPacket<WorldPacket>("WorldPacket", m_state);
    RegisterPacket<AuthPacket>("AuthPacket", m_state);

    m_state.set_function("CreateWorldPacket", sol::overload(
        [](uint32_t opcode, uint32_t reserve) {
            return WorldPacket(Opcodes(opcode), reserve);
        },
        [](uint32_t opcode)
        {
            return WorldPacket(Opcodes(opcode));
        }
    ));

    m_state.set_function("CreateAuthPacket", sol::overload(
        [](uint32 size) { return AuthPacket(size); },
        []() { return AuthPacket(); }
    ));

    m_state.set("RootBot", BotEvents(thread->m_events->GetRootEvent()));
    m_state.set_function("CreateBot", sol::overload(
        [thread](sol::table parentsTable) {
            std::vector<BotEvents> parents;
            for (auto [key, value] : parentsTable)
            {
                if (value.is<BotEvents>())
                {
                    parents.push_back(value.as<BotEvents>());
                }
            }
            return thread->m_events->CreateEvents(parents);
        },
        [thread]() {
            return thread->m_events->CreateEvents({});
        }
    ));

    auto LBot = m_state.new_usertype<Bot>("Bot");
    LBot.set_function("Disconnect", &Bot::QueueDisconnect);
    LBot.set_function("GetUsername", &Bot::GetUsername);
    LBot.set_function("GetPassword", &Bot::GetPassword);

    fs::path path = fs::path(sConfigMgr->GetStringDefault("Lua.Path","./")) / "bots";

    if (std::filesystem::exists(path))
    {
        for (const std::filesystem::directory_entry& dir_entry :
            std::filesystem::recursive_directory_iterator(path))
        {
            if (std::filesystem::is_regular_file(dir_entry.path()) && dir_entry.path().extension() == ".lua")
            {
                m_state.safe_script_file(dir_entry.path().string());
            }
        }
    }
}