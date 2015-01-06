/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

/**
 * HOW TO USE:
 * -----------
 *
 * Classes that inherit from 'gg::msg::IEventReceiver' are able to receive events
 * sent from other modules of the program. Upon instantiating such a class, a unique
 * event receiver ID is assigned to it. This ID can be used to send events directly
 * to the new instance. You can do so by calling:
 * 'gg::msg::sendEvent(type, {args...}, {destination IDs}, optional sender ID)'
 *
 * Use 'gg::msg::IEventReceiver::getNextEvent()' function to receive queued events.
 * Event arguments can be accessed by 'message->getArg(0)'.
 *
 * Events have an event type (a number) and variable number of arguments. They
 * also have an optional sender, which is the ID of an event receiver.
 */

#ifndef GG_MESSAGE_HPP_INCLUDED
#define GG_MESSAGE_HPP_INCLUDED

#include <cstdint>
#include <memory>
#include <queue>
#include <typeinfo>
#include <vector>
#include "gg/fastmutex.hpp"
#include "gg/var.hpp"

namespace gg
{
	typedef uint16_t EventType; // 0 is invalid
	typedef uint16_t EventReceiverID; // 0 is invalid
	typedef uint16_t EventReceiverGroupID;

	class Event final
	{
	private:
		EventType m_type;
		VarArray m_args;
		EventReceiverID m_sender; // 0 means not specified

	public:
		Event(EventType type, VarArray&& args, EventReceiverID sender = 0) :
			m_type(type), m_args(args), m_sender(sender)
		{
		}

		EventType getType() const
		{
			return m_type;
		}

		const Var& getArg(size_t arg_num) const
		{
			return m_args[arg_num];
		}

		const VarArray& getArgs() const
		{
			return m_args;
		}

		EventReceiverID getSender() const
		{
			return m_sender;
		}
	};

	bool addEventType(EventType, std::vector<const std::type_info*>&&);
	unsigned sendEvent(std::shared_ptr<Event>, const std::vector<EventReceiverID>&);
	unsigned sendEventToGroups(std::shared_ptr<Event>, const std::vector<EventReceiverGroupID>&);

	inline unsigned sendEvent(EventType type, VarArray&& args, const std::vector<EventReceiverID>& receiver_ids, EventReceiverID sender = 0)
	{
		std::shared_ptr<Event> msg(new Event(type, std::move(args), sender));
		return sendEvent(msg, receiver_ids);
	}

	inline unsigned sendEventToGroups(EventType type, VarArray&& args, const std::vector<EventReceiverGroupID>& group_ids, EventReceiverID sender = 0)
	{
		std::shared_ptr<Event> msg(new Event(type, std::move(args), sender));
		return sendEventToGroups(msg, group_ids);
	}

	template<class... Args>
	bool addEventType(EventType type)
	{
		// message types between 1-100 are internally reserved
		if (type <= 100)
			return false;

		return addEventType(type, { typeid(Args)... });
	}

	class EventReceiverAccessor;

	class IEventReceiver
	{
	private:
		friend class EventReceiverAccessor;

		EventReceiverID m_id;
		FastMutex m_evt_queue_mutex;
		std::queue<std::shared_ptr<Event>> m_evt_queue;
		std::vector<EventType> m_types;

		static void registerInstance(IEventReceiver*);
		static void unregisterInstance(EventReceiverID);
		static void addToGroup(EventReceiverID, EventReceiverGroupID);

	protected:
		void addEventType(EventType type)
		{
			for (EventType t : m_types)
				if (t == type) return;

			m_types.push_back(type);
		}

		void addToGroup(EventReceiverGroupID group_id) const
		{
			addToGroup(m_id, group_id);
		}

	public:
		IEventReceiver()
		{
			registerInstance(this); // sets the value of m_id
		}

		virtual ~IEventReceiver()
		{
			unregisterInstance(m_id);
		}

		EventReceiverID getID() const
		{
			return m_id;
		}

		bool isEventTypeSupported(EventType type) const
		{
			for (EventType t : m_types)
				if (t == type) return true;

			return false;
		}

		std::shared_ptr<Event> getNextEvent()
		{
			std::lock_guard<FastMutex> guard(m_evt_queue_mutex);

			if (!m_evt_queue.empty())
			{
				auto msg = m_evt_queue.front();
				m_evt_queue.pop();
				return msg;
			}
			else
			{
				return {};
			}
		}
	};
};

#endif // GG_MESSAGE_HPP_INCLUDED
