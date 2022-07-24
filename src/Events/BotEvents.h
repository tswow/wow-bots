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

#include "BotEvent.h"
#include "BotAuth.h"
#include "BotPacket.h"
#include "BotMutable.h"

#include <vector>
#include <map>
#include <set>
#include <cstdint>

#define ROOT_EVENT_NAME "tswow:root_events"

class Bot;
class BotEventsMgr;

class BotEventData
{
public:
    EVENT_STORAGE(OnCreate, Bot& bot)
    EVENT_STORAGE(OnAuthChallenge, Bot& bot, AuthPacket& packetOut, BotMutable<bool> cancel)
    EVENT_STORAGE(OnAuthProof, Bot& bot, ServerAuthChallenge& challenge, AuthPacket& packetOut, BotMutable<bool> cancel)
    EVENT_STORAGE(OnRequestRealms, Bot& bot, AuthPacket& packetOut, BotMutable<bool> cancel)
    EVENT_STORAGE(OnSelectRealm, Bot& bot, std::vector<RealmInfo> const& realms, BotMutable<RealmInfo> realm)
    EVENT_STORAGE(OnCloseAuthConnection, Bot& bot, BotMutable<bool> shouldClose, BotMutable<bool> cancel)
    EVENT_STORAGE(OnWorldAuthChallenge, Bot& bot, WorldPacket& packetIn, WorldPacket& packetOut, BotMutable<bool> cancel)
    EVENT_STORAGE(OnWorldAuthResponse, Bot& bot, WorldAuthResponse& response, WorldPacket& packetOut, BotMutable<bool> cancel)
    EVENT_STORAGE(OnWorldPacket, Bot& bot, WorldPacket& packet)
private:
    BotEventData(BotEventsMgr* mgr);
    std::vector<BotEventData*> m_parents;
    std::vector<BotEventData*> m_children;
    BotEventsMgr* m_mgr;
    void apply_extensions(BotEventData* parent)
    {
        EXTEND_EVENT(this, parent, OnWorldPacket);
        EXTEND_EVENT(this, parent, OnCreate);
        EXTEND_EVENT(this, parent, OnAuthChallenge);
        EXTEND_EVENT(this, parent, OnAuthProof);
        EXTEND_EVENT(this, parent, OnRequestRealms);
        EXTEND_EVENT(this, parent, OnSelectRealm);
        EXTEND_EVENT(this, parent, OnCloseAuthConnection);
        EXTEND_EVENT(this, parent, OnWorldAuthChallenge);
        EXTEND_EVENT(this, parent, OnWorldAuthResponse);
    }
    friend class BotEventsMgr;
    friend class BotEvents;
};

class BotEvents
{
public:
    EVENT(OnCreate)
    EVENT(OnAuthChallenge)
    EVENT(OnAuthProof)
    EVENT(OnRequestRealms)
    EVENT(OnSelectRealm)
    EVENT(OnCloseAuthConnection)
    EVENT(OnWorldAuthChallenge)
    EVENT(OnWorldAuthResponse)
    ID_EVENT(OnWorldPacket)
    void Register(std::string const& mod, std::string const& name);
    BotEvents() = default;
    bool IsLoaded();
private:
    BotEventData * m_storage = nullptr;
    BotEvents(BotEventData* data);
    friend class BotEventsMgr;
};

class BotThread;
class BotEventsMgr
{
public:
    void Reset();
    void Build();
    BotEvents CreateEvents(std::vector<BotEvents> const& parents);
    BotEvents GetEvents(std::string const& events);
    BotEvents GetRootEvent();
    BotEventsMgr();
    static BotEventData* GetStorage(BotEvents const& events);
private:
    uint32_t GetDeepestChild(BotEventData* events, std::vector<std::vector<BotEventData*>>& depthLayers, std::map<BotEventData*, uint32_t>& cachedDepth);
    void ApplyParents(BotEventData* target, BotEventData* cur, std::set<BotEventData*>& visited);
    std::vector<std::unique_ptr<BotEventData>> m_events;
    std::map<std::string, BotEventData*> m_namedEvents;
    friend class BotEvents;
};
