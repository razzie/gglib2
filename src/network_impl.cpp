/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "network_impl.hpp"


void initNetwork()
{
}


gg::TypeIndex gg::addSerializerFunctions(const std::type_info& type, gg::InitFunction init_func, gg::SaveFunction save_func)
{
	return 0;
}

gg::TypeIndex gg::getTypeIndex(const std::type_info& type)
{
	return 0;
}

const std::type_info& gg::getTypeInfo(gg::TypeIndex type)
{
	return typeid(void);
}

bool gg::serialize(const gg::Var& var, std::shared_ptr<gg::IBuffer> buffer)
{
	return false;
}

bool gg::deserialize(gg::Var& var, std::shared_ptr<gg::IBuffer> buffer)
{
	return false;
}
