/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_NETWORK_HPP_INCLUDED
#define GG_NETWORK_HPP_INCLUDED

#include <cstdint>
#include <functional>
#include <typeinfo>
#include "gg/buffer.hpp"
#include "gg/var.hpp"

namespace gg
{
	namespace net
	{
		class ISerializable
		{
		public:
			virtual bool init(std::shared_ptr<Buffer>) = 0;
			virtual bool save(std::shared_ptr<Buffer>) const = 0;
		};

		typedef std::function<bool(Var&, std::shared_ptr<Buffer>)> InitFunction;
		typedef std::function<bool(const Var&, std::shared_ptr<Buffer>)> SaveFunction;

		uint16_t addSerializerFunctions(const std::type_info&, InitFunction, SaveFunction);
		uint16_t getInternalTypeID(const std::type_info&);

		template<class T>
		uint16_t addClass()
		{
			InitFunction init_func =
				[](Var& var, std::shared_ptr<Buffer> buf) -> bool
				{
					var.construct<T>();
					return var.get<T>().init(buf);
				};

			SaveFunction save_func =
				[](const Var& var, std::shared_ptr<Buffer> buf) -> bool
				{
					return var.get<T>().save(buf);
				};

			return addSerializerFunctions(typeid(T), init_func, save_func);
		}

		template<class T>
		uint16_t addPOD() // plain-old-data
		{
			InitFunction init_func =
				[](Var& var, std::shared_ptr<Buffer> buf) -> bool
				{
					var.construct<T>();
					if (buf->read(static_cast<char*>(var.getPtr()), sizeof(T)) < sizeof(T))
						return false;
					else
						return true;
				};

			SaveFunction save_func =
				[](const Var& var, std::shared_ptr<Buffer> buf) -> bool
				{
					buf->write(static_cast<const char*>(&var.get<T>()), sizeof(T));
					return true;
				};

			return addSerializerFunctions(typeid(T), init_func, save_func);
		}
	};
};

#endif // GG_NETWORK_HPP_INCLUDED
