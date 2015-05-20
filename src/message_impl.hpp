/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include "gg/message.hpp"

namespace gg
{
	class MessageReceiverAccessor
	{
	public:
		MessageReceiverAccessor(IMessageReceiver* handler) :
			m_handler(handler)
		{
		}

		MessageReceiverAccessor(const MessageReceiverAccessor& o) :
			m_handler(o.m_handler)
		{
		}

		void setID(MessageReceiverID id)
		{
			m_handler->m_id = id;
		}

		void pushMessage(std::shared_ptr<IMessage> msg)
		{
			std::lock_guard<decltype(m_handler->m_msg_queue_mutex)> guard(m_handler->m_msg_queue_mutex);
			m_handler->m_msg_queue.push(msg);
		}

	private:
		IMessageReceiver* m_handler;
	};
};
