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
#include "BotProfileLua.h"
#include "Bot.h"
#include "BotProfile.h"
#include "BotMutable.h"
#include "BotLuaShared.h"
#include "BotLogging.h"
#include "Config.h"
#include "BehaviorTree.h"
#include "Movement.h"
#include "Update.h"
#include "Packets.h"
#include "PacketLua.h"

#include <vector>

namespace fs = std::filesystem;

template <typename T>
auto RegisterPacket(std::string const& name, sol::state& state)
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

    type.set_function("ReadUInt8", &T::ReadUInt8);
    type.set_function("ReadInt8", &T::ReadInt8);
    type.set_function("ReadUInt16", &T::ReadUInt16);
    type.set_function("ReadInt16", &T::ReadInt16);
    type.set_function("ReadUInt32", &T::ReadUInt32);
    type.set_function("ReadInt32", &T::ReadInt32);
    type.set_function("ReadUInt64", &T::ReadUInt64);
    type.set_function("ReadInt64", &T::ReadInt64);
    type.set_function("ReadFloat", &T::ReadFloat);
    type.set_function("ReadDouble", &T::ReadDouble);
    type.set_function("ReadString", &T::ReadString);
    type.set_function("ReadCString", &T::ReadCString);

    type.set_function("WriteBytes", [](T& target, sol::table table) {
        std::vector<uint8_t> vec;
        vec.reserve(table.size());
        for (size_t i = 1; i <= table.size(); ++i)
        {
            vec.push_back(table.get<uint8_t>(i));
        }
        return target.WriteBytes<std::vector<uint8_t>>(vec);
    });
    type.set_function("Send", &T::Send);
    return type;
}

BotProfileLua::BotProfileLua(BotThread* thread)
    : BotLuaState(fs::path(sConfigMgr->GetStringDefault("Lua.Path", "./")) / "profiles")
    , m_thread(thread)
{}

void BotProfileLua::LoadLibraries()
{
    LuaRegisterBehaviorTree<Bot,std::monostate,std::monostate>(m_state,"BotLua",m_thread->m_events->GetBehaviorTreeContext(),"Bot");
    RegisterPacketLua(m_state);
    MovementPacket::Register(m_state);
    UpdateData::Register(m_state);

    auto LBotProfile = m_state.new_usertype<BotProfile>("BotProfile");
    LBotProfile.set_function("Register", &BotProfile::Register);
    LBotProfile.set_function("SetBehaviorRoot", &BotProfile::SetBehaviorRoot);
    LBotProfile.set_function("OnWorldPacket", sol::overload(&BotProfile::LOnWorldPacket, &BotProfile::_LOnWorldPacket,&BotProfile::LidOnWorldPacket));
    LBotProfile.set_function("OnLoad", &BotProfile::LOnLoad);
    LBotProfile.set_function("OnAuthChallenge", &BotProfile::LOnAuthChallenge);
    LBotProfile.set_function("OnAuthProof", &BotProfile::LOnAuthProof);
    LBotProfile.set_function("OnRequestRealms", &BotProfile::LOnRequestRealms);
    LBotProfile.set_function("OnSelectRealm", &BotProfile::LOnSelectRealm);
    LBotProfile.set_function("OnCloseAuthConnection", &BotProfile::LOnCloseAuthConnection);
    LBotProfile.set_function("OnWorldAuthChallenge", &BotProfile::LOnWorldAuthChallenge);
    LBotProfile.set_function("OnWorldAuthResponse", &BotProfile::LOnWorldAuthResponse);
    LBotProfile.set_function("OnLoggedIn", &BotProfile::LOnLoggedIn);
    LBotProfile.set_function("OnMovementPacket", [](BotProfile& bot, sol::protected_function callback) {
        bot.OnMovementPacket([=](Bot& bot, MovementPacket packet) {
            callback(bot, packet);
        });
        return bot;
    });
    LBotProfile.set_function("OnUpdateData", [](BotProfile& bot, sol::protected_function callback) {
        bot.OnUpdateData([=](Bot& bot, UpdateDataPacket packet) {
            callback(bot, packet);
        });
        return bot;
    });

    LUA_EVENTS(LBotProfile);

    auto worldpacket = RegisterPacket<WorldPacket>("WorldPacket", m_state);
    worldpacket.set_function("GetOpcode", &WorldPacket::GetOpcode);
    worldpacket.set_function("ReadPackedGUID", &WorldPacket::ReadPackedGUID);
    worldpacket.set_function("WritePackedGUID", &WorldPacket::WritePackedGUID);
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

    m_state.set("RootBot", BotProfile(m_thread->m_events->GetRootEvent()));
    m_state.set_function("CreateBotProfile", sol::overload(
        [this](sol::table parentsTable) {
            std::vector<BotProfile> parents;
            for (auto [key, value] : parentsTable)
            {
                if (value.is<BotProfile>())
                {
                    parents.push_back(value.as<BotProfile>());
                }
            }
            return m_thread->m_events->CreateEvents(parents);
        },
        [this]() {
            return m_thread->m_events->CreateEvents({});
        }
    ));

    auto LBot = m_state.new_usertype<Bot>("Bot");
    LBot.set_function("Disconnect", &Bot::QueueDisconnect);
    LBot.set_function("GetUsername", &Bot::GetUsername);
    LBot.set_function("GetPassword", &Bot::GetPassword);
    LBot.set_function("IsLoggedIn", &Bot::IsLoggedIn);
    LBot.set_function("GetThreadID", &Bot::GetThreadID);

    LBot.set_function("SetData", [this](Bot* bot, std::string const& key, sol::object value) {
        InitializeBotData(bot);
        bot->m_data[key] = value;
        return bot;
    });

    LBot.set_function("HasData", [this](Bot* bot, std::string const& key) {
        return bot->m_data.valid() ? bot->m_data[key] != sol::nil : false;
    });

    LBot.set_function("GetData", sol::overload(
        [this](Bot* bot, std::string const& key, sol::object value) {
            InitializeBotData(bot);
            return bot->m_data.get_or(key, value);
        },
        [this](Bot* bot, std::string const& key) {
            InitializeBotData(bot);
            return bot->m_data[key];
        }
    ));

    fs::path path = fs::path(sConfigMgr->GetStringDefault("Lua.Path","./")) / "profiles";
}

void BotProfileLua::InitializeBotData(Bot* bot)
{
    if (!bot->m_data.valid())
    {
        bot->m_data = m_state.create_table();
    }
}
