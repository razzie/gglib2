/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <memory>
#include "message_impl.hpp"




void gg::msg::Message::registerMessageType(gg::msg::MessageType msg_type, std::vector<const std::type_info*>&& types)
{

}

bool gg::msg::Message::isValid(gg::msg::MessageType msg_type, const gg::VarArray& args)
{
	return false;
}

bool gg::msg::Message::sendMessage(gg::msg::MessageType msg_type, gg::msg::MessageHandlerID handler_id, gg::VarArray&& args)
{
	return false;
}

bool gg::msg::Message::sendMessageToGroup(gg::msg::MessageType msg_type, gg::msg::MessageHandlerGroupID group_id, gg::VarArray&& args)
{
	return false;
}


std::shared_ptr<gg::msg::Message> gg::msg::IMessageHandler::getNextMessage(const gg::msg::IMessageHandler* handler)
{
	return {};
}

void gg::msg::IMessageHandler::removeMessageHandler(const gg::msg::IMessageHandler* handler)
{

}

gg::msg::MessageHandlerID gg::msg::IMessageHandler::registerMessageHandler(
	const gg::msg::IMessageHandler* handler, gg::msg::MessageHandlerGroupID group_id, gg::msg::MessageHandlerID custom_id)
{
	return 0;
}
