/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <cassert>
#include <cstring>
#include <stdexcept>
#include "network_impl.hpp"
#include "ieee754.hpp"

#define SERIALIZATION_ERROR "Serialization error"

gg::Packet::Packet(gg::IPacket::Mode mode) :
	m_mode(mode),
	m_data_len(0),
	m_data_pos(0)
{
}

gg::Packet::~Packet()
{
}

gg::IPacket::Mode gg::Packet::getMode() const
{
	return m_mode;
}

gg::Packet& gg::Packet::operator&(int8_t& i)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&i), sizeof(int8_t)) < sizeof(int8_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
	}
	else
	{
		int8_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(int8_t)) < sizeof(int8_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
		i = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(int16_t& i)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&i), sizeof(int16_t)) < sizeof(int16_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
	}
	else
	{
		int16_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(int16_t)) < sizeof(int16_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
		i = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(int32_t& i)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&i), sizeof(int32_t)) < sizeof(int32_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
	}
	else
	{
		int32_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(int32_t)) < sizeof(int32_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
		i = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(int64_t& i)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&i), sizeof(int64_t)) < sizeof(int64_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
	}
	else
	{
		int64_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(int64_t)) < sizeof(int64_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
		i = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(uint8_t& u)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&u), sizeof(uint8_t)) < sizeof(uint8_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
	}
	else
	{
		int8_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(uint8_t)) < sizeof(uint8_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
		u = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(uint16_t& u)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&u), sizeof(uint16_t)) < sizeof(uint16_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
	}
	else
	{
		int16_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(uint16_t)) < sizeof(uint16_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
		u = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(uint32_t& u)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&u), sizeof(uint32_t)) < sizeof(uint32_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
	}
	else
	{
		int32_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(uint32_t)) < sizeof(uint32_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
		u = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(uint64_t& u)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&u), sizeof(uint64_t)) < sizeof(uint64_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
	}
	else
	{
		int64_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(uint64_t)) < sizeof(uint64_t))
			throw std::runtime_error(SERIALIZATION_ERROR);
		u = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(float& f)
{
	if (m_mode == Mode::WRITE)
	{
		uint32_t tmp = pack754_32(f);
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

gg::Packet& gg::Packet::operator&(double& f)
{
	if (m_mode == Mode::WRITE)
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

gg::Packet& gg::Packet::operator&(std::string& str)
{
	if (m_mode == Mode::WRITE)
	{
		uint16_t len = static_cast<uint16_t>(str.length());
		*this & len;
		if (write(str.c_str(), len) < len)
			throw std::runtime_error(SERIALIZATION_ERROR);
	}
	else
	{
		uint16_t len;
		*this & len;
		str.resize(len + 1);
		if (read(&str[0], len) < len)
			throw std::runtime_error(SERIALIZATION_ERROR);
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(ISerializable& serializable)
{
	serializable.serialize(*this);
	return *this;
}

size_t gg::Packet::write(const char* ptr, size_t len)
{
	assert(m_mode == Mode::WRITE);

	if (BUF_SIZE - m_data_len < len)
		len = BUF_SIZE - m_data_len;

	std::memcpy(&m_data[m_data_len], ptr, len);
	m_data_len += len;
	return len;
}

size_t gg::Packet::read(char* ptr, size_t len)
{
	assert(m_mode == Mode::READ);

	if (m_data_len - m_data_pos < len)
		len = m_data_len - m_data_pos;

	std::memcpy(ptr, &m_data[m_data_pos], len);
	m_data_pos += len;
	return len;
}
