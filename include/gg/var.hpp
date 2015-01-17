/**
 * Copyright (c) 2014-2015 G�bor G�rzs�ny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_VAR_HPP_INCLUDED
#define GG_VAR_HPP_INCLUDED

#include <algorithm>
#include <iosfwd>
#include <typeinfo>
#include <sstream>
#include <vector>
#include "gg/streamutil.hpp"

namespace gg
{
	class Var
	{
	public:
		Var()
		{
		}

		Var(const Var& v)
		{
			if (v.m_storage != nullptr)
				m_storage = v.m_storage->clone();
		}

		Var(Var&& v)
		{
			std::swap(m_storage, v.m_storage);
		}

		~Var()
		{
			if (m_storage != nullptr)
				delete m_storage;
		}

		template<class T>
		Var(T t) :
			m_storage(new Storage<T>(t))
		{
		}

		template<class T, class... Args>
		Var& construct(Args... args)
		{
			if (m_storage != nullptr)
				delete m_storage;

			m_storage = new Storage<T>(std::forward<Args>(args)...);

			return *this;
		}

		template<class T>
		Var& operator= (const T& t)
		{
			if (m_storage != nullptr)
				delete m_storage;

			m_storage = new Storage<T>(t);

			return *this;
		}

		Var& operator= (const Var& v)
		{
			if (m_storage != nullptr)
			{
				delete m_storage;
				m_storage = nullptr;
			}

			if (v.m_storage != nullptr)
				m_storage = v.m_storage->clone();

			return *this;
		}

		Var& operator= (Var&& v)
		{
			std::swap(m_storage, v.m_storage);

			return *this;
		}

		const std::type_info& getType() const
		{
			if (m_storage != nullptr)
				return m_storage->getType();
			else
				return typeid(void);
		}

		bool isEmpty() const
		{
			return (m_storage == nullptr);
		}

		void clear()
		{
			if (m_storage != nullptr)
			{
				delete m_storage;
				m_storage = nullptr;
			}
		}

		void* getPtr()
		{
			if (m_storage != nullptr)
				return static_cast<void*>(m_storage->getPtr());
			else
				return nullptr;
		}

		const void* getPtr() const
		{
			if (m_storage != nullptr)
				return static_cast<const void*>(m_storage->getPtr());
			else
				return nullptr;
		}

		template<class T>
		T& get()
		{
			if (m_storage == nullptr || m_storage->get_type() != typeid(T))
				throw std::bad_cast();

			return *static_cast<T*>(m_storage->get_ptr());
		}

		template<class T>
		const T& get()
		{
			if (m_storage == nullptr || m_storage->get_type() != typeid(T))
				throw std::bad_cast();

			return *static_cast<const T*>(m_storage->get_ptr());
		}

		template<class T>
		operator T() const
		{
			return get<T>();
		}

		template<class T>
		bool convert(T& p) const
		{
			if (m_storage == nullptr)
				return false;

			if (m_storage->getType() == typeid(T))
			{
				p = *static_cast<const T*>(m_storage->getPtr());
				return true;
			}
			else
			{
				std::stringstream ss;
				m_storage->toStream(ss);
				return static_cast<bool>(ss >> extract(p));
			}
		}

		friend std::ostream& operator<<(std::ostream&, const Var&);

	private:
		class IStorage
		{
		public:
			virtual ~IStorage() {}
			virtual IStorage* clone() const = 0;
			virtual void* getPtr() = 0;
			virtual const void* getPtr() const = 0;
			virtual const std::type_info& getType() const = 0;
			virtual void toStream(std::ostream&) const = 0;
		};

		template<class T>
		class Storage : public IStorage
		{
		public:
			template<class... Args>
			Storage(Args... args) :
				m_value(std::forward<Args>(args)...), m_type(&typeid(T))
			{
			}

			IStorage* clone() const
			{
				return new Storage<T>(m_value);
			}

			void* getPtr()
			{
				return static_cast<void*>(&m_value);
			}

			const void* getPtr() const
			{
				return static_cast<const void*>(&m_value);
			}

			const std::type_info& getType() const
			{
				return *m_type;
			}

			void toStream(std::ostream& o) const
			{
				o << insert(m_value);
			}

		private:
			T m_value;
			const std::type_info* m_type;
		};

		IStorage* m_storage = nullptr;
	};


	typedef std::vector<Var> VarArray;

	inline std::ostream& operator<<(std::ostream& s, const Var& v)
	{
		Var::IStorage* storage = v.m_storage;

		if (storage != nullptr)
			storage->toStream(s);

		return s;
	}

	inline std::ostream& operator<<(std::ostream& s, const VarArray& va)
	{
		auto it = va.begin();

		s << "(" << insert(*(it++));
		std::for_each(it, va.end(), [&](const Var& v){ s << ", " << insert(v); });
		s << ")";

		return s;
	}
};

#endif // GG_VAR_HPP_INCLUDED
