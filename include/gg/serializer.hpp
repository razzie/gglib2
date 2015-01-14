/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_SERIALIZER_HPP_INCLUDED
#define GG_SERIALIZER_HPP_INCLUDED

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <typeinfo>
#include <type_traits>
#include "gg/buffer.hpp"
#include "gg/var.hpp"
#include "gg/storage.hpp"

namespace gg
{
	class ISerializable
	{
	public:
		virtual ~ISerializable() {}
		virtual bool serialize(Buffer&) const = 0;
		virtual bool deserialize(Buffer&) = 0;
	};

	typedef std::function<bool(const void*, Buffer&)> SerializerFunction;
	typedef std::function<bool(void*, Buffer&)> DeserializerFunction;
	bool addSerializableType(const std::type_info&, size_t, SerializerFunction, DeserializerFunction);

	// overloaded serializer functions
	bool serialize(const ISerializable&, Buffer&);
	bool serialize(const Var&, Buffer&);
	bool serialize(const VarArray&, Buffer&);
	bool serialize(const IStorage&, Buffer&);
	bool serialize(const std::type_info&, const void*, Buffer&);

	template<class T>
	bool serialize(const T& o, Buffer& buf)
	{
		return serialize(typeid(T), reinterpret_cast<const void*>(&o), buf);
	}

	// overloaded deserializer functions
	bool deserialize(ISerializable&, Buffer&);
	bool deserialize(Var&, Buffer&); // var.construct<T>() should be called previously
	bool deserialize(VarArray&, Buffer&); // all Var entries should be (default) constructed previously
	bool deserialize(IStorage&, Buffer&);
	bool deserialize(const std::type_info&, void*, Buffer&);

	template<class T>
	bool deserialize(T& o, Buffer& buf)
	{
		return deserialize(typeid(T), reinterpret_cast<void*>(&o), buf);
	}

	template<class T>
	bool addSerializableClass(
		typename std::enable_if<std::is_base_of<ISerializable, T>::value>::type* = 0)
	{
		SerializerFunction save_func =
			[](const void* ptr, Buffer& buf) -> bool
		{
			return reinterpret_cast<const ISerializable*>(ptr)->serialize(buf);
		};

		DeserializerFunction init_func =
			[](void* ptr, Buffer& buf) -> bool
		{
			return reinterpret_cast<ISerializable*>(ptr)->deserialize(buf);
		};

		return addSerializableType(typeid(T), sizeof(T), save_func, init_func);
	}

	template<class T>
	bool addSerializableType(
		std::function<bool(const T&, Buffer&)> orig_save_func,
		std::function<bool(T&, Buffer&)> orig_init_func)
	{
		SerializerFunction save_func =
			[orig_save_func](const void* ptr, Buffer& buf) -> bool
		{
			const T& o = *reinterpret_cast<const T*>(ptr);
			return orig_save_func(o, buf);
		};

		DeserializerFunction init_func =
			[orig_init_func](void* ptr, Buffer& buf) -> bool
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
			[](const void* ptr, Buffer& buf) -> bool
		{
			buf.write(reinterpret_cast<const char*>(ptr), sizeof(T));
			return true;
		};

		DeserializerFunction init_func =
			[](void* ptr, Buffer& buf) -> bool
		{
			if (buf.read(reinterpret_cast<char*>(ptr), sizeof(T)) < sizeof(T))
				return false;
			else
				return true;
		};

		return addSerializableType(typeid(T), sizeof(T), save_func, init_func);
	}
};

#endif // GG_SERIALIZER_HPP_INCLUDED
