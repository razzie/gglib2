/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_MESSAGE_HPP_INCLUDED
#define GG_MESSAGE_HPP_INCLUDED

#include <cstdint>
#include <memory>
#include <typeinfo>
#include <vector>
#include "gg/var.hpp"

namespace gg
{
	namespace msg
	{
		typedef uint16_t MessageType; // 0 is invalid
		typedef uint16_t MessageHandlerID; // 0 is invalid
		typedef uint16_t MessageHandlerGroupID; // 0 is invalid

		class Message
		{
		private:
			template<class Arg0, class... Args>
			static bool getTypes(std::vector<const std::type_info*>& types)
			{
				types.push_back(&typeid(Arg0));
				return getTypes<Args...>(types);
			}

			static void registerMessageType(MessageType msg_type, std::vector<const std::type_info*>&& types);
			static bool isValid(MessageType msg_type, const VarArray& args);
			static bool sendMessage(MessageType msg_type, MessageHandlerID handler_id, VarArray&& args);
			static bool sendMessageToGroup(MessageType msg_type, MessageHandlerGroupID group_id, VarArray&& args);

		protected:
			MessageType m_msg_type;
			MessageHandlerID m_sender; // 0 means not specified
			VarArray m_args;

		public:
			template<class... Args>
			static void registerType(MessageType msg_type)
			{
				std::vector<const std::type_info*> types;
				getTypes<Args...>(types);
				registerMessageType(msg_type, std::move(types));
			}

			Message(MessageType msg_type, MessageHandlerID sender = 0) :
				m_msg_type(msg_type),
				m_sender(sender)
			{
			}

			template<class... Args>
			Message(MessageType msg_type, MessageHandlerID sender = 0, Args...) :
				m_msg_type(msg_type),
				m_sender(sender),
				m_args(std::forward<Args>(args)...)
			{
			}

			Message(Message&& msg) :
				m_msg_type(msg.m_msg_type),
				m_sender(msg.m_sender),
				m_args(std::move(msg.m_args))
			{
			}

			template<class... Args>
			void insert(Args... args)
			{
				m_args.insert({ std::forward<Args>(args)... });
			}
			
			Var& operator[](size_t arg_num)
			{
				return m_args[arg_num];
			}

			const Var& operator[](size_t arg_num) const
			{
				return m_args[arg_num];
			}

			MessageHandlerID getSender() const
			{
				return m_sender;
			}

			// sends the message to a message handler (and possibly invalidates it)
			bool send(MessageHandlerID handler_id)
			{
				if (!isValid(m_msg_type, m_args))
					return false;

				return sendMessage(m_msg_type, handler_id, std::move(m_args));
			}

			// sends the message to a message handler group (and possibly invalidates it)
			bool sendToGroup(MessageHandlerGroupID group_id)
			{
				if (!isValid(m_msg_type, m_args))
					return false;

				return sendMessageToGroup(m_msg_type, group_id, std::move(m_args));
			}
		};

		template<class... Args>
		bool sendMessage(MessageType msg_type, MessageHandlerID handler_id, Args... args)
		{
			return Message(msg_type, 0, std::forward<Args>(args)...).send(handler_id);
		}

		template<class... Args>
		bool sendMessageToGroup(MessageType msg_type, MessageHandlerGroupID group_id, Args... args)
		{
			return Message(msg_type, 0, std::forward<Args>(args)...).sendToGroup(group_id);
		}

		class IMessageHandler
		{
		private:
			static std::shared_ptr<Message> getNextMessage(const IMessageHandler*);
			static void removeMessageHandler(const IMessageHandler*);
			static MessageHandlerID registerMessageHandler(
				const IMessageHandler*,
				MessageHandlerGroupID group_id = 0,
				MessageHandlerID custom_id = 0);

		protected:
			MessageHandlerID m_id;
			MessageHandlerGroupID m_group_id;
			std::vector<MessageType> m_msg_types;

		public:
			IMessageHandler(MessageHandlerGroupID group_id = 0, MessageHandlerID custom_id = 0) :
				m_id(registerMessageHandler(this, group_id, custom_id)),
				m_group_id(group_id)
			{
			}

			virtual ~IMessageHandler()
			{
				removeMessageHandler(this);
			}

			MessageHandlerID getID() const
			{
				return m_id;
			}

			MessageHandlerGroupID getGroupID() const
			{
				return m_group_id;
			}

			void registerMessageType(MessageType msg_type)
			{
				for (MessageType t : m_msg_types)
					if (t == msg_type) return;

				m_msg_types.push_back(msg_type);
			}

			bool isMessageTypeSupported(MessageType msg_type) const
			{
				for (MessageType t : m_msg_types)
					if (t == msg_type) return true;

				return false;
			}

			std::shared_ptr<Message> getNextMessage() const
			{
				return getNextMessage(this);
			}

			template<class... Args>
			bool sendMessage(MessageType msg_type, MessageHandlerID handler_id, Args... args) const
			{
				return Message(msg_type, m_id, std::forward<Args>(args)...).send(handler_id);
			}

			template<class... Args>
			bool sendMessageToGroup(MessageType msg_type, MessageHandlerGroupID group_id, Args... args) const
			{
				return Message(msg_type, m_id, std::forward<Args>(args)...).sendToGroup(group_id);
			}
		};
	};
};

#endif // GG_MESSAGE_HPP_INCLUDED
