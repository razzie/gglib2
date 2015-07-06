/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "network_impl.hpp"

static void test()
{
	gg::Storage<int, int, float> s = { 1, 2, 3.0f };

	gg::IPacket* p = nullptr;
	gg::serialize(*p, s);
}
