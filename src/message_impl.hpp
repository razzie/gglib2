/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
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
	namespace msg
	{
		class MessageHandlerAccessor
		{
		private:
			IMessageHandler* m_handler;

		public:
			MessageHandlerAccessor(IMessageHandler* handler) :
				m_handler(handler)
			{
			}

			MessageHandlerAccessor(const MessageHandlerAccessor& o) :
				m_handler(o.m_handler)
			{
			}

			void setID(MessageHandlerID id)
			{
				m_handler->m_id = id;
			}

			void pushMessage(std::shared_ptr<Message> msg)
			{
				std::lock_guard<gg::FastMutex> guard(m_handler->m_msg_queue_mutex);
				m_handler->m_msg_queue.push(msg);
			}
		};
	};
};

#endif // GG_MESSAGE_IMPL_HPP_INCLUDED
