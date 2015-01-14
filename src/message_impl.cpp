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

static const size_t MSG_HANDLERS_SEPARATION_NUM = 4;

struct GlobalsMSG
{
	gg::FastMutex msg_types_mutex;
	std::map<gg::MessageType, std::vector<const std::type_info*>> msg_types;
	bool runtime_checks = true;

	std::atomic<gg::MessageReceiverID> msg_receiver_id = { 1 };
	gg::FastMutex msg_receivers_mutex[MSG_HANDLERS_SEPARATION_NUM];
	std::map<gg::MessageReceiverID, gg::IMessageReceiver*> msg_receivers[MSG_HANDLERS_SEPARATION_NUM];
};

static GlobalsMSG globals;


bool gg::addMessageType(gg::MessageType msg_type, std::vector<const std::type_info*>&& types)
{
	std::lock_guard<gg::FastMutex> guard(globals.msg_types_mutex);
	return globals.msg_types.emplace(msg_type, std::move(types)).second;
}

static bool isMessageValid(std::shared_ptr<gg::IMessage> msg)
{
	decltype(globals.msg_types.begin()) it;

	{ // using exception-safe lock_guard in block
		std::lock_guard<gg::FastMutex> guard(globals.msg_types_mutex);
		it = globals.msg_types.find(msg->getMessageType());
	}

	if (it != globals.msg_types.end())
	{
		auto& arg_types = it->second;
		const size_t arg_num = arg_types.size();

		if (arg_num != msg->size()) return false;

		for (size_t i = 0; i < arg_num; ++i)
		{
			if (*arg_types[i] != msg->getType(i)) return false;
		}

		return true;
	}

	return false;
}

unsigned gg::sendMessage(std::shared_ptr<gg::IMessage> msg, const std::vector<gg::MessageReceiverID>& receiver_ids)
{
	if (globals.runtime_checks && !isMessageValid(msg))
		return 0;

	unsigned receiver_cnt = 0;

	for (auto receiver_id : receiver_ids)
	{
		if (receiver_id == 0) continue;

		std::lock_guard<gg::FastMutex> guard(globals.msg_receivers_mutex[receiver_id % MSG_HANDLERS_SEPARATION_NUM]);
		auto it = globals.msg_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].find(receiver_id);
		if (it != globals.msg_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].end())
		{
			if (it->second->isMessageTypeSupported(msg->getMessageType()))
			{
				gg::MessageReceiverAccessor(it->second).pushMessage(msg);
				++receiver_cnt;
			}
		}
	}
	
	return receiver_cnt;
}

void gg::enableRuntimeMessageChecks(bool enabled)
{
	globals.runtime_checks = enabled;
}


void gg::IMessageReceiver::registerInstance(gg::IMessageReceiver* receiver)
{
	MessageReceiverID receiver_id = globals.msg_receiver_id.fetch_add(1);
	MessageReceiverAccessor(receiver).setID(receiver_id);

	std::lock_guard<gg::FastMutex> guard(globals.msg_receivers_mutex[receiver_id % MSG_HANDLERS_SEPARATION_NUM]);
	globals.msg_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].emplace(receiver_id, receiver);
}

void gg::IMessageReceiver::unregisterInstance(gg::MessageReceiverID receiver_id)
{
	if (receiver_id == 0) return;

	std::lock_guard<gg::FastMutex> guard(globals.msg_receivers_mutex[receiver_id % MSG_HANDLERS_SEPARATION_NUM]);
	globals.msg_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].erase(receiver_id);
}
