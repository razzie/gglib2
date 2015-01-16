/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

/**
 * The purpose of 'gg::ILogger' class (and 'gg::log' instance) is to provide
 * a thread-safe way to log messages to the standard output or an optional
 * other output, like a file.
 */

#ifndef GG_LOGGER_HPP_INCLUDED
#define GG_LOGGER_HPP_INCLUDED

#include <iostream>
#include <memory>

namespace gg
{
	class ILogger : public virtual std::ostream
	{
	public:
		enum Timestamp
		{
			ELAPSED_TIME, // [min:sec:msec] - default
			SYSTEM_TIME, // [hour:min:sec] - in UTC
			NONE
		};

		virtual ~ILogger() {}
		virtual void setTimestamp(Timestamp) = 0;
		virtual void redirect(std::ostream& = std::cout) = 0;
		virtual void redirect(std::shared_ptr<std::ostream>) = 0;
		virtual void redirect(const std::string& file_name) = 0;

	protected:
		ILogger() : std::ostream(0) {}
	};

	extern ILogger& log;

	inline void hookStdCout()
	{
		std::cout.rdbuf(log.rdbuf());
	}
};

#endif // GG_LOGGER_HPP_INCLUDED
