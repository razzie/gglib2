/**
 * Copyright (c) 2014-2016 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <map>
#include <mutex>
#include <thread>
#include <string>
#include "gg/timer.hpp"
#include "gg/logger.hpp"

namespace gg
{
	class Logger : public ILogger, public std::streambuf
	{
	public:
		Logger();
		virtual ~Logger();
		virtual void setTimestamp(Timestamp);
		virtual void redirect(std::ostream&);
		virtual void redirect(std::shared_ptr<std::ostream>);
		virtual void redirect(const std::string& file_name);
		void write(const std::string&) const;

	protected:
		// inherited from std::streambuf
		int overflow(int c = std::char_traits<char>::eof());
		int sync();

	private:
		mutable std::mutex m_mutex;
		std::map<std::thread::id, std::string> m_buffer;
		std::shared_ptr<std::ostream> m_output;
		Timestamp m_timestamp;
		Timer m_timer;
	};
};
