/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_FASTMUTEX_HPP_INCLUDED
#define GG_FASTMUTEX_HPP_INCLUDED

#include <atomic>
#include <mutex>
#include <thread>

namespace gg
{
	class FastMutex
	{
	public:
		FastMutex()
		{
			m_lock.clear();
		}

		FastMutex(const FastMutex&) = delete;

		void lock()
		{
			while (m_lock.test_and_set())
			{
				std::this_thread::sleep_for(
					std::chrono::milliseconds(10));
			}
		}

		void unlock()
		{
			m_lock.clear();
		}

	private:
		std::atomic_flag m_lock;
	};
};

#endif // GG_FASTMUTEX_HPP_INCLUDED
