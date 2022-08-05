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
#include "Bot.h"
#include "BotSocket.h"
#include "BotMgr.h"
#include "BotProfile.h"
#include "BotLogging.h"
#include "HMAC.h"

#include "BehaviorTree.h"

#include "boost/asio/high_resolution_timer.hpp"
#include <chrono>

using namespace std::literals::chrono_literals;

Bot::Bot(BotThread* thread, std::string const& username, std::string const& password, std::string const& events, std::string const& authserver)
    : m_thread(thread)
    , m_username(username)
    , m_password(password)
    , m_disconnected(false)
    , m_events(events)
    , m_authserverIp(authserver)
{
    std::transform(m_username.begin(), m_username.end(), m_username.begin(), [](uint8_t c) { return std::toupper(c); });
    std::transform(m_password.begin(), m_password.end(), m_password.begin(), [](uint8_t c) { return std::toupper(c); });
    FIRE(OnCreate, this->GetEvents(), {}, *this);
}

std::string const& Bot::GetUsername() const
{
    return m_username;
}

std::string const& Bot::GetPassword() const
{
    return m_password;
}

void Bot::QueueDisconnect()
{
    if (!m_disconnected)
    {
        m_thread->m_bot_count--;
        m_thread->m_queuedRemoves.push_back(m_username);
    }
    m_disconnected = true;
}

void Bot::DisconnectNow()
{
    if (m_worldSocket.has_value() || m_authSocket.has_value())
    {
        BOT_LOG_DEBUG("bot","Logging out %s",m_username.c_str());
    }

    if (m_worldSocket.has_value())
    {
        m_worldSocket.value().Close();
    }

    if (m_authSocket.has_value())
    {
        m_authSocket.value().Close();
    }

    m_worldSocket.reset();
    m_authSocket.reset();
    m_encrypt.reset();
    m_decrypt.reset();
}

BotSocket& Bot::GetWorldSocket()
{
    // TODO: handle errors
    return m_worldSocket.value();
}

BotSocket& Bot::GetAuthSocket()
{
    // TODO: handle errors
    return m_authSocket.value();
}

BotProfile Bot::GetEvents()
{
    return m_cached_events;
}

void Bot::SetEncryptionKey(std::array<uint8_t, 40> const& key)
{
    m_encrypt.emplace();
    m_decrypt.emplace();
    uint8 ClientEncryptionKey[] = { 0xC2, 0xB3, 0x72, 0x3C, 0xC6, 0xAE, 0xD9, 0xB5, 0x34, 0x3C, 0x53, 0xEE, 0x2F, 0x43, 0x67, 0xCE };
    m_encrypt.value().Init(Trinity::Crypto::HMAC_SHA1::GetDigestOf(ClientEncryptionKey, key));
    uint8 ClientDecryptionKey[] = { 0xCC, 0x98, 0xAE, 0x04, 0xE8, 0x97, 0xEA, 0xCA, 0x12, 0xDD, 0xC0, 0x93, 0x42, 0x91, 0x53, 0x57 };
    m_decrypt.value().Init(Trinity::Crypto::HMAC_SHA1::GetDigestOf(ClientDecryptionKey, key));
    std::array<uint8_t, 1024> arr;
    m_encrypt.value().UpdateData(arr);

    m_decrypt.value().UpdateData(arr);
}

boost::asio::awaitable<void> Bot::WorldPacketLoop()
{
    for (;;)
    {
        try
        {
            WorldPacket curPacket = co_await WorldPacket::ReadWorldPacket(*this);
            try
            {
                FIRE_ID(uint32_t(curPacket.GetOpcode()), OnWorldPacket, GetEvents(), { curPacket.Reset(); }, * this, curPacket)
            }
            catch (std::exception const& e)
            {
                // TODO: better reporting
                BOT_LOG_ERROR("bot", e.what());
            }
        }
        catch (std::exception const&)
        {
            // TODO: filter by exception type
            break;
        }
    }
}

boost::asio::awaitable<void> Bot::Connect(boost::asio::any_io_executor& exec, std::string const& authServerIp)
{
    DisconnectNow();

    BOT_LOG_DEBUG("bot","Logging in %s", GetUsername().c_str());
    try
    {
        co_await sAuthMgr->AuthenticateBot(exec, authServerIp, *this);
    }
    catch (std::exception const& e)
    {
        BOT_LOG_ERROR("bot","Error logging in  bot %s: %s",m_username.c_str(),e.what());
        co_return;
    }

    BOT_LOG_DEBUG("bot","Successfully logged in as %s", GetUsername().c_str());
    co_await WorldPacketLoop();
}

void Bot::LoadScripts()
{
    m_behavior = nullptr;
    m_cached_events = m_thread->m_events->GetEvents(m_events);
    if (m_cached_events.m_root)
    {
        m_behavior = std::make_unique<TreeExecutor<Bot, std::monostate, std::monostate>>(m_thread->m_events->GetBehaviorTreeContext(),m_cached_events.m_root);
        m_thread->m_botsWithAI[this->m_username] = this;
    }
}
