/**
 * Copyright (c) 2014-2015 G�bor G�rzs�ny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "ieee754.hpp"
#include "archive_impl.hpp"


/*static bool isBigEndian()
{
	union
	{
		uint32_t i;
		char c[4];
	} chk = { 0x01020304 };

	return (chk.c[0] == 1);
}*/


gg::Archive::Archive(IArchive::Mode mode) :
	m_mode(mode),
	m_data_len(0),
	m_data_pos(0)
{
}

gg::Archive::~Archive()
{
}

gg::IArchive::Mode gg::Archive::getMode() const
{
	return m_mode;
}

const char* gg::Archive::getData() const
{
	return m_data;
}

size_t gg::Archive::getSize() const
{
	return m_data_len;
}

char* gg::Archive::getDataPtr()
{
	return m_data;
}

void gg::Archive::setSize(size_t size)
{
	m_data_len = size;
}

gg::Archive& gg::Archive::operator&(int8_t& i)
{
	if (m_mode == Mode::DESERIALIZE)
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

gg::Archive& gg::Archive::operator&(int16_t& i)
{
	if (m_mode == Mode::DESERIALIZE)
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

gg::Archive& gg::Archive::operator&(int32_t& i)
{
	if (m_mode == Mode::DESERIALIZE)
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

gg::Archive& gg::Archive::operator&(int64_t& i)
{
	if (m_mode == Mode::DESERIALIZE)
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

gg::Archive& gg::Archive::operator&(uint8_t& u)
{
	if (m_mode == Mode::DESERIALIZE)
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

gg::Archive& gg::Archive::operator&(uint16_t& u)
{
	if (m_mode == Mode::DESERIALIZE)
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

gg::Archive& gg::Archive::operator&(uint32_t& u)
{
	if (m_mode == Mode::DESERIALIZE)
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

gg::Archive& gg::Archive::operator&(uint64_t& u)
{
	if (m_mode == Mode::DESERIALIZE)
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

gg::Archive& gg::Archive::operator&(float& f)
{
	if (m_mode == Mode::DESERIALIZE)
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

gg::Archive& gg::Archive::operator&(double& f)
{
	if (m_mode == Mode::DESERIALIZE)
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

gg::Archive& gg::Archive::operator&(std::string& str)
{
	if (m_mode == Mode::DESERIALIZE)
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
		str.resize(len + 1);
		if (read(&str[0], len) < len)
			throw SerializationError();
	}

	return *this;
}

gg::Archive& gg::Archive::operator&(ISerializable& serializable)
{
	serializable.serialize(*this);
	return *this;
}

size_t gg::Archive::write(const char* ptr, size_t len)
{
	if (m_mode != Mode::DESERIALIZE)
		throw SerializationError();

	if (BUF_SIZE - m_data_len < len)
		len = BUF_SIZE - m_data_len;

	std::memcpy(&m_data[m_data_len], ptr, len);
	m_data_len += len;
	return len;
}

size_t gg::Archive::read(char* ptr, size_t len)
{
	if (m_mode != Mode::SERIALIZE)
		throw SerializationError();

	if (m_data_len - m_data_pos < len)
		len = m_data_len - m_data_pos;

	std::memcpy(ptr, &m_data[m_data_pos], len);
	m_data_pos += len;
	return len;
}


const char* gg::SerializationError::what() const
{
	return "Serialization error";
}
