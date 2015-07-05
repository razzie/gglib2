/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <typeinfo>
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
		virtual std::shared_ptr<IBuffer> getBuffer() const = 0;
		virtual void setBuffer(std::shared_ptr<IBuffer>) = 0;
		virtual operator bool() const = 0;
		virtual void setFailBit(bool = true) = 0;

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


	namespace __SerializeStorage
	{
		template<size_t N>
		bool serialize(ISerializer& serializer, IStorage& storage)
		{
			return true;
		}

		template<size_t N, class Type0, class... Types>
		bool serialize(ISerializer& serializer, IStorage& storage)
		{
			serializer & storage.get<Type0>(N);
			if (serializer)
				return serialize<N + 1, Types...>(serializer, storage);
			else
				return false;
		}
	}

	template<class... Types>
	bool serialize(ISerializer& serializer, Storage<Types...>& storage)
	{
		return __SerializeStorage::serialize<0, Types...>(serializer, storage);
	}



};
