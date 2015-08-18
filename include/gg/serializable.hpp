/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

namespace gg
{
	class ISerializable;

	class IArchive
	{
	public:
		enum Mode
		{
			SERIALIZE,
			DESERIALIZE
		};

		virtual ~IArchive() = default;
		virtual Mode getMode() const = 0;
		virtual const char* getData() const = 0;
		virtual size_t getSize() const = 0;

		virtual IArchive& operator& (int8_t&) = 0;
		virtual IArchive& operator& (int16_t&) = 0;
		virtual IArchive& operator& (int32_t&) = 0;
		virtual IArchive& operator& (int64_t&) = 0;
		virtual IArchive& operator& (uint8_t&) = 0;
		virtual IArchive& operator& (uint16_t&) = 0;
		virtual IArchive& operator& (uint32_t&) = 0;
		virtual IArchive& operator& (uint64_t&) = 0;
		virtual IArchive& operator& (float&) = 0;
		virtual IArchive& operator& (double&) = 0;
		virtual IArchive& operator& (std::string&) = 0;
		virtual IArchive& operator& (ISerializable&) = 0;

		virtual size_t write(const char* ptr, size_t len) = 0;
		virtual size_t read(char* ptr, size_t len) = 0;

		template<class T>
		IArchive& operator& (T& t)
		{
			serialize(*this, t);
			return *this;
		}
	};

	class ISerializationError : public std::exception
	{
	public:
		virtual ~ISerializationError() = default;
		virtual const char* what() const = 0;
	};

	class ISerializable
	{
	public:
		virtual ~ISerializable() = default;
		virtual void serialize(IArchive&) = 0;
	};
};
