/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#if defined GG_BUILD
#	define GG_API __declspec(dllexport)
#else
#	define GG_API __declspec(dllimport)
#endif

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <typeinfo>
#include <type_traits>
#include "gg/any.hpp"
#include "gg/storage.hpp"

namespace gg
{
	class IBuffer
	{
	public:
		virtual ~IBuffer() {}
		virtual size_t available() const = 0;
		virtual void skip(size_t len) = 0;
		virtual void clear() = 0;
		virtual void write(char c) = 0;
		virtual void write(const char* ptr, size_t len) = 0;
		virtual size_t peek(char* ptr, size_t len, size_t start_pos = 0) const = 0;
		virtual size_t read(char* ptr, size_t len) = 0;
		virtual void copyFrom(const IBuffer& buf)
		{
			size_t a = buf.available();
			char* tmp = new char[a];
			a = buf.peek(tmp, a);
			write(tmp, a);
			delete[] tmp;
		}
	};

	class ISerializable
	{
	public:
		virtual ~ISerializable() {}
		virtual bool serialize(IBuffer&) const = 0;
		virtual bool deserialize(IBuffer&) = 0;
	};

	typedef std::function<bool(const void*, IBuffer&)> SerializerFunction;
	typedef std::function<bool(void*, IBuffer&)> DeserializerFunction;
	bool GG_API addSerializableType(const std::type_info&, size_t, SerializerFunction, DeserializerFunction);

	// overloaded serializer functions
	bool GG_API serialize(const ISerializable&, IBuffer&);
	bool GG_API serialize(const Any&, IBuffer&);
	bool GG_API serialize(const Any::Array&, IBuffer&);
	bool GG_API serialize(const IStorage&, IBuffer&);
	bool GG_API serialize(const std::type_info&, const void*, IBuffer&);

	template<class T>
	bool serialize(const T& o, IBuffer& buf)
	{
		return serialize(typeid(T), reinterpret_cast<const void*>(&o), buf);
	}

	// overloaded deserializer functions
	bool GG_API deserialize(ISerializable&, IBuffer&);
	bool GG_API deserialize(Any&, IBuffer&); // Any.emplace<T>() should be called previously
	bool GG_API deserialize(Any::Array&, IBuffer&); // all Any entries should be (default) constructed previously
	bool GG_API deserialize(IStorage&, IBuffer&);
	bool GG_API deserialize(const std::type_info&, void*, IBuffer&);

	template<class T>
	bool deserialize(T& o, IBuffer& buf)
	{
		return deserialize(typeid(T), reinterpret_cast<void*>(&o), buf);
	}

	template<class T>
	bool addSerializableClass(
		typename std::enable_if<std::is_base_of<ISerializable, T>::value>::type* = 0)
	{
		SerializerFunction save_func =
			[](const void* ptr, IBuffer& buf) -> bool
		{
			return reinterpret_cast<const ISerializable*>(ptr)->serialize(buf);
		};

		DeserializerFunction init_func =
			[](void* ptr, IBuffer& buf) -> bool
		{
			return reinterpret_cast<ISerializable*>(ptr)->deserialize(buf);
		};

		return addSerializableType(typeid(T), sizeof(T), save_func, init_func);
	}

	template<class T>
	bool addSerializableType(
		std::function<bool(const T&, IBuffer&)> orig_save_func,
		std::function<bool(T&, IBuffer&)> orig_init_func)
	{
		SerializerFunction save_func =
			[orig_save_func](const void* ptr, IBuffer& buf) -> bool
		{
			const T& o = *reinterpret_cast<const T*>(ptr);
			return orig_save_func(o, buf);
		};

		DeserializerFunction init_func =
			[orig_init_func](void* ptr, IBuffer& buf) -> bool
		{
			T& o = *reinterpret_cast<T*>(ptr);
			return orig_init_func(o, buf);
		};

		return addSerializableType(typeid(T), sizeof(T), save_func, init_func);
	}

	template<class T>
	bool addSerializableTrivialType(
		typename std::enable_if<std::is_trivial<T>::value>::type* = 0)
	{
		SerializerFunction save_func =
			[](const void* ptr, IBuffer& buf) -> bool
		{
			buf.write(reinterpret_cast<const char*>(ptr), sizeof(T));
			return true;
		};

		DeserializerFunction init_func =
			[](void* ptr, IBuffer& buf) -> bool
		{
			if (buf.read(reinterpret_cast<char*>(ptr), sizeof(T)) < sizeof(T))
				return false;
			else
				return true;
		};

		return addSerializableType(typeid(T), sizeof(T), save_func, init_func);
	}
};
