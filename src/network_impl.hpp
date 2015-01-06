/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_NETWORK_IMPL_HPP_INCLUDED
#define GG_NETWORK_IMPL_HPP_INCLUDED

#include "gg/network.hpp"

namespace gg
{
	bool serialize(const Var&, std::shared_ptr<IBuffer>);
	bool deserialize(Var&, std::shared_ptr<IBuffer>);
}

#endif // GG_NETWORK_IMPL_HPP_INCLUDED
