/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <cstdint>
#include <iterator>
#include <string>
#include <type_traits>

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
			trySerialize(t);
			return *this;
		}

	private:
		template<class T>
		struct HasIterator
		{
		private:
			typedef char                      yes;
			typedef struct { char array[2]; } no;

			template<class C> static yes test(typename C::iterator*);
			template<class C> static no  test(...);

		public:
			static const bool value = sizeof(test<T>(0)) == sizeof(yes);
			typedef T type;
		};

		template<class T>
		struct HasBeginEnd
		{
			template<class C> static char(&f(typename std::enable_if<
				std::is_same<decltype(static_cast<typename C::iterator(C::*)()>(&C::begin)),
				typename C::iterator(C::*)()>::value, void>::type*))[1];

			template<class C> static char(&f(...))[2];

			template<class C> static char(&g(typename std::enable_if<
				std::is_same<decltype(static_cast<typename C::iterator(C::*)()>(&C::end)),
				typename C::iterator(C::*)()>::value, void>::type*))[1];

			template<class C> static char(&g(...))[2];

			static bool const beg_value = sizeof(f<T>(0)) == 1;
			static bool const end_value = sizeof(g<T>(0)) == 1;
		};

		template<class T>
		struct IsContainer : std::integral_constant<bool,
			HasIterator<T>::value && HasBeginEnd<T>::beg_value && HasBeginEnd<T>::end_value>
		{
		};

		template<class Container>
		void serializeContainer(Container& cont,
			typename std::enable_if<IsContainer<Container>::value>::type* = 0)
		{
			if (getMode() == Mode::SERIALIZE)
			{
				uint16_t size = static_cast<uint16_t>(cont.size());
				(*this) & size;
				for (Container::value_type& val : cont)
				{
					(*this) & val;
				}
			}
			else
			{
				uint16_t size;
				(*this) & size;
				for (uint16_t i = 0; i < size; ++i)
				{
					Container::value_type val;
					(*this) & val;
					*std::inserter(cont, cont.end()) = std::move(val);
				}
			}
		}

		template<class Container>
		typename std::enable_if<IsContainer<Container>::value>::type
			trySerialize(Container& cont)
		{
			serializeContainer(cont);
		}

		template<class T>
		typename std::enable_if<!IsContainer<T>::value>::type
			trySerialize(T& t)
		{
			serialize(*this, t);
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
