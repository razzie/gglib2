/**
 * Copyright (c) 2014-2015 G�bor G�rzs�ny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_MESSAGE_IMPL_HPP_INCLUDED
#define GG_MESSAGE_IMPL_HPP_INCLUDED

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
			std::lock_guard<gg::FastMutex> guard(m_handler->m_msg_queue_mutex);
			m_handler->m_msg_queue.push(msg);
		}

	private:
		IMessageReceiver* m_handler;
	};
};

#endif // GG_MESSAGE_IMPL_HPP_INCLUDED