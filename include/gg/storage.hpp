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

namespace gg
{
	template<class... Types>
	class Storage
	{
	public:
		Storage()
		{
			default_construct<0, 0, Types...>();
		}

		Storage(Types... values)
		{
			construct<0, 0, Types...>(std::forward<Types>(values)...);
		}

		virtual ~Storage()
		{
			destruct<0, Types...>();
		}

		unsigned count() const
		{
			return size;
		}

		template<class T>
		T& get(unsigned n)
		{
			if (n >= size || typeid(T) != *m_types[n]) throw std::bad_cast();
			return *reinterpret_cast<T*>(m_ptrs[n]);
		}

		template<class T>
		const T& get(unsigned n) const
		{
			if (n >= size || typeid(T) != *m_types[n]) throw std::bad_cast();
			return *reinterpret_cast<const T*>(m_ptrs[n]);
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

		const std::type_info& getType(unsigned n) const
		{
			if (n >= size) return typeid(void);
			return *m_types[n];
		}

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

		template<unsigned N, size_t offset>
		void default_construct() {}

		template<unsigned N, size_t offset, class T0, class... Ts>
		void default_construct()
		{
			m_ptrs[N] = reinterpret_cast<char*>(new (m_buffer + offset) T0());
			m_types[N] = &typeid(T0);
			default_construct<N + 1, offset + sizeof(T0), Ts...>();
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
	};
};

#endif // GG_STORAGE_HPP_INCLUDED
