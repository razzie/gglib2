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
#include "gg/any.hpp"
#include "gg/storage.hpp"

namespace gg
{
	class ISerializable;

	class IBuffer
	{
	public:
		virtual ~IBuffer() = default;
		virtual size_t available() const = 0;
		virtual void skip(size_t len) = 0;
		virtual void clear() = 0;
		virtual void write(const char* ptr, size_t len) = 0;
		virtual size_t peek(char* ptr, size_t len, size_t start_pos = 0) const = 0;
		virtual size_t read(char* ptr, size_t len) = 0;
	};

	class ISerializer
	{
	public:
		enum Mode
		{
			SERIALIZE,
			DESERIALIZE
		};

		virtual ~ISerializer() = default;
		virtual Mode getMode() const = 0;
		virtual void setMode(Mode) = 0;
		virtual void setFailBit(bool = true) = 0;
		virtual operator bool() const = 0;
		virtual ISerializer& operator& (int8_t&) = 0;
		virtual ISerializer& operator& (int16_t&) = 0;
		virtual ISerializer& operator& (int32_t&) = 0;
		virtual ISerializer& operator& (int64_t&) = 0;
		virtual ISerializer& operator& (uint8_t&) = 0;
		virtual ISerializer& operator& (uint16_t&) = 0;
		virtual ISerializer& operator& (uint32_t&) = 0;
		virtual ISerializer& operator& (uint64_t&) = 0;
		virtual ISerializer& operator& (float&) = 0;
		virtual ISerializer& operator& (double&) = 0;
		virtual ISerializer& operator& (std::string&) = 0;
		virtual ISerializer& operator& (Any&) = 0;
		virtual ISerializer& operator& (Any::Array&) = 0;
		virtual ISerializer& operator& (IStorage&) = 0;
		virtual ISerializer& operator& (ISerializable&) = 0;

		template<class T>
		ISerializer& operator& (T& t)
		{
			if (!serialize(*this, t))
				setFailBit();

			return *this;
		}
	};

	class ISerializable
	{
	public:
		virtual ~ISerializable() = default;
		virtual bool serialize(ISerializer&) = 0;
	};

	// generic serializer for STL containers
	template<class Container, class T>
	bool serialize(ISerializer& serializer, Container<T>& container)
	{
		for (auto it = container.begin(), end = container.end(); it != end; ++it)
		{
			if ( !(serializer & *it) )
				return false;
		}
		return true;
	}



};
