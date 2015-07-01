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
#include "gg/fastmutex.hpp"
#include "safebuffer.hpp"
#include "message_impl.hpp"

static const size_t MSG_HANDLERS_SEPARATION_NUM = 4;

struct GlobalsMSG
{
	gg::FastMutex msg_types_mutex;
	std::map<gg::MessageType, gg::MessageConstructor> msg_types;

	std::atomic<gg::MessageReceiverID> msg_receiver_id = { 1 };
	gg::FastMutex msg_receivers_mutex[MSG_HANDLERS_SEPARATION_NUM];
	std::map<gg::MessageReceiverID, gg::IMessageReceiver*> msg_receivers[MSG_HANDLERS_SEPARATION_NUM];
};

static GlobalsMSG globals;


bool gg::addMessageType(gg::MessageType msg_type, MessageConstructor msg_ctor)
{
	std::lock_guard<decltype(globals.msg_types_mutex)> guard(globals.msg_types_mutex);
	return globals.msg_types.emplace(msg_type, msg_ctor).second;
}

bool gg::sendMessage(std::shared_ptr<gg::IMessage> msg, MessageReceiverID receiver_id)
{
	if (receiver_id == 0) return false;

	std::lock_guard<decltype(globals.msg_receivers_mutex[receiver_id % MSG_HANDLERS_SEPARATION_NUM])>
		guard(globals.msg_receivers_mutex[receiver_id % MSG_HANDLERS_SEPARATION_NUM]);

	auto it = globals.msg_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].find(receiver_id);
	if (it != globals.msg_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].end())
	{
		if (it->second->isMessageTypeSupported(msg->getMessageType()))
		{
			gg::MessageReceiverAccessor(it->second).pushMessage(msg);
			return true;
		}
	}

	return false;
}

std::shared_ptr<gg::IMessage> gg::deserializeMessage(gg::IBuffer& buf)
{
	MessageType type;
	gg::MessageConstructor msg_ctor;
	gg::SafeBuffer tmp_buf(buf);

	if (buf.peek(reinterpret_cast<char*>(&type), sizeof(type)) < sizeof(type))
		return{};

	{ // exception-safe mutex locking
		std::lock_guard<decltype(globals.msg_types_mutex)> guard(globals.msg_types_mutex);
		auto it = globals.msg_types.find(type);
		if (it == globals.msg_types.end())
			return{};
		else
			msg_ctor = it->second;
	}

	std::shared_ptr<gg::IMessage> msg = msg_ctor(tmp_buf);
	if (msg) tmp_buf.finalize();

	return msg;
}


void gg::IMessageReceiver::registerInstance(gg::IMessageReceiver* receiver)
{
	MessageReceiverID receiver_id = globals.msg_receiver_id.fetch_add(1);
	MessageReceiverAccessor(receiver).setID(receiver_id);

	std::lock_guard<decltype(globals.msg_receivers_mutex[receiver_id % MSG_HANDLERS_SEPARATION_NUM])>
		guard(globals.msg_receivers_mutex[receiver_id % MSG_HANDLERS_SEPARATION_NUM]);

	globals.msg_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].emplace(receiver_id, receiver);
}

void gg::IMessageReceiver::unregisterInstance(gg::MessageReceiverID receiver_id)
{
	if (receiver_id == 0) return;

	std::lock_guard<decltype(globals.msg_receivers_mutex[receiver_id % MSG_HANDLERS_SEPARATION_NUM])>
		guard(globals.msg_receivers_mutex[receiver_id % MSG_HANDLERS_SEPARATION_NUM]);

	globals.msg_receivers[receiver_id % MSG_HANDLERS_SEPARATION_NUM].erase(receiver_id);
}
