/**
 * Copyright (c) 2014-2016 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "ieee754.hpp"
#include "stream_impl.hpp"


/*static bool isBigEndian()
{
	union
	{
		uint32_t i;
		char c[4];
	} chk = { 0x01020304 };

	return (chk.c[0] == 1);
}*/


gg::Stream::Stream(IStream::Mode mode) :
	m_mode(mode)
{
}

gg::Stream::~Stream()
{
}

gg::IStream::Mode gg::Stream::getMode() const
{
	return m_mode;
}

gg::Stream& gg::Stream::operator&(int8_t& i)
{
	if (m_mode == Mode::SERIALIZE)
	{
		if (write(reinterpret_cast<const char*>(&i), sizeof(int8_t)) < sizeof(int8_t))
			throw SerializationError();
	}
	else
	{
		int8_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(int8_t)) < sizeof(int8_t))
			throw SerializationError();
		i = tmp;
	}

	return *this;
}

gg::Stream& gg::Stream::operator&(int16_t& i)
{
	if (m_mode == Mode::SERIALIZE)
	{
		if (write(reinterpret_cast<const char*>(&i), sizeof(int16_t)) < sizeof(int16_t))
			throw SerializationError();
	}
	else
	{
		int16_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(int16_t)) < sizeof(int16_t))
			throw SerializationError();
		i = tmp;
	}

	return *this;
}

gg::Stream& gg::Stream::operator&(int32_t& i)
{
	if (m_mode == Mode::SERIALIZE)
	{
		if (write(reinterpret_cast<const char*>(&i), sizeof(int32_t)) < sizeof(int32_t))
			throw SerializationError();
	}
	else
	{
		int32_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(int32_t)) < sizeof(int32_t))
			throw SerializationError();
		i = tmp;
	}

	return *this;
}

gg::Stream& gg::Stream::operator&(int64_t& i)
{
	if (m_mode == Mode::SERIALIZE)
	{
		if (write(reinterpret_cast<const char*>(&i), sizeof(int64_t)) < sizeof(int64_t))
			throw SerializationError();
	}
	else
	{
		int64_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(int64_t)) < sizeof(int64_t))
			throw SerializationError();
		i = tmp;
	}

	return *this;
}

gg::Stream& gg::Stream::operator&(uint8_t& u)
{
	if (m_mode == Mode::SERIALIZE)
	{
		if (write(reinterpret_cast<const char*>(&u), sizeof(uint8_t)) < sizeof(uint8_t))
			throw SerializationError();
	}
	else
	{
		int8_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(uint8_t)) < sizeof(uint8_t))
			throw SerializationError();
		u = tmp;
	}

	return *this;
}

gg::Stream& gg::Stream::operator&(uint16_t& u)
{
	if (m_mode == Mode::SERIALIZE)
	{
		if (write(reinterpret_cast<const char*>(&u), sizeof(uint16_t)) < sizeof(uint16_t))
			throw SerializationError();
	}
	else
	{
		int16_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(uint16_t)) < sizeof(uint16_t))
			throw SerializationError();
		u = tmp;
	}

	return *this;
}

gg::Stream& gg::Stream::operator&(uint32_t& u)
{
	if (m_mode == Mode::SERIALIZE)
	{
		if (write(reinterpret_cast<const char*>(&u), sizeof(uint32_t)) < sizeof(uint32_t))
			throw SerializationError();
	}
	else
	{
		int32_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(uint32_t)) < sizeof(uint32_t))
			throw SerializationError();
		u = tmp;
	}

	return *this;
}

gg::Stream& gg::Stream::operator&(uint64_t& u)
{
	if (m_mode == Mode::SERIALIZE)
	{
		if (write(reinterpret_cast<const char*>(&u), sizeof(uint64_t)) < sizeof(uint64_t))
			throw SerializationError();
	}
	else
	{
		int64_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(uint64_t)) < sizeof(uint64_t))
			throw SerializationError();
		u = tmp;
	}

	return *this;
}

gg::Stream& gg::Stream::operator&(float& f)
{
	if (m_mode == Mode::SERIALIZE)
	{
		uint32_t tmp = static_cast<uint32_t>(pack754_32(f));
		*this & tmp;
	}
	else
	{
		uint32_t tmp;
		*this & tmp;
		f = static_cast<float>(unpack754_32(tmp));
	}

	return *this;
}

gg::Stream& gg::Stream::operator&(double& f)
{
	if (m_mode == Mode::SERIALIZE)
	{
		uint64_t tmp = pack754_64(f);
		*this & tmp;
	}
	else
	{
		uint64_t tmp;
		*this & tmp;
		f = static_cast<float>(unpack754_64(tmp));
	}

	return *this;
}

gg::Stream& gg::Stream::operator&(std::string& str)
{
	if (m_mode == Mode::SERIALIZE)
	{
		uint16_t len = static_cast<uint16_t>(str.length());
		*this & len;
		if (write(str.c_str(), len) < len)
			throw SerializationError();
	}
	else
	{
		uint16_t len;
		*this & len;
		str.resize(len);
		if (read(&str[0], len) < len)
			throw SerializationError();
	}

	return *this;
}

gg::Stream& gg::Stream::operator&(ISerializable& serializable)
{
	serializable.serialize(*this);
	return *this;
}


const char* gg::SerializationError::what() const
{
	return "Serialization error";
}
