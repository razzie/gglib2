/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_STORAGE_HPP_INCLUDED
#define GG_STORAGE_HPP_INCLUDED

#include <typeinfo>
//#include <stdexcept>

namespace gg
{
	template<class... Types>
	class Storage
	{
	private:
		template<unsigned...>
		struct sum;

		template<unsigned size>
		struct sum<size>
		{
			enum { value = size };
		};

		template<unsigned size, unsigned... sizes>
		struct sum<size, sizes...>
		{
			enum { value = size + sum<sizes...>::value };
		};

		template<unsigned N, size_t offset>
		void construct() {}

		template<unsigned N, size_t offset, class T0, class... Ts>
		void construct(T0 t0, Ts... ts)
		{
			m_ptrs[N] = reinterpret_cast<char*>(new (m_buffer + offset) T0(t0));
			m_types[N] = &typeid(T0);
			construct<N + 1, offset + sizeof(T0), Ts...>(ts...);
		}

		template<size_t offset>
		void destruct() {}

		template<size_t offset, class T0, class... Ts>
		void destruct()
		{
			reinterpret_cast<T0*>(m_buffer + offset)->~T0();
			destruct<offset + sizeof(T0), Ts...>();
		}

		static const unsigned size = sizeof...(Types);
		char  m_buffer[sum<sizeof(Types)...>::value];
		char* m_ptrs[size];
		const std::type_info* m_types[size];

	public:
		Storage(Types... values)
		{
			construct<0, 0, Types...>(values...);
		}

		virtual ~Storage()
		{
			destruct<0, Types...>();
		}

		unsigned count() const
		{
			return size;
		}

		char* getPtr(unsigned n)
		{
			if (n >= size) return nullptr;
			return m_ptrs[n];
		}

		const char* getPtr(unsigned n) const
		{
			if (n >= size) return nullptr;
			return m_ptrs[n];
		}

		template<unsigned N, class T>
		T& get()
		{
			if (N >= size || typeid(T) != *m_types[N]) throw std::bad_cast();
			return *reinterpret_cast<T*>(getPtr(N));
		}

		template<unsigned N, class T>
		const T& get() const
		{
			if (N >= size || typeid(T) != *m_types[N]) throw std::bad_cast();
			return *reinterpret_cast<const T*>(getPtr(N));
		}
	};
};

#endif // GG_STORAGE_HPP_INCLUDED
