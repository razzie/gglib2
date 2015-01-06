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
 * Classes that inherit from 'gg::msg::IMessageReceiver' are able to receive messages
 * sent from other modules of the program. Upon instantiating such a class, a unique
 * message handler ID is assigned to it. This ID can be used to send messages directly
 * to the new instance. You can do so by calling:
 * 'gg::msg::sendMessage(type, {args...}, {destination IDs}, optional sender ID)'
 *
 * Use 'gg::msg::IMessageHandler::getNextMessage()' function to receive messages.
 * Message arguments can be accessed by 'message->getArg(0)'.
 *
 * Messages have a message type (a number) and variable number of arguments. They
 * also have an optional sender, which is the ID of a message handler.
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
	typedef uint16_t MessageType; // 0 is invalid
	typedef uint16_t MessageHandlerID; // 0 is invalid
	typedef uint16_t MessageHandlerGroupID;

	class Message final
	{
	private:
		MessageType m_msg_type;
		VarArray m_args;
		MessageHandlerID m_sender; // 0 means not specified

	public:
		Message(MessageType msg_type, VarArray&& args, MessageHandlerID sender = 0) :
			m_msg_type(msg_type), m_args(args), m_sender(sender)
		{
		}

		MessageType getType() const
		{
			return m_msg_type;
		}

		const Var& getArg(size_t arg_num) const
		{
			return m_args[arg_num];
		}

		const VarArray& getArgs() const
		{
			return m_args;
		}

		MessageHandlerID getSender() const
		{
			return m_sender;
		}
	};

	bool addMessageType(MessageType, std::vector<const std::type_info*>&&);
	unsigned sendMessage(std::shared_ptr<Message>, const std::vector<MessageHandlerID>&);
	unsigned sendMessageToGroups(std::shared_ptr<Message>, const std::vector<MessageHandlerGroupID>&);

	inline unsigned sendMessage(MessageType msg_type, VarArray&& args, const std::vector<MessageHandlerID>& handler_ids, MessageHandlerID sender = 0)
	{
		std::shared_ptr<Message> msg(new Message(msg_type, std::move(args), sender));
		return sendMessage(msg, handler_ids);
	}

	inline unsigned sendMessageToGroups(MessageType msg_type, VarArray&& args, const std::vector<MessageHandlerGroupID>& group_ids, MessageHandlerID sender = 0)
	{
		std::shared_ptr<Message> msg(new Message(msg_type, std::move(args), sender));
		return sendMessageToGroups(msg, group_ids);
	}

	template<class... Args>
	bool addMessageType(MessageType msg_type)
	{
		// message types between 1-100 are internally reserved
		if (msg_type <= 100)
			return false;

		return addMessageType(msg_type, { typeid(Args)... });
	}

	class MessageHandlerAccessor;

	class IMessageHandler
	{
	private:
		friend class MessageHandlerAccessor;

		MessageHandlerID m_id;
		FastMutex m_msg_queue_mutex;
		std::queue<std::shared_ptr<Message>> m_msg_queue;
		std::vector<MessageType> m_msg_types;

		static void registerInstance(IMessageHandler*);
		static void unregisterInstance(MessageHandlerID);
		static void addToGroup(MessageHandlerID, MessageHandlerGroupID);

	protected:
		void addMessageType(MessageType msg_type)
		{
			for (MessageType t : m_msg_types)
				if (t == msg_type) return;

			m_msg_types.push_back(msg_type);
		}

		void addToGroup(MessageHandlerGroupID group_id) const
		{
			addToGroup(m_id, group_id);
		}

	public:
		IMessageHandler()
		{
			registerInstance(this); // sets the value of m_id
		}

		virtual ~IMessageHandler()
		{
			unregisterInstance(m_id);
		}

		MessageHandlerID getID() const
		{
			return m_id;
		}

		bool isMessageTypeSupported(MessageType msg_type) const
		{
			for (MessageType t : m_msg_types)
				if (t == msg_type) return true;

			return false;
		}

		std::shared_ptr<Message> getNextMessage()
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
	};
};

#endif // GG_MESSAGE_HPP_INCLUDED
