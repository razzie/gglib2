/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_MESSAGE_IMPL_HPP_INCLUDED
#define GG_MESSAGE_IMPL_HPP_INCLUDED

#include "gg/event.hpp"

namespace gg
{
	class EventReceiverAccessor
	{
	private:
		IEventReceiver* m_handler;

	public:
		EventReceiverAccessor(IEventReceiver* handler) :
			m_handler(handler)
		{
		}

		EventReceiverAccessor(const EventReceiverAccessor& o) :
			m_handler(o.m_handler)
		{
		}

		void setID(EventReceiverID id)
		{
			m_handler->m_id = id;
		}

		void pushEvent(std::shared_ptr<Event> evt)
		{
			std::lock_guard<gg::FastMutex> guard(m_handler->m_evt_queue_mutex);
			m_handler->m_evt_queue.push(evt);
		}
	};
};

#endif // GG_MESSAGE_IMPL_HPP_INCLUDED
