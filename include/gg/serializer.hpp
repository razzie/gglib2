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
	typedef std::function<bool(const void*, Buffer&)> SerializerFunction;
	typedef std::function<bool(void*, Buffer&)> DeserializerFunction;
	bool addSerializableType(const std::type_info&, SerializerFunction, DeserializerFunction);

	// overloaded serializer functions
	bool serialize(const Var&, Buffer&);
	bool serialize(const IStorage&, Buffer&);
	bool serialize(const std::type_info&, const void*, Buffer&);

	// overloaded deserializer functions
	bool deserialize(Var&, Buffer&); // var.construct<T>() should be called before
	bool deserialize(IStorage&, Buffer&);
	bool deserialize(const std::type_info&, void*, Buffer&);

	class ISerializable
	{
	public:
		virtual bool serialize(Buffer&) const;
		virtual bool deserialize(Buffer&);
	};

	template<class T>
	uint16_t addSerializableClass(
		typename std::enable_if<std::is_base_of<ISerializable, T>::value>::type* = 0)
	{
		SerializerFunction save = [](const void* ptr, Buffer& buf) -> bool
		{
			return reinterpret_cast<const ISerializable*>(ptr)->serialize(buf);
		};

		DeserializerFunction init = [](void* ptr, Buffer& buf) -> bool
		{
			return reinterpret_cast<ISerializable*>(ptr)->deserialize(buf);
		};

		return addSerializerFunctions(typeid(T), save, init);
	}

	template<class T>
	uint16_t addSerializablePOD( // plain-old-data
		typename std::enable_if<std::is_pod<T>::value>::type* = 0)
	{
		SerializerFunction save = [](const void* ptr, Buffer& buf) -> bool
		{
			buf.write(reinterpret_cast<const char*>(ptr), sizeof(T));
			return true;
		};

		DeserializerFunction init = [](void* ptr, Buffer& buf) -> bool
		{
			if (buf.read(reinterpret_cast<char*>(ptr), sizeof(T)) < sizeof(T))
				return false;
			else
				return true;
		};

		return addSerializerFunctions(typeid(T), save, init);
	}
};

#endif // GG_SERIALIZER_HPP_INCLUDED
