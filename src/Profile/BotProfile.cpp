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
#include "BotProfile.h"
#include "BehaviorTree.h"
#include "Update.h"

#include <set>
#include <map>
#include <memory>

BotProfileData::BotProfileData(BotProfileMgr* mgr)
    : m_mgr(mgr)
{}

void BotProfileMgr::Reset()
{
    m_namedEvents.clear();
    m_events.clear();
    m_events.push_back(std::unique_ptr<BotProfileData>(new BotProfileData(this)));
    m_namedEvents[ROOT_EVENT_NAME] = m_events[0].get();
    m_btContext = std::make_unique<BehaviorTreeContext<Bot, std::monostate, std::monostate>>();
}

BotProfile BotProfileMgr::GetRootEvent()
{
    return BotProfile(m_events[0].get());
}


BotProfileMgr::BotProfileMgr()
{
    Reset();
}

BehaviorTreeContext<Bot, std::monostate, std::monostate>* BotProfileMgr::GetBehaviorTreeContext()
{
    return m_btContext.get();
}

BotProfile BotProfileMgr::CreateEvents(std::vector<BotProfile> const& parents)
{
    m_events.push_back(std::unique_ptr<BotProfileData>(new BotProfileData(this)));
    BotProfileData* evts = m_events[m_events.size() - 1].get();

    // Add parents
    for (BotProfile const& evt: parents)
    {
        evts->m_parents.push_back(evt.m_storage);
        evt.m_storage->m_children.push_back(evts);
    }

    // If no parents, default to root event parent
    if (parents.size() == 0)
    {
        evts->m_parents.push_back(GetRootEvent().m_storage);
        GetRootEvent().m_storage->m_children.push_back(evts);
    }

    return BotProfile(evts);
}

void BotProfileMgr::ApplyParents(BotProfileData* target, BotProfileData* cur, std::set<BotProfileData*>& visited)
{
    for (BotProfileData* parent : cur->m_parents)
    {
        if (!visited.contains(parent))
        {
            visited.insert(parent);
            target->apply_extensions(parent);
            ApplyParents(target, parent, visited);
        }
    }
}

uint32_t BotProfileMgr::GetDeepestChild(BotProfileData* events, std::vector<std::vector<BotProfileData*>>& depthLayers, std::map<BotProfileData*, uint32_t>& cachedDepth)
{
    auto itr = cachedDepth.find(events);
    if (itr != cachedDepth.end())
    {
        return itr->second;
    }

    uint32_t max = 0;
    for (BotProfileData* child: events->m_children)
    {
        uint32_t depth = GetDeepestChild(child, depthLayers, cachedDepth);
        if (depth > max)
        {
            max = depth;
        }
    }

    if (max >= depthLayers.size())
    {
        depthLayers.resize(max + 1);
    }
    depthLayers[max].push_back(events);
    cachedDepth[events] = max;
    return max;
}

void BotProfileMgr::Build()
{
    // set up depth layers
    std::vector<std::vector<BotProfileData*>> depthLayers;
    std::map<BotProfileData*, uint32_t> cachedDepth;
    GetDeepestChild(GetRootEvent().m_storage, depthLayers, cachedDepth);

    // apply parenting at levels
    for (std::vector<BotProfileData*> const& layer : depthLayers)
    {
        for (BotProfileData* evt : layer)
        {
            std::set<BotProfileData*> visited;
            ApplyParents(evt, evt, visited);
        }
    }

    // clean up some memory
    for (auto& value : m_events)
    {
        value->m_children.clear();
        value->m_parents.clear();
    }
}

BotProfile::BotProfile()
    : m_storage(nullptr)
{}

BotProfile::BotProfile(BotProfileData* data)
    : m_storage(data)
{}

BotProfile BotProfile::Register(std::string const& mod, std::string const& name)
{
    std::string merged = mod + ":" + name;
    auto& namedEvents = m_storage->m_mgr->m_namedEvents;
    if (namedEvents.find(merged) != namedEvents.end())
    {
        throw std::runtime_error("Tried to register event handler " + merged + " twice!");
    }
    namedEvents[merged] = m_storage;
    return *this;
}

BotProfile BotProfile::SetBehaviorRoot(Node<Bot, std::monostate, std::monostate>* root)
{
    m_storage->m_root = root;
    return *this;
}

BotProfile BotProfileMgr::GetEvents(std::string const& events)
{
    auto itr = m_namedEvents.find(events);
    if (itr == m_namedEvents.end())
    {
        throw new std::runtime_error("Attempted to load non-existing events " + events);
    }
    return BotProfile(itr->second);
}

bool BotProfile::IsLoaded()
{
    return m_storage != nullptr;
}

BotProfileData* BotProfileMgr::GetStorage(BotProfile const& events)
{
    return events.m_storage;
}

BotProfile BotProfile::OnMovementPacket(std::function<void(Bot& bot, MovementPacket packet)> callback)
{
    OnWorldPacket(std::vector<uint32_t>({
        uint32(Opcodes::MSG_MOVE_START_FORWARD),
        uint32(Opcodes::MSG_MOVE_START_BACKWARD),
        uint32(Opcodes::MSG_MOVE_STOP),
        uint32(Opcodes::MSG_MOVE_START_STRAFE_LEFT),
        uint32(Opcodes::MSG_MOVE_START_STRAFE_RIGHT),
        uint32(Opcodes::MSG_MOVE_STOP_STRAFE),
        uint32(Opcodes::MSG_MOVE_JUMP),
        uint32(Opcodes::MSG_MOVE_START_TURN_LEFT),
        uint32(Opcodes::MSG_MOVE_START_TURN_RIGHT),
        uint32(Opcodes::MSG_MOVE_STOP_TURN),
        uint32(Opcodes::MSG_MOVE_START_PITCH_UP),
        uint32(Opcodes::MSG_MOVE_START_PITCH_DOWN),
        uint32(Opcodes::MSG_MOVE_STOP_PITCH),
        uint32(Opcodes::MSG_MOVE_SET_RUN_MODE),
        uint32(Opcodes::MSG_MOVE_SET_WALK_MODE),
        uint32(Opcodes::MSG_MOVE_FALL_LAND),
        uint32(Opcodes::MSG_MOVE_START_SWIM),
        uint32(Opcodes::MSG_MOVE_STOP_SWIM),
        uint32(Opcodes::MSG_MOVE_SET_FACING),
        uint32(Opcodes::MSG_MOVE_SET_PITCH),
        uint32(Opcodes::MSG_MOVE_HEARTBEAT),
        uint32(Opcodes::MSG_MOVE_START_ASCEND),
        uint32(Opcodes::MSG_MOVE_STOP_ASCEND),
        uint32(Opcodes::MSG_MOVE_START_DESCEND)
    }), [=](Bot& bot, WorldPacket& packet) {
        callback(bot, MovementPacket::Read(packet));
    });

    return *this;
}

BotProfile BotProfile::OnUpdateData(std::function<void(Bot& bot, UpdateDataPacket packet)> callback)
{
    OnWorldPacket(uint32_t(Opcodes::SMSG_UPDATE_OBJECT), [=](Bot& bot, WorldPacket& packet) {
        callback(bot, UpdateDataPacket::Read(packet));
    });

    OnWorldPacket(uint32_t(Opcodes::SMSG_COMPRESSED_UPDATE_OBJECT), [=](Bot& bot, WorldPacket& packet) {
        callback(bot, UpdateDataPacket::ReadCompressed(packet));
    });

    return *this;
}
