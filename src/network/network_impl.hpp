/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include "gg/network.hpp"

namespace gg
{
	class Packet : public IPacket
	{
	public:
		Packet(Mode);
		virtual ~Packet();
		virtual Mode getMode() const;

		virtual Packet& operator& (int8_t&);
		virtual Packet& operator& (int16_t&);
		virtual Packet& operator& (int32_t&);
		virtual Packet& operator& (int64_t&);
		virtual Packet& operator& (uint8_t&);
		virtual Packet& operator& (uint16_t&);
		virtual Packet& operator& (uint32_t&);
		virtual Packet& operator& (uint64_t&);
		virtual Packet& operator& (float&);
		virtual Packet& operator& (double&);
		virtual Packet& operator& (std::string&);
		virtual Packet& operator& (ISerializable&);

	protected:
		static const size_t BUF_SIZE = 1024;

		Mode m_mode;
		char m_data[BUF_SIZE];
		size_t m_data_len;
		size_t m_data_pos;

		size_t write(const char* ptr, size_t len);
		size_t read(char* ptr, size_t len);
	};


};
