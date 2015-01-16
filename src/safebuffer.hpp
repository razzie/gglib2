/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_SAFEBUFFER_HPP_INCLUDED
#define GG_SAFEBUFFER_HPP_INCLUDED

#include "gg/serializer.hpp"

namespace gg
{
	class SafeBuffer : public IBuffer
	{
	public:
		SafeBuffer(IBuffer& buf) :
			m_buf(buf), m_pos(0)
		{
		}

		virtual ~SafeBuffer()
		{
		}

		virtual size_t available() const
		{
			size_t a = m_buf.available();
			if (m_pos >= a)
				return 0;
			else
				return (a - m_pos);
		}

		virtual void skip(size_t len)
		{
			m_pos += len;
		}

		virtual void clear()
		{
			m_pos = m_buf.available();
		}

		virtual void write(char c)
		{
			m_buf.write(c);
		}

		virtual void write(const char* ptr, size_t len)
		{
			m_buf.write(ptr, len);
		}

		virtual size_t peek(char* ptr, size_t len, size_t start_pos = 0) const
		{
			return m_buf.peek(ptr, len, start_pos + m_pos);
		}

		virtual size_t read(char* ptr, size_t len)
		{
			size_t n = m_buf.peek(ptr, len, m_pos);
			m_pos += n;
			return n;
		}

		void finalize()
		{
			m_buf.skip(m_pos);
			m_pos = 0;
		}

	private:
		IBuffer& m_buf;
		size_t m_pos;
	};
};

#endif // GG_SAFEBUFFER_HPP_INCLUDED
