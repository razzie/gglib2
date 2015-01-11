/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

void initMessages();
void initNetwork();
void initNtHookEngine();

class Initializer
{
public:
	Initializer()
	{
		initMessages();
		initNetwork();
		initNtHookEngine();
	}
};

static Initializer init;
