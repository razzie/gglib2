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

#include <cstdint>
#include <random>

namespace gg
{
	typedef uint16_t ID;

	template<class NumberType = ID>
	class IDGenerator
	{
	public:
		IDGenerator() = default;
		IDGenerator(const IDGenerator&) = delete;
		~IDGenerator() = default;

		NumberType next()
		{
			NumberType val;
			uint8_t* p = reinterpret_cast<uint8_t*>(&val);
			for (size_t i = 0; i < sizeof(NumberType); ++i)
				p[i] = m_generators[i].next();
			return val;
		}

	private:
		class ByteGenerator
		{
		public:
			ByteGenerator() :
				m_current(0)
			{
				std::random_device rd;
				std::mt19937 mt(rd());
				std::uniform_int_distribution<uint16_t> dist(0, 255);

				do
				{
					const uint8_t a = (dist(mt) % 163) + 1;
					const uint8_t b = (dist(mt) % 131) + 1;
					const uint8_t c = (dist(mt) % 101) + 1;

					m_step = (a * 255 * 255) + (b * 255) + c;
					m_step &= ~0xc0000000;
					m_step %= 257;
				} while (m_step == 0);
			}

			uint8_t next()
			{
				m_current += m_step;
				m_current %= 257;
				return static_cast<uint8_t>(m_current);
			}

		private:
			uint32_t m_step;
			uint32_t m_current;
		};

		ByteGenerator m_generators[sizeof(NumberType)];
	};
};
