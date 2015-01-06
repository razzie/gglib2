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
#include "message_impl.hpp"

static gg::FastMutex s_msg_types_mutex;
static std::map<gg::MessageType, std::vector<const std::type_info*>> s_msg_types;

static std::atomic<gg::MessageHandlerID> s_msg_handler_id = { 1 };
static const size_t MSG_HANDLERS_SEPARATION_NUM = 4;
static gg::FastMutex s_msg_handlers_mutex[MSG_HANDLERS_SEPARATION_NUM];
static std::map<gg::MessageHandlerID, gg::IMessageHandler*> s_msg_handlers[MSG_HANDLERS_SEPARATION_NUM];

static const size_t MSG_GROUPS_SEPARATION_NUM = 4;
static gg::FastMutex s_groups_mutex[MSG_GROUPS_SEPARATION_NUM];
static std::map<gg::MessageHandlerGroupID, std::vector<gg::MessageHandlerID>> s_groups[MSG_GROUPS_SEPARATION_NUM];


bool gg::addMessageType(gg::MessageType msg_type, std::vector<const std::type_info*>&& types)
{
	std::lock_guard<gg::FastMutex> guard(s_msg_types_mutex);
	return s_msg_types.emplace(msg_type, std::move(types)).second;
}

static bool isMessageValid(gg::MessageType msg_type, const gg::VarArray& args)
{
#ifdef GGLIB_DEBUG
	decltype(s_msg_types.begin()) it;

	{ // using exception-safe lock_guard in block
		std::lock_guard<gg::FastMutex> guard(s_msg_types_mutex);
		it = s_msg_types.find(msg_type);
	}

	if (it != s_msg_types.end())
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
#else
	return true;
#endif
}

unsigned gg::sendMessage(std::shared_ptr<gg::Message> msg, const std::vector<gg::MessageHandlerID>& handler_ids)
{
	if (!isMessageValid(msg->getType(), msg->getArgs()))
		return 0;

	unsigned receiver_cnt = 0;

	for (auto handler_id : handler_ids)
	{
		if (handler_id == 0) continue;

		std::lock_guard<gg::FastMutex> guard(s_msg_handlers_mutex[handler_id % MSG_HANDLERS_SEPARATION_NUM]);
		auto it = s_msg_handlers[handler_id % MSG_HANDLERS_SEPARATION_NUM].find(handler_id);
		if (it != s_msg_handlers[handler_id % MSG_HANDLERS_SEPARATION_NUM].end())
		{
			if (it->second->isMessageTypeSupported(msg->getType()))
			{
				gg::MessageHandlerAccessor(it->second).pushMessage(msg);
			}
		}
	}
	
	return receiver_cnt;
}

unsigned gg::sendMessageToGroups(std::shared_ptr<gg::Message> msg, const std::vector<gg::MessageHandlerGroupID>& group_ids)
{
	return 0;
}


void gg::IMessageHandler::registerInstance(gg::IMessageHandler* handler)
{
	MessageHandlerID handler_id = s_msg_handler_id.fetch_add(1);
	MessageHandlerAccessor(handler).setID(handler_id);

	std::lock_guard<gg::FastMutex> guard(s_msg_handlers_mutex[handler_id % MSG_HANDLERS_SEPARATION_NUM]);
	s_msg_handlers[handler_id % MSG_HANDLERS_SEPARATION_NUM].emplace(handler_id, handler);
}

void gg::IMessageHandler::unregisterInstance(gg::MessageHandlerID handler_id)
{
	if (handler_id == 0) return;

	std::lock_guard<gg::FastMutex> guard(s_msg_handlers_mutex[handler_id % MSG_HANDLERS_SEPARATION_NUM]);
	s_msg_handlers[handler_id % MSG_HANDLERS_SEPARATION_NUM].erase(handler_id);
}

void gg::IMessageHandler::addToGroup(gg::MessageHandlerID handler_id, gg::MessageHandlerGroupID group_id)
{
	if (handler_id == 0 || group_id == 0) return;
	std::lock_guard<gg::FastMutex> guard(s_groups_mutex[group_id % MSG_GROUPS_SEPARATION_NUM]);
	(s_groups[group_id % MSG_GROUPS_SEPARATION_NUM])[group_id].push_back(handler_id);
}
