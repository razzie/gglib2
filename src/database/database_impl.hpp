/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include "gg/database.hpp"

namespace gg
{
	class Database : public IDatabase
	{
	public:
		virtual ~Database() = default;
	};

	class DatabaseManager : public IDatabaseManager
	{
	public:
		virtual ~DatabaseManager() = default;
	};
};
