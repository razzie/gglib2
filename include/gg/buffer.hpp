/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_BUFFER_HPP_INCLUDED
#define GG_BUFFER_HPP_INCLUDED

#include <deque>
#include <iomanip>
#include <mutex>

namespace gg
{
	class Buffer
	{
	public:
		Buffer()
		{
		}

		virtual ~Buffer()
		{
		}

		virtual size_t available() const
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			return m_data.size();
		}

		virtual void skip(size_t len)
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			auto it_begin = m_data.begin(), it_end = std::next(it_begin, len);
			m_data.erase(it_begin, it_end);
		}

		virtual void clear()
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			m_data.clear();
		}

		virtual void write(char c)
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			m_data.push_back(c);
		}

		virtual void write(const char* ptr, size_t len)
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			for (size_t i = 0; i < len; ++i)
				m_data.push_back(ptr[i]);
		}

		virtual size_t peek(char* ptr, size_t len, size_t start_pos = 0)
		{
			std::lock_guard<std::mutex> guard(m_mutex);

			if (start_pos > m_data.size()) return 0;

			auto it = std::next(m_data.begin(), start_pos), end = m_data.end();
			size_t i = 0;
			for (; it != end && i < len; ++it, ++i)
			{
				ptr[i] = *it;
			}

			return i;
		}

		virtual size_t read(char* ptr, size_t len)
		{
			std::lock_guard<std::mutex> guard(m_mutex);

			auto it = m_data.begin(), end = m_data.end();
			size_t i = 0;
			for (; it != end && i < len; ++it, ++i)
			{
				ptr[i] = *it;
			}

			m_data.erase(m_data.begin(), it);

			return i;

		}

		virtual void copyFrom(const Buffer& buf)
		{
			std::lock_guard<std::mutex> guard1(m_mutex);
			std::lock_guard<std::mutex> guard2(buf.m_mutex);
			m_data.insert(m_data.end(), buf.m_data.begin(), buf.m_data.end());
		}

		virtual void moveFrom(Buffer& buf)
		{
			std::lock_guard<std::mutex> guard1(m_mutex);
			std::lock_guard<std::mutex> guard2(buf.m_mutex);
			m_data.insert(m_data.end(),
				std::make_move_iterator(buf.m_data.begin()),
				std::make_move_iterator(buf.m_data.end()));
		}

		friend std::ostream& operator<<(std::ostream& o, const Buffer& buf)
		{
			std::lock_guard<std::mutex> guard(buf.m_mutex);
			std::ios state(NULL);
			int i = 0;

			state.copyfmt(o);
			o << std::setfill('0') << std::hex;
			for (char c : buf.m_data)
				o << std::setw(2) << (int)c
				<< ((++i % 8 == 0) ? "\n" : " ");
			o.copyfmt(state);

			return o;
		}

	protected:
		mutable std::mutex m_mutex;
		std::deque<char> m_data;
	};
};

#endif // GG_BUFFER_HPP_INCLUDED
