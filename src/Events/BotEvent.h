/*
 * This file is part of tswow (https://github.com/tswow/).
 * Copyright (C) 2020-2022 tswow <https://github.com/tswow/>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

// Modified version of TSEvent.h that supports extensions, and removes mapped id events
#pragma once

#include <vector>

#include "sol/sol.hpp"

template <class C>
struct TSEvent;

template <class C>
struct TSEvent
{
		using cxx_callbacks = std::vector<C>;
		using lua_callbacks = std::vector<sol::protected_function>;
		using cxx_id_callbacks = std::vector<cxx_callbacks>;
		using lua_id_callbacks = std::vector<lua_callbacks>;
		cxx_callbacks m_cxx_callbacks;
		lua_callbacks m_lua_callbacks;
		cxx_id_callbacks m_id_cxx_callbacks;
		lua_id_callbacks m_id_lua_callbacks;

		bool has_non_id_entries()
		{
				return m_cxx_callbacks.size() > 0 || m_lua_callbacks.size() > 0;
		}

		void extend(TSEvent<C> const& evt)
		{
				for (C callback : evt.m_cxx_callbacks)
				{
						m_cxx_callbacks.push_back(callback);
				}

				for (sol::protected_function callback : evt.m_lua_callbacks)
				{
						m_lua_callbacks.push_back(callback);
				}

				if (m_id_cxx_callbacks.size() < evt.m_id_cxx_callbacks.size())
				{
						m_id_cxx_callbacks.resize(evt.m_id_cxx_callbacks.size());
				}

				if (m_id_lua_callbacks.size() < evt.m_id_lua_callbacks.size())
				{
						m_id_lua_callbacks.resize(evt.m_id_lua_callbacks.size());
				}

				for (int i = 0; i < evt.m_id_cxx_callbacks.size(); ++i)
				{
						for (C callback : evt.m_id_cxx_callbacks[i])
						{
								m_id_cxx_callbacks[i].push_back(callback);
						}
				}

				for (int i = 0; i < evt.m_id_lua_callbacks.size(); ++i)
				{
						for (sol::protected_function callback : evt.m_id_lua_callbacks[i])
						{
								m_id_lua_callbacks[i].push_back(callback);
						}
				}
		}

		void clear()
		{
				m_cxx_callbacks.clear();
				m_lua_callbacks.clear();
				for (cxx_callbacks& cb : m_id_cxx_callbacks)
				{
						cb.clear();
				}
				for (lua_callbacks& cb : m_id_lua_callbacks)
				{
						cb.clear();
				}
		}
};

class TSMappedEvents
{
protected:
		virtual uint32_t get_registry_id(uint32_t id) = 0;
};

class TSMappedEventsDirect : public TSMappedEvents
{
protected:
		uint32_t get_registry_id(uint32_t id) final override
		{
				return id;
		}
};

#define EVENT_STORAGE(name,...) \
		using name##__type = void (*)(__VA_ARGS__);\
		TSEvent<name##__type> name##_callbacks;\

#define EVENT_ROOT(name,is_fn,fn_cxx,fn_lua) \
		BotEvents name(BotEventData::name##__type cb) {\
				m_storage->name##_callbacks.m_cxx_callbacks.push_back(cb);\
				if(is_fn) fn_cxx(cb);\
				return *this;\
		}\
		BotEvents L##name(sol::protected_function cb)\
		{\
				m_storage->name##_callbacks.m_lua_callbacks.push_back(cb);\
				if(is_fn) fn_lua(cb);\
				return *this;\
		}\

#define ID_EVENT_ROOT(name,is_fn,fn_plain_cxx,fn_plain_lua,fn_mapped_cxx,fn_mapped_lua)\
		EVENT_ROOT(name,is_fn,fn_plain_cxx,fn_plain_lua)\
		BotEvents name(uint32_t reg_id, BotEventData::name##__type cb)\
		{\
				auto & cbs = m_storage->name##_callbacks.m_id_cxx_callbacks;\
				if(reg_id >= cbs.size())\
				{\
						cbs.resize(uint64_t(reg_id) + 1);\
				}\
				cbs[reg_id].push_back(cb);\
				if (is_fn) fn_mapped_cxx(cb,reg_id);\
				return *this;\
		}\
		BotEvents name(std::vector<uint32_t> ids, BotEventData::name##__type cb) {\
				for(uint32_t id : ids)\
				{\
						name(id, cb);\
				}\
				return *this;\
		}\
		BotEvents _L##name(uint32_t reg_id, sol::protected_function cb)\
		{\
				auto& cbs = m_storage->name##_callbacks.m_id_lua_callbacks;\
				if(reg_id >= cbs.size())\
				{\
						cbs.resize(uint64_t(reg_id) + 1);\
				}\
				cbs[reg_id].push_back(cb);\
				if (is_fn) fn_mapped_lua(cb,reg_id);\
				return *this;\
		}\
		BotEvents Lid##name(sol::object obj, sol::protected_function cb)\
		{\
				switch(obj.get_type())\
				{\
						case sol::type::number:\
								_L##name(obj.as<uint32_t>(), cb);\
								break;\
						case sol::type::table:\
								sol::table table = obj.as<sol::table>();\
								for(size_t i = 1; i <= table.size(); ++i)\
								{\
										_L##name((uint32_t)table.get<double>(i), cb);\
								}\
								break;\
				}\
				return *this;\
		}\

#define EVENT_FN(name,fn)\
		EVENT_ROOT(name,true,fn,fn)

#define ID_EVENT_FN(name,fn)\
		ID_EVENT_ROOT(name,true,fn,fn,fn,fn)

#define EVENT(name)\
		EVENT_ROOT(name,false,[](BotEventData::name##__type){},[](sol::protected_function){})

#define ID_EVENT(name)\
		ID_EVENT_ROOT(name,false,[](BotEventData::name##__type){},[](sol::protected_function){},[](BotEventData::name##__type,uint32_t){},[](sol::protected_function,uint32_t){});

#define FIRE(name,event_target,setup,...)\
		{\
				for(auto cb : BotEventsMgr::GetStorage(event_target)->name##_callbacks.m_cxx_callbacks)\
				{\
						setup\
						cb(__VA_ARGS__);\
				}\
				\
				for(auto cb : BotEventsMgr::GetStorage(event_target)->name##_callbacks.m_lua_callbacks)\
				{\
					try\
					{\
						setup\
						cb(__VA_ARGS__);\
					}\
					catch (std::exception const& e)\
					{\
						BOT_LOG_ERROR("event","Error handling event " #name ": %s",e.what());\
					}\
					catch (...)\
					{\
						BOT_LOG_ERROR("event","Error handling event " #name ": Unknown Error");\
					}\
				}\
		}\

#define FIRE_ID(ref,name,event_target,setup,...)\
		{\
				FIRE(name,event_target,setup,__VA_ARGS__)\
				auto cxx_cbs = BotEventsMgr::GetStorage(event_target)->name##_callbacks.m_id_cxx_callbacks;\
				if(ref < cxx_cbs.size())\
				{\
						for(auto cb: cxx_cbs[ref])\
						{\
								setup\
								cb(__VA_ARGS__);\
						}\
				}\
				auto lua_cbs = BotEventsMgr::GetStorage(event_target)->name##_callbacks.m_id_lua_callbacks;\
				if(ref < lua_cbs.size())\
				{\
						for(auto cb: lua_cbs[ref])\
						{\
								try\
								{\
										setup\
										cb(__VA_ARGS__);\
								}\
								catch (std::exception const& e)\
								{\
									BOT_LOG_ERROR("event","Error handling event " #name ": %s",e.what());\
								}\
								catch (...)\
								{\
									BOT_LOG_ERROR("event","Error handling event " #name ": Unknown Error");\
								}\
						}\
				}\
		}

#define EXTEND_EVENT(target,evt,name) target->name##_callbacks.extend(evt->name##_callbacks)
