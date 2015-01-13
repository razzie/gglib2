/**
 * Copyright (c) 2014-2015 G�bor G�rzs�ny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

void initMessages();
void initSerializer();
void initNtHookEngine();

class Initializer
{
public:
	Initializer()
	{
		initMessages();
		initSerializer();
		initNtHookEngine();
	}
};

static Initializer init;
