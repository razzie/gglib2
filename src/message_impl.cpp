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
static bool m_runtime_checks = true;

static std::atomic<gg::MessageReceiverID> s_msg_receiver_id = { 1 };
static const size_t MSG_HANDLERS_SEPARATION_NUM = 4;
static gg::FastMutex s_msg_receivers_mutex[MSG_HANDLERS_SEPARATION_NUM];
static std::map<gg::MessageReceiverID, gg::IMessageReceiver*> s_msg_receivers[MSG_HANDLERS_SEPARATION_NUM];


bool gg::addMessageType(gg::MessageType msg_type, std::vector<const std::type_info*>&& types)
{
	std::lock_guard<gg::FastMutex> guard(s_msg_types_mutex);
	return s_msg_types.emplace(msg_type, std::move(types)).second;
}

static bool isMessageValid(std::shared_ptr<gg::IMessage> msg)
{
	decltype(s_msg_types.begin()) it;

	{ // using exception-safe lock_guard in block
		std::lock_guard<gg::FastMutex> guard(s_msg_types_mutex);
		it = s_msg_types.find(msg->getType());
	}

	if (it != s_msg_types.end())
	{
		auto& arg_types = it->second;
		const size_t arg_num = arg_types.size();

		if (arg_num != msg->getParamCount()) return false;

		for (size_t i = 0; i < arg_num; ++i)
		{
			if (*arg_types[i] != msg->getParamType(i)) return false;
		}

		return true;
	}
}

unsigned gg::sendMessage(std::shared_ptr<gg::IMessage> msg, const std::vector<gg::MessageReceiverID>& receiver_ids)
{
	if (m_runtime_checks && !isMessageValid(msg))
		return 0;

	unsigned receiver_cnt = 0;

	for (auto receiver_id : receiver_ids)
	{
		if (receiver_id == 0) continue;

		std::lock_guard<gg::FastMutex> guard(s_msg_receivers_mutex[receiver_id % MSG_HANDLERS_SEPARATION_NUM]);
		auto it = s_msg_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].find(receiver_id);
		if (it != s_msg_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].end())
		{
			if (it->second->isMessageTypeSupported(msg->getType()))
			{
				gg::MessageReceiverAccessor(it->second).pushMessage(msg);
			}
		}
	}
	
	return receiver_cnt;
}

void gg::enableRuntimeMessageChecks(bool enabled)
{
	m_runtime_checks = enabled;
}


void gg::IMessageReceiver::registerInstance(gg::IMessageReceiver* receiver)
{
	MessageReceiverID receiver_id = s_msg_receiver_id.fetch_add(1);
	MessageReceiverAccessor(receiver).setID(receiver_id);

	std::lock_guard<gg::FastMutex> guard(s_msg_receivers_mutex[receiver_id % MSG_HANDLERS_SEPARATION_NUM]);
	s_msg_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].emplace(receiver_id, receiver);
}

void gg::IMessageReceiver::unregisterInstance(gg::MessageReceiverID receiver_id)
{
	if (receiver_id == 0) return;

	std::lock_guard<gg::FastMutex> guard(s_msg_receivers_mutex[receiver_id % MSG_HANDLERS_SEPARATION_NUM]);
	s_msg_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].erase(receiver_id);
}
