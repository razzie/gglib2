/**
 * Copyright (c) 2014-2015 G�bor G�rzs�ny (www.gorzsony.com)
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
	private:
		std::atomic_flag m_lock;

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
				std::this_thread::yield();
			}
		}

		void unlock()
		{
			m_lock.clear();
		}
	};
};

#endif // GG_FASTMUTEX_HPP_INCLUDED
