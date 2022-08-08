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
#include "BotConfig.h"
#include "Movement.h"

#include "PacketsFwd.h"

#include <vector>
#include <map>
#include <set>
#include <cstdint>
#include <variant>

class Bot;
class BotProfileMgr;
class MovementPacket;
class UpdateDataPacket;

template <typename C, typename LC, typename DC>
class Node;

template <typename C, typename LC, typename DC>
class BehaviorTreeContext;

class BotProfileData
{
public:
    EVENT_STORAGE(OnLoad, Bot& bot)
    EVENT_STORAGE(OnAuthChallenge, Bot& bot, AuthPacket& packetOut, BotMutable<bool> cancel)
    EVENT_STORAGE(OnAuthProof, Bot& bot, ServerAuthChallenge& challenge, AuthPacket& packetOut, BotMutable<bool> cancel)
    EVENT_STORAGE(OnRequestRealms, Bot& bot, AuthPacket& packetOut, BotMutable<bool> cancel)
    EVENT_STORAGE(OnSelectRealm, Bot& bot, std::vector<RealmInfo> const& realms, BotMutable<RealmInfo> realm)
    EVENT_STORAGE(OnCloseAuthConnection, Bot& bot, BotMutable<bool> shouldClose, BotMutable<bool> cancel)
    EVENT_STORAGE(OnWorldAuthChallenge, Bot& bot, WorldPacket& packetIn, WorldPacket& packetOut, BotMutable<bool> cancel)
    EVENT_STORAGE(OnWorldAuthResponse, Bot& bot, WorldAuthResponse& response, WorldPacket& packetOut, BotMutable<bool> cancel)
    EVENT_STORAGE(OnWorldPacket, Bot& bot, WorldPacket& packet)
private:
    BotProfileData(BotProfileMgr* mgr);
    std::vector<BotProfileData*> m_parents;
    std::vector<BotProfileData*> m_children;
    BotProfileMgr* m_mgr;
    Node<Bot, std::monostate, std::monostate>* m_root = nullptr;
    void apply_extensions(BotProfileData* parent)
    {
        EXTEND_EVENT(this, parent, OnWorldPacket);
        EXTEND_EVENT(this, parent, OnLoad);
        EXTEND_EVENT(this, parent, OnAuthChallenge);
        EXTEND_EVENT(this, parent, OnAuthProof);
        EXTEND_EVENT(this, parent, OnRequestRealms);
        EXTEND_EVENT(this, parent, OnSelectRealm);
        EXTEND_EVENT(this, parent, OnCloseAuthConnection);
        EXTEND_EVENT(this, parent, OnWorldAuthChallenge);
        EXTEND_EVENT(this, parent, OnWorldAuthResponse);
    }
    friend class BotProfileMgr;
    friend class BotProfile;
    friend class Bot;
};

class BotProfile
{
public:
    EVENT(OnLoad)
    EVENT(OnAuthChallenge)
    EVENT(OnAuthProof)
    EVENT(OnRequestRealms)
    EVENT(OnSelectRealm)
    EVENT(OnCloseAuthConnection)
    EVENT(OnWorldAuthChallenge)
    EVENT(OnWorldAuthResponse)
    ID_EVENT(OnWorldPacket)
    PACKET_EVENTS_DECL
    BotProfile OnUpdateData(std::function<void(Bot& bot, UpdateDataPacket packet)> callback);
    BotProfile OnMovementPacket(std::function<void(Bot& bot, MovementPacket packet)> callback);
    BotProfile SetBehaviorRoot(Node<Bot, std::monostate, std::monostate>* root);
    BotProfile Register(std::string const& mod, std::string const& name);
    BotProfile();
    bool IsLoaded();
private:
    BotProfileData * m_storage = nullptr;
    BotProfile(BotProfileData* data);
    friend class BotProfileMgr;
    friend class Bot;
};

class BotThread;
class BotProfileMgr
{
public:
    void Reset();
    void Build();
    BotProfile CreateEvents(std::vector<BotProfile> const& parents);
    BotProfile GetEvents(std::string const& events);
    BotProfile GetRootEvent();
    BehaviorTreeContext<Bot, std::monostate, std::monostate>* GetBehaviorTreeContext();
    BotProfileMgr();
    static BotProfileData* GetStorage(BotProfile const& events);
private:
    uint32_t GetDeepestChild(BotProfileData* events, std::vector<std::vector<BotProfileData*>>& depthLayers, std::map<BotProfileData*, uint32_t>& cachedDepth);
    void ApplyParents(BotProfileData* target, BotProfileData* cur, std::set<BotProfileData*>& visited);
    std::vector<std::unique_ptr<BotProfileData>> m_events;
    std::map<std::string, BotProfileData*> m_namedEvents;
    std::unique_ptr<BehaviorTreeContext<Bot, std::monostate, std::monostate>> m_btContext;
    friend class BotProfile;
};
