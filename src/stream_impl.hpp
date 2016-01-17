/**
 * Copyright (c) 2014-2016 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <cstdint>
#include <string>
#include "gg/serializable.hpp"

namespace gg
{
	class Stream : public virtual IStream
	{
	public:
		static const size_t BUF_SIZE = 8192;

		Stream(Mode);
		virtual ~Stream();
		virtual Mode getMode() const;
		virtual Stream& operator& (int8_t&);
		virtual Stream& operator& (int16_t&);
		virtual Stream& operator& (int32_t&);
		virtual Stream& operator& (int64_t&);
		virtual Stream& operator& (uint8_t&);
		virtual Stream& operator& (uint16_t&);
		virtual Stream& operator& (uint32_t&);
		virtual Stream& operator& (uint64_t&);
		virtual Stream& operator& (float&);
		virtual Stream& operator& (double&);
		virtual Stream& operator& (std::string&);
		virtual Stream& operator& (ISerializable&);

		// the following functions should be implemented by the end user
		virtual size_t write(const char* ptr, size_t len) = 0;
		virtual size_t read(char* ptr, size_t len) = 0;

	private:
		Mode m_mode;
	};

	class SerializationError : public ISerializationError
	{
	public:
		SerializationError() = default;
		virtual ~SerializationError() = default;
		virtual const char* what() const;
	};
};
