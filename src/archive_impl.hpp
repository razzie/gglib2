/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
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
	class Archive : public virtual IArchive
	{
	public:
		static const size_t BUF_SIZE = 8192;

		Archive(Mode);
		virtual ~Archive();
		virtual Mode getMode() const;
		virtual Archive& operator& (int8_t&);
		virtual Archive& operator& (int16_t&);
		virtual Archive& operator& (int32_t&);
		virtual Archive& operator& (int64_t&);
		virtual Archive& operator& (uint8_t&);
		virtual Archive& operator& (uint16_t&);
		virtual Archive& operator& (uint32_t&);
		virtual Archive& operator& (uint64_t&);
		virtual Archive& operator& (float&);
		virtual Archive& operator& (double&);
		virtual Archive& operator& (std::string&);
		virtual Archive& operator& (ISerializable&);

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
