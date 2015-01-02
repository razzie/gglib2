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
#include "gg/network.hpp"

namespace gg
{
	namespace msg
	{
		typedef uint16_t MessageType; // 0 is invalid
		typedef uint16_t MessageHandlerID; // 0 is invalid
		typedef uint16_t MessageHandlerGroupID; // 0 is invalid

		class Message
		{
		protected:
			MessageType m_msg_type;
			MessageHandlerID m_sender; // 0 means not specified
			VarArray m_args;

		public:
			static const size_t MAX_ARG_NUM = 16; // a message can have maximum 16 args

			Message(MessageType msg_type, MessageHandlerID sender = 0) :
				m_msg_type(msg_type),
				m_sender(sender)
			{
			}

			template<class... Args>
			Message(MessageType msg_type, MessageHandlerID sender, Args... args) :
				m_msg_type(msg_type),
				m_sender(sender)
			{
				m_args.insert({ std::forward<Args>(args)... });
			}

			Message(Message&& msg) :
				m_msg_type(msg.m_msg_type),
				m_sender(msg.m_sender),
				m_args(std::move(msg.m_args))
			{
			}

			virtual ~Message()
			{
			}

			template<class... Args>
			bool addArgs(Args... args)
			{
				m_args.insert({ args... });
				return true;
			}
			
			template<class T>
			T& getArg(size_t arg_num)
			{
				return m_args[arg_num].get<T>();
			}

			template<class T>
			const T& getArg(size_t arg_num) const
			{
				return m_args[arg_num].get<T>();
			}

			bool send(MessageHandlerID);
			bool sendToGroup(MessageHandlerGroupID);
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
			struct MessageTypeData
			{
				MessageType msg_type;
				size_t arg_count;
				net::TypeIndex arg_types[Message::MAX_ARG_NUM];
			};

			MessageHandlerID m_id;
			MessageHandlerGroupID m_group_id;
			std::vector<MessageTypeData> m_msg_types;

			template<class... Args>
			void addMessageType(MessageType msg_type)
			{
				struct
				{
					size_t arg_count = 0;
					net::TypeIndex arg_types[Message::MAX_ARG_NUM];

					int operator()(const std::type_info& t)
					{
						arg_types[arg_count] = net::getTypeIndex(t);
						++arg_count;
						return 0;
					}
				} addType;

				struct
				{
					void operator()(...) {}
				} expand;

				expand(addType(typeid(Args))...);

				m_msg_types.push_back({ msg_type, addType.arg_count, addType.arg_types });
			}

			static std::shared_ptr<Message> getNextMessage(const IMessageHandler*);
			static void removeMessageHandler(const IMessageHandler*);
			static MessageHandlerID registerMessageHandler(
				const IMessageHandler*,
				MessageHandlerGroupID group_id = 0,
				MessageHandlerID custom_id = 0);

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

			bool checkMessage(const Message& msg) const
			{
				// should compare argument types here
				return true;
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
