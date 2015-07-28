/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

/**
 * Credits: Mike McShaffry
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <random>
#include <stdexcept>

namespace gg
{
	typedef uint16_t ID;

	class IDGenerator
	{
	public:
		IDGenerator() :
			m_current(0)
		{
			std::random_device rd;
			std::mt19937 mt(rd());
			std::uniform_int_distribution<uint16_t> dist(0, MAX_VALUE);

			const uint8_t a = (dist(mt) % 163) + 1;
			const uint8_t b = (dist(mt) % 131) + 1;
			const uint8_t c = (dist(mt) % 101) + 1;

			m_step = (a * MAX_VALUE * MAX_VALUE) + (b * MAX_VALUE) + c;
			m_step &= ~0xc0000000;
			m_step %= PRIME;
		}

		ID next()
		{
			uint32_t next = m_current.fetch_add(m_step) + m_step;
			if (next > ~0xc0000000)
				m_current.fetch_sub(~0xc0000000);

			next %= PRIME;
			return static_cast<uint16_t>(next);
		}

	private:
		const uint16_t MAX_VALUE = ~0;
		const uint32_t PRIME = 65537;

		uint32_t m_step;
		std::atomic<uint32_t> m_current;
	};
};
