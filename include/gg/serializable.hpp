/**
 * Copyright (c) 2014-2015 G�bor G�rzs�ny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

namespace gg
{
	class IPacket;

	class ISerializable
	{
	public:
		virtual ~ISerializable() = default;
		virtual void serialize(IPacket&) = 0;
	};
};
