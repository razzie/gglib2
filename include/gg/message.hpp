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
 * Message arguments can be accessed by 'message->getParam<std::string>(param_num)'.
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
#include <typeinfo>
#include <vector>
#include "gg/fastmutex.hpp"
#include "gg/storage.hpp"

namespace gg
{
	typedef uint16_t MessageType; // 0 is invalid
	typedef uint16_t MessageReceiverID; // 0 is invalid

	class IMessage
	{
	public:
		IMessage(MessageType type) :
			m_type(type), m_sender(0)
		{
		}

		MessageType getType() const
		{
			return m_type;
		}

		virtual ~IMessage() {}
		virtual char* getParamPtr(unsigned) = 0;
		virtual const char* getParamPtr(unsigned) const = 0;
		virtual const std::type_info& getParamType(unsigned) const = 0;
		virtual unsigned getParamCount() const = 0;

		template<class T>
		T& getParam(unsigned n)
		{
			if (n >= getParamCount() || typeid(T) != getParamType(n)) throw std::bad_cast();
			return *reinterpret_cast<T*>(getParamPtr(n));
		}

		template<class T>
		const T& getParam(unsigned n) const
		{
			if (n >= getParamCount() || typeid(T) != getParamType(n)) throw std::bad_cast();
			return *reinterpret_cast<const T*>(getParamPtr(n));
		}

		void setSender(MessageReceiverID sender)
		{
			m_sender = sender;
		}

		MessageReceiverID getSender() const
		{
			return m_sender;
		}

	private:
		MessageType m_type;
		MessageReceiverID m_sender;
	};

	template<class... Params>
	class Message : public IMessage, public Storage<Params...>
	{
	public:
		Message(MessageType type, Params... params) :
			IMessage(type),
			Storage(std::forward<Params>(params)...)
		{
		}

		virtual char* getParamPtr(unsigned n)
		{
			return Storage::getPtr(n);
		}

		virtual const char* getParamPtr(unsigned n) const
		{
			return Storage::getPtr(n);
		}

		virtual const std::type_info& getParamType(unsigned n) const
		{
			return Storage::getType(n);
		}

		virtual unsigned getParamCount() const
		{
			return Storage::count();
		}
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
