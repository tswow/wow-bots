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
#include "BotMain.h"

#include <boost/asio/awaitable.hpp>

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <memory>
#include <optional>
#include <atomic>

class Bot;
class BotProfile;
class BotLua;
class BotProfileMgr;

class BotThread
{
public:
    void start();
    void Reload();
    BotThread();
    std::unique_ptr<BotProfileMgr> m_events;
    std::unique_ptr<BotLua> m_lua;
    ~BotThread();
private:
    boost::asio::awaitable<void> run();
    uint32_t m_threadId;
    std::vector<std::string> m_queuedLogins;
    std::vector<std::string> m_queuedRemoves;
    int m_bot_count = 0;
    std::atomic<bool> m_shouldReload = true;
    friend class Bot;
    friend class BotMgr;
};

class BotMgr
{
public:
    static BotMgr* instance();
    void AddBot(std::string const& username, std::string const& password, std::string const& events, std::string const& authserver = "127.0.0.1");
    void RemoveBot(std::string const& username);
    void Initialize();
    void Reload();
    std::mutex m_botMutex;
private:
    std::map<std::string, std::unique_ptr<Bot>> m_bots;
    std::vector<std::unique_ptr<BotThread>> m_threads;
    friend class BotThread;
};

#define sBotMgr BotMgr::instance()