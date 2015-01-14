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
 * Classes that inherit from 'gg::IMessageReceiver' are able to receive messages
 * sent from other modules of the program. Upon instantiating such a class, a unique
 * message receiver ID is assigned to it. This ID can be used to send messages directly
 * to the new instance. You can do so by calling:
 * 'gg::sendMessage(type, args..., {destination IDs}, optional sender ID = 0)'
 *
 * Use 'gg::IMessageReceiver::getNextMessage()' function to receive queued messages.
 * Message arguments can be accessed by 'message->get<std::string>(param_num)'.
 * Parameter numbers are 0 based.
 *
 * Messages have an message type (a number) and variable number of arguments. They
 * also have an optional sender, which is the ID of an message receiver.
 *
 * An message type should be registered before sending any message of its kind:
 * 'gg::addMessageType<int, float, std::string>(123)'
 */

#ifndef GG_MESSAGE_HPP_INCLUDED
#define GG_MESSAGE_HPP_INCLUDED

#include <cstdint>
#include <memory>
#include <queue>
#include <stdexcept>
#include <typeinfo>
#include <vector>
#include "gg/fastmutex.hpp"
#include "gg/storage.hpp"

namespace gg
{
	typedef uint16_t MessageType; // 0 is invalid
	typedef uint16_t MessageReceiverID; // 0 is invalid

	class IMessage : public IStorage
	{
	public:
		IMessage(MessageType type) :
			m_type(type), m_sender(0)
		{
		}

		virtual ~IMessage() {}

		MessageType getMessageType() const
		{
			return m_type;
		}

		void setSender(MessageReceiverID sender)
		{
			m_sender = sender;
		}

		MessageReceiverID getSender() const
		{
			return m_sender;
		}

		// inherited from IStorage
		virtual unsigned size() const = 0; // number of elements
		virtual char* getPtr(unsigned) = 0;
		virtual const char* getPtr(unsigned) const = 0;
		virtual const std::type_info& getType(unsigned) const = 0;

	private:
		MessageType m_type;
		MessageReceiverID m_sender;
	};

	template<class... Params>
	class Message : public IMessage
	{
	public:
		Message(MessageType type, Params... params) :
			IMessage(type),
			m_storage(std::forward<Params>(params)...)
		{
		}

		// inherited from IMessage (through IStorage)
		virtual unsigned size() const
		{
			return m_storage.size();
		}

		virtual char* getPtr(unsigned n)
		{
			return m_storage.getPtr(n);
		}

		virtual const char* getPtr(unsigned n) const
		{
			return m_storage.getPtr(n);
		}

		virtual const std::type_info& getType(unsigned n) const
		{
			return m_storage.getType(n);
		}

	private:
		Storage<Params...> m_storage;
	};

	bool addMessageType(MessageType, std::vector<const std::type_info*>&&);
	unsigned sendMessage(std::shared_ptr<IMessage>, const std::vector<MessageReceiverID>&);
	void enableRuntimeMessageChecks(bool = true); // enabled by default

	template<class... Args>
	bool addMessageType(MessageType type) // message types between 1-100 are reserved
	{
		return addMessageType(type, { &typeid(Args)... });
	}

	template<class... Params>
	unsigned sendMessage(MessageType type, Params... params, const std::vector<MessageReceiverID>& receivers, MessageReceiverID sender = 0)
	{
		std::shared_ptr<IMessage> msg(new Message<Params...>(type, std::forward<Params>(params)...));
		msg->setSender(sender);
		return sendMessage(msg, receivers);
	}

	class IMessageReceiver
	{
	public:
		IMessageReceiver()
		{
			registerInstance(this); // sets the value of m_id
		}

		virtual ~IMessageReceiver()
		{
			unregisterInstance(m_id);
		}

		MessageReceiverID getID() const
		{
			return m_id;
		}

		bool isMessageTypeSupported(MessageType type) const
		{
			for (MessageType t : m_msg_types)
				if (t == type) return true;

			return false;
		}

		bool isMessageAvailable() const
		{
			return !m_msg_queue.empty();
		}

		std::shared_ptr<IMessage> getNextMessage()
		{
			std::lock_guard<FastMutex> guard(m_msg_queue_mutex);

			if (!m_msg_queue.empty())
			{
				auto msg = m_msg_queue.front();
				m_msg_queue.pop();
				return msg;
			}
			else
			{
				return {};
			}
		}

	protected:
		void addMessageType(MessageType type)
		{
			for (MessageType t : m_msg_types)
				if (t == type) return;

			m_msg_types.push_back(type);
		}

	private:
		friend class MessageReceiverAccessor;

		MessageReceiverID m_id;
		FastMutex m_msg_queue_mutex;
		std::queue<std::shared_ptr<IMessage>> m_msg_queue;
		std::vector<MessageType> m_msg_types;

		static void registerInstance(IMessageReceiver*);
		static void unregisterInstance(MessageReceiverID);
	};
};

#endif // GG_MESSAGE_HPP_INCLUDED
