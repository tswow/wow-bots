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
#include "BotEvents.h"

#include <set>
#include <map>
#include <memory>

BotEventData::BotEventData(BotEventsMgr* mgr)
    : m_mgr(mgr)
{}

void BotEventsMgr::Reset()
{
    m_namedEvents.clear();
    m_events.clear();
    m_events.push_back(std::unique_ptr<BotEventData>(new BotEventData(this)));
    m_namedEvents[ROOT_EVENT_NAME] = m_events[0].get();
}

BotEvents BotEventsMgr::GetRootEvent()
{
    return BotEvents(m_events[0].get());
}


BotEventsMgr::BotEventsMgr()
{
    Reset();
}

BotEvents BotEventsMgr::CreateEvents(std::vector<BotEvents> const& parents)
{
    m_events.push_back(std::unique_ptr<BotEventData>(new BotEventData(this)));
    BotEventData* evts = m_events[m_events.size() - 1].get();

    // Add parents
    for (BotEvents const& evt: parents)
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

    return BotEvents(evts);
}

void BotEventsMgr::ApplyParents(BotEventData* target, BotEventData* cur, std::set<BotEventData*>& visited)
{
    for (BotEventData* parent : cur->m_parents)
    {
        if (!visited.contains(parent))
        {
            visited.insert(parent);
            target->apply_extensions(parent);
            ApplyParents(target, parent, visited);
        }
    }
}

uint32_t BotEventsMgr::GetDeepestChild(BotEventData* events, std::vector<std::vector<BotEventData*>>& depthLayers, std::map<BotEventData*, uint32_t>& cachedDepth)
{
    auto itr = cachedDepth.find(events);
    if (itr != cachedDepth.end())
    {
        return itr->second;
    }

    uint32_t max = 0;
    for (BotEventData* child: events->m_children)
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

void BotEventsMgr::Build()
{
    // set up depth layers
    std::vector<std::vector<BotEventData*>> depthLayers;
    std::map<BotEventData*, uint32_t> cachedDepth;
    GetDeepestChild(GetRootEvent().m_storage, depthLayers, cachedDepth);

    // apply parenting at levels
    for (std::vector<BotEventData*> const& layer : depthLayers)
    {
        for (BotEventData* evt : layer)
        {
            std::set<BotEventData*> visited;
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

BotEvents::BotEvents(BotEventData* data)
    : m_storage(data)
{}

void BotEvents::Register(std::string const& mod, std::string const& name)
{
    std::string merged = mod + ":" + name;
    auto& namedEvents = m_storage->m_mgr->m_namedEvents;
    if (namedEvents.find(merged) != namedEvents.end())
    {
        throw std::runtime_error("Tried to register event handler " + merged + " twice!");
    }
    namedEvents[merged] = m_storage;
}

BotEvents BotEventsMgr::GetEvents(std::string const& events)
{
    auto itr = m_namedEvents.find(events);
    if (itr == m_namedEvents.end())
    {
        throw new std::runtime_error("Attempted to load non-existing events " + events);
    }
    return BotEvents(itr->second);
}

bool BotEvents::IsLoaded()
{
    return m_storage != nullptr;
}

BotEventData* BotEventsMgr::GetStorage(BotEvents const& events)
{
    return events.m_storage;
}

