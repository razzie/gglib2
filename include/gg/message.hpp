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
#include <functional>
#include <memory>
#include <queue>
#include <stdexcept>
#include <typeinfo>
#include "gg/fastmutex.hpp"
#include "gg/storage.hpp"
#include "gg/serializer.hpp"

namespace gg
{
	typedef uint16_t MessageType; // 0 is invalid
	typedef uint16_t MessageReceiverID; // 0 is invalid

	class IMessage : public IStorage, public ISerializable
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

		// inherited from ISerializable
		virtual bool serialize(IBuffer& buf) const
		{
			bool result =
				gg::serialize(m_type, buf) &&
				gg::serialize(m_sender, buf) &&
				gg::serialize(static_cast<const IStorage&>(*this), buf);
			return result;
		}

		virtual bool deserialize(IBuffer& buf)
		{
			bool result =
				gg::deserialize(m_type, buf) &&
				gg::deserialize(m_sender, buf) &&
				gg::deserialize(static_cast<IStorage&>(*this), buf);
			return result;
		}

	private:
		MessageType m_type;
		MessageReceiverID m_sender;
	};

	template<class... Params>
	class Message : public IMessage
	{
	public:
		Message(MessageType type) :
			IMessage(type),
			m_storage() // default constructs parameters
		{
		}

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

	template<class... Params>
	std::shared_ptr<IMessage> createMessage(MessageType type, Params... params)
	{
		return std::shared_ptr<IMessage>(new Message<Params...>(type, std::forward<Params>(params)...));
	}

	typedef std::function<std::shared_ptr<IMessage>(IBuffer&)> MessageConstructor;
	bool addMessageType(MessageType, MessageConstructor);
	bool sendMessage(std::shared_ptr<IMessage>, MessageReceiverID);
	std::shared_ptr<IMessage> deserializeMessage(IBuffer&);

	template<class... Params>
	bool addMessageType(MessageType type) // message types between 1-100 are reserved
	{
		MessageConstructor msg_ctor =
			[type](IBuffer& buf) -> std::shared_ptr<IMessage>
		{
			std::shared_ptr<IMessage> msg(new Message<Params...>(type));
			if (msg->deserialize(buf))
				return msg;
			else
				return {};
		};

		return addMessageType(type, msg_ctor);
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

		virtual bool isMessageTypeSupported(MessageType) const
		{
			return true;
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

	private:
		friend class MessageReceiverAccessor;

		MessageReceiverID m_id;
		FastMutex m_msg_queue_mutex;
		std::queue<std::shared_ptr<IMessage>> m_msg_queue;

		static void registerInstance(IMessageReceiver*);
		static void unregisterInstance(MessageReceiverID);
	};
};

#endif // GG_MESSAGE_HPP_INCLUDED
