/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

/**
 * HOW TO USE:
 * -----------
 *
 * You can use 'gg::net::addClass<Foo>()' or 'gg::net::addPOD<int>()' to register
 * serializable types (where Foo inherits 'init' and 'save' methods from
 * 'ISerializable'). The only purpose of this is to be able to send messages
 * (see 'gg/message.hpp') over network.
 */

#ifndef GG_NETWORK_HPP_INCLUDED
#define GG_NETWORK_HPP_INCLUDED

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <typeinfo>
#include "gg/var.hpp"

namespace gg
{
	class IBuffer
	{
	public:
		virtual ~IBuffer() {}
		virtual size_t available() const;
		virtual void skip(size_t len);
		virtual void clear();
		virtual void write(char c);
		virtual void write(const char* ptr, size_t len);
		virtual size_t peek(char* ptr, size_t len, size_t start_pos = 0);
		virtual size_t read(char* ptr, size_t len);
	};

	class ISerializable
	{
	public:
		virtual ~ISerializable() {}
		virtual bool init(std::shared_ptr<IBuffer>) = 0;
		virtual bool save(std::shared_ptr<IBuffer>) const = 0;
	};

	typedef std::function<bool(Var&, std::shared_ptr<IBuffer>)> InitFunction;
	typedef std::function<bool(const Var&, std::shared_ptr<IBuffer>)> SaveFunction;

	typedef uint16_t TypeIndex; // 0 is invalid

	TypeIndex addSerializerFunctions(const std::type_info&, InitFunction, SaveFunction);
	TypeIndex getTypeIndex(const std::type_info&);
	const std::type_info& getTypeInfo(TypeIndex);

	template<class T>
	uint16_t addSerializableClass()
	{
		InitFunction init_func =
			[](Var& var, std::shared_ptr<IBuffer> buf) -> bool
			{
				var.construct<T>();
				return var.get<T>().init(buf);
			};

		SaveFunction save_func =
			[](const Var& var, std::shared_ptr<IBuffer> buf) -> bool
			{
				return var.get<T>().save(buf);
			};

		return addSerializerFunctions(typeid(T), init_func, save_func);
	}

	template<class T>
	uint16_t addSerializablePOD() // plain-old-data
	{
		InitFunction init_func =
			[](Var& var, std::shared_ptr<IBuffer> buf) -> bool
			{
				var.construct<T>();
				if (buf->read(static_cast<char*>(var.getPtr()), sizeof(T)) < sizeof(T))
					return false;
				else
					return true;
			};

		SaveFunction save_func =
			[](const Var& var, std::shared_ptr<IBuffer> buf) -> bool
			{
				buf->write(static_cast<const char*>(&var.get<T>()), sizeof(T));
				return true;
			};

		return addSerializerFunctions(typeid(T), init_func, save_func);
	}
};

#endif // GG_NETWORK_HPP_INCLUDED
