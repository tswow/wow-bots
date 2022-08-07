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
#include "BotMgr.h"
#include "Bot.h"
#include "BotAuth.h"
#include "BotProfile.h"
#include "BotProfileLua.h"
#include "BotLogging.h"
#include "Config.h"
#include "BehaviorTree.h"

#include "boost/asio.hpp"

#include <map>
#include <thread>

static uint64_t now()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

boost::asio::awaitable<void> BotThread::run()
{
    BOT_LOG_DEBUG("BotThread", "Starting bot thread %i", m_threadId);
    boost::asio::any_io_executor executor = co_await boost::asio::this_coro::executor;

    // message loop
    for (;;)
    {
        boost::asio::deadline_timer t(executor, boost::posix_time::milliseconds(5));
        co_await t.async_wait(boost::asio::use_awaitable);
        {
            for (auto& [_, bot] : m_botsWithAI)
            {
                if (bot->m_behavior)
                {
                    bot->m_behavior->Update(*bot, now());
                }
            }


            if (m_queuedLogins.size() > 0 || m_queuedRemoves.size() > 0 || m_shouldReload)
            {
                std::scoped_lock lock(sBotMgr->m_botMutex);
                for (std::string const& str : m_queuedLogins)
                {
                    auto itr = sBotMgr->m_bots.find(str);
                    if (itr == sBotMgr->m_bots.end())
                    {
                        continue;
                    }
                    Bot* bot = itr->second.get();
                    if (bot->m_thread != this || bot->m_disconnected)
                    {
                        continue;
                    }
                    bot->LoadScripts();
                    boost::asio::co_spawn(executor, bot->Connect(executor, "127.0.0.1"), boost::asio::detached);
                }
                m_queuedLogins.clear();

                for (std::string const& str : m_queuedRemoves)
                {
                    auto itr = sBotMgr->m_bots.find(str);
                    if (itr == sBotMgr->m_bots.end())
                    {
                        continue;
                    }
                    Bot* bot = itr->second.get();
                    if (bot->m_thread != this || !bot->m_disconnected)
                    {
                        continue;
                    }
                    if (bot->m_behavior != nullptr)
                    {
                        m_botsWithAI.erase(str);
                    }
                    bot->DisconnectNow();
                    sBotMgr->m_bots.erase(str);
                }

                if (m_shouldReload)
                {
                    for (auto& bot : sBotMgr->m_bots)
                    {
                        if (bot.second->m_thread == this)
                        {
                            bot.second->UnloadScripts();
                        }
                    }
                    m_botsWithAI.clear();
                    m_events->Reset();
                    m_lua = std::make_unique<BotProfileLua>(this);
                    m_lua->Start();
                    for (auto& bot : sBotMgr->m_bots)
                    {
                        if (bot.second->m_thread == this)
                        {
                            bot.second->LoadScripts();
                        }
                    }
                    m_shouldReload = false;
                }
            }
        }
    }
}

BotThread::BotThread()
    : m_events(std::make_unique<BotProfileMgr>())
    , m_threadId(UINT32_MAX)
{
}

void BotThread::Reload()
{
    m_shouldReload = true;
}

void BotThread::start(int thread)
{
    m_threadId = thread;
    boost::asio::io_context ctx;
    boost::asio::co_spawn(ctx, run(), boost::asio::detached);
    ctx.run();
}

BotMgr* BotMgr::instance()
{
    static BotMgr mgr;
    return &mgr;
}

void BotMgr::StartBot(std::string const& username, std::string const& password, std::string const& events, std::string const& authserver)
{
    std::scoped_lock(m_botMutex);
    auto old = m_bots.find(username);
    if (old != m_bots.end())
    {
        if (old->second->m_disconnected)
        {
            old->second->m_disconnected = false;
            old->second->m_thread->m_bot_count++;
        }
        old->second->m_thread->m_queuedLogins.push_back(username);
        return;
    }
    BotThread* cur = nullptr;
    for (std::unique_ptr<BotThread>& thread : m_threads)
    {
        if (!cur || thread->m_bot_count < cur->m_bot_count)
        {
            cur = thread.get();
        }
    }
    if (!cur)
    {
        return;
    }
    m_bots[username] = std::make_unique<Bot>(cur, username, password, events, authserver);
    cur->m_queuedLogins.push_back(username);
    cur->m_bot_count++;
}

void BotMgr::StopBot(std::string const& username)
{
    std::scoped_lock(m_botMutex);
    auto bot = m_bots.find(username);
    if (bot == m_bots.end() || bot->second->m_disconnected)
    {
        return;
    }
    bot->second->QueueDisconnect();
}

void BotMgr::Initialize()
{
    int threadCount = sConfigMgr->GetIntDefault("Bots.ThreadCount", 1);
    m_threads.resize(threadCount);
    for (int i = 0; i < threadCount; ++i)
    {
        BotThread* thread = (m_threads[i] = std::make_unique<BotThread>()).get();
        std::thread(&BotThread::start, thread, i).detach();
    }
}

void BotMgr::Reload()
{
    for (std::unique_ptr<BotThread>& thread : m_threads)
    {
        thread->m_shouldReload = true;
    }
}

BotThread::~BotThread()
{
    // force reset callbacks before we clear the lua state
    m_events->Reset();
}

void StartBot(std::string const& username, std::string const& password, std::string const& events, std::string const& authserver)
{
    sBotMgr->StartBot(
        username,
        password,
        events,
        authserver.size()
            ? authserver
            : sConfigMgr->GetStringDefault("Bots.DefaultAuthServer","127.0.0.1")
    );
}

void StopBot(std::string const& username)
{
    sBotMgr->StopBot(username);
}
