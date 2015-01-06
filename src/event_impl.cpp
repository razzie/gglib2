/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <atomic>
#include <map>
#include <queue>
#include "event_impl.hpp"

static gg::FastMutex s_evt_types_mutex;
static std::map<gg::EventType, std::vector<const std::type_info*>> s_evt_types;

static std::atomic<gg::EventReceiverID> s_evt_receiver_id = { 1 };
static const size_t MSG_HANDLERS_SEPARATION_NUM = 4;
static gg::FastMutex s_evt_receivers_mutex[MSG_HANDLERS_SEPARATION_NUM];
static std::map<gg::EventReceiverID, gg::IEventReceiver*> s_evt_receivers[MSG_HANDLERS_SEPARATION_NUM];


bool gg::addEventType(gg::EventType evt_type, std::vector<const std::type_info*>&& types)
{
	std::lock_guard<gg::FastMutex> guard(s_evt_types_mutex);
	return s_evt_types.emplace(evt_type, std::move(types)).second;
}

static bool isEventValid(gg::EventType evt_type, const gg::VarArray& args)
{
//#ifdef GGLIB_DEBUG
	decltype(s_evt_types.begin()) it;

	{ // using exception-safe lock_guard in block
		std::lock_guard<gg::FastMutex> guard(s_evt_types_mutex);
		it = s_evt_types.find(evt_type);
	}

	if (it != s_evt_types.end())
	{
		auto& arg_types = it->second;
		const size_t arg_num = arg_types.size();

		if (arg_num != args.size()) return false;

		for (size_t i = 0; i < arg_num; ++i)
		{
			if (*arg_types[i] != args[i].getType()) return false;
		}

		return true;
	}
//#else
//	return true;
//#endif
}

unsigned gg::sendEvent(std::shared_ptr<gg::Event> evt, const std::vector<gg::EventReceiverID>& receiver_ids)
{
	if (!isEventValid(evt->getType(), evt->getArgs()))
		return 0;

	unsigned receiver_cnt = 0;

	for (auto receiver_id : receiver_ids)
	{
		if (receiver_id == 0) continue;

		std::lock_guard<gg::FastMutex> guard(s_evt_receivers_mutex[receiver_id % MSG_HANDLERS_SEPARATION_NUM]);
		auto it = s_evt_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].find(receiver_id);
		if (it != s_evt_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].end())
		{
			if (it->second->isEventTypeSupported(evt->getType()))
			{
				gg::EventReceiverAccessor(it->second).pushEvent(evt);
			}
		}
	}
	
	return receiver_cnt;
}


void gg::IEventReceiver::registerInstance(gg::IEventReceiver* receiver)
{
	EventReceiverID receiver_id = s_evt_receiver_id.fetch_add(1);
	EventReceiverAccessor(receiver).setID(receiver_id);

	std::lock_guard<gg::FastMutex> guard(s_evt_receivers_mutex[receiver_id % MSG_HANDLERS_SEPARATION_NUM]);
	s_evt_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].emplace(receiver_id, receiver);
}

void gg::IEventReceiver::unregisterInstance(gg::EventReceiverID receiver_id)
{
	if (receiver_id == 0) return;

	std::lock_guard<gg::FastMutex> guard(s_evt_receivers_mutex[receiver_id % MSG_HANDLERS_SEPARATION_NUM]);
	s_evt_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].erase(receiver_id);
}
