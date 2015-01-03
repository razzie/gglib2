/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "network_impl.hpp"

gg::net::TypeIndex gg::net::addSerializerFunctions(const std::type_info& type, gg::net::InitFunction init_func, gg::net::SaveFunction save_func)
{
	return 0;
}

gg::net::TypeIndex gg::net::getTypeIndex(const std::type_info& type)
{
	return 0;
}

const std::type_info& gg::net::getTypeInfo(gg::net::TypeIndex type)
{
	return typeid(void);
}

bool gg::net::serialize(const gg::Var& var, std::shared_ptr<gg::net::IBuffer> buffer)
{
	return false;
}

bool gg::net::deserialize(gg::Var& var, std::shared_ptr<gg::net::IBuffer> buffer)
{
	return false;
}
