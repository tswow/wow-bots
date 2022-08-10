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

#include "BotSocket.h"
#include "BotProfile.h"

#include <sol/sol.hpp>

#include <string>
#include <optional>
#include <variant>
#include <memory>

class BotThread;
class BotProfile;
class Bot;

template<typename C, typename LC, typename DC>
class TreeExecutor;

class Bot
{
public:
    // Disconnects auth/world connections immediately. Not thread-safe.
    void DisconnectNow();
    // Disconnects this bot the next time its owning thread 
    void QueueDisconnect();
    void Connect();
    void SetEncryptionKey(std::array<uint8_t,40> const& key);
    Bot(BotThread* thread, std::string const& username, std::string const& password, std::string const& events, std::string const& authserver);
    std::string const& GetUsername() const;
    std::string const& GetPassword() const;
    BotSocket& GetWorldSocket();
    BotSocket& GetAuthSocket();
    BotSocket2& GetAuthSocket2();
    BotSocket2& GetWorldSocket2();
    BotProfile GetEvents();
    friend class WorldPacket;
    friend class BotThread;
    friend class BotMgr;
    friend class BotProfileLua;
    friend class AuthMgr;
private:
    BotThread* m_thread;
    std::string m_username;
    std::string m_password;
    std::string m_authserverIp;
    bool m_disconnected;
    std::unique_ptr<TreeExecutor<Bot,std::monostate,std::monostate>> m_behavior;
    std::string m_events;
    BotProfile m_cached_events;
    std::optional<Trinity::Crypto::ARC4> m_encrypt;
    std::optional<Trinity::Crypto::ARC4> m_decrypt;
    std::optional<BotSocket> m_worldSocket;
    std::optional<BotSocket> m_authSocket;
    std::array<uint8_t, 20> m_m2Hash;
    std::optional<BotSocket2> m_authSocket2;
    std::optional<BotSocket2> m_worldSocket2;

    boost::asio::io_context m_ioc;
    sol::table m_data;
    void LoadScripts();
    void UnloadScripts();
    void Authenticate();
};
