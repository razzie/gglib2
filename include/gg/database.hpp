/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#if defined GGDATABASE_BUILD
#	define GG_API __declspec(dllexport)
#else
#	define GG_API __declspec(dllimport)
#endif

#include <cstdint>
#include <string>

namespace gg
{
	class IDatabase
	{
	public:
		virtual ~IDatabase() = default;
	};

	class IDatabaseManager
	{
	public:
		virtual ~IDatabaseManager() = default;
	};

	extern GG_API IDatabaseManager& db;
};
