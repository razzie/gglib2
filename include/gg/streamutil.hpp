/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <iosfwd>
#include <locale>
#include <stdexcept>
#include <string>

namespace gg
{
	template<class T>
	class StreamManipulator
	{
	public:
		typedef std::ios&(*Manipulator)(std::ios&, T);

		StreamManipulator(Manipulator m, T t) :
			m_manip(m), m_value(t)
		{
		}

		friend std::ios& operator<< (std::ios& io, const StreamManipulator<T>& m)
		{
			return m.m_manip(io, m.m_value);
		}

	private:
		Manipulator m_manip;
		T m_value;
	};

	template<class T>
	class IstreamManipulator
	{
	public:
		typedef std::istream&(*Manipulator)(std::istream&, T);

		IstreamManipulator(Manipulator m, T t) :
			m_manip(m), m_value(t)
		{
		}

		friend std::istream& operator<< (std::istream& io, const IstreamManipulator<T>& m)
		{
			return m.m_manip(io, m.m_value);
		}

	private:
		Manipulator m_manip;
		T m_value;
	};

	template<class T>
	class OstreamManipulator
	{
	public:
		typedef std::ostream&(*Manipulator)(std::ostream&, T);

		OstreamManipulator(Manipulator m, T t) :
			m_manip(m), m_value(t)
		{
		}

		friend std::ostream& operator<< (std::ostream& io, const OstreamManipulator<T>& m)
		{
			return m.m_manip(io, m.m_value);
		}

	private:
		Manipulator m_manip;
		T m_value;
	};


	inline IstreamManipulator<char> delimiter(char d)
	{
		struct CustomLocale : std::ctype<char>
		{
			std::ctype_base::mask m_rc[table_size];

			CustomLocale(char delim) :
				std::ctype<char>(get_table(static_cast<unsigned char>(delim)))
			{
			}

			const std::ctype_base::mask* get_table(unsigned char delim)
			{
				memset(m_rc, 0, sizeof(std::ctype_base::mask) * table_size);
				m_rc[delim] = std::ctype_base::space;
				m_rc['\n'] = std::ctype_base::space;
				return &m_rc[0];
			}
		};

		IstreamManipulator<char>::Manipulator m =
			[](std::istream& i, char d) -> std::istream&
			{
				i.imbue(std::locale(i.getloc(), new CustomLocale(d)));
				return i;
			};

		return IstreamManipulator<char>(m, d);
	}

	inline IstreamManipulator<char> next(char d)
	{
		IstreamManipulator<char>::Manipulator m =
			[](std::istream& i, char d) -> std::istream&
			{
#pragma push_macro("max")
#undef max
				i.ignore(std::numeric_limits<std::streamsize>::max(), d);
#pragma pop_macro("max")
				return i;
			};

		return IstreamManipulator<char>(m, d);
	}

	inline std::istream& nextLine(std::istream& i)
	{
		return (i << next('\n'));
	}


	template<class Param>
	std::tuple<Param> parse(std::istream& i, char* d = nullptr)
	{
		Param p;
		if (d) i << delimiter(*d);
		if (!static_cast<bool>(i >> extract(p)))
			throw std::logic_error("parse error: " + typeid(Param).name());
		return std::tuple<Param> { p };
	}

	template<class Param1, class Param2, class... Params>
	std::tuple<Param1, Param2, Params...> parse(std::istream& i, char* d = nullptr)
	{
		if (d) i << delimiter(*d);
		auto a = parse<Param1>(i);
		auto b = parse<Param2, Params...>(i);
		return std::tuple_cat(a, b);
	}

	template<class... Params>
	std::tuple<Params...> parse(std::string str, char* d = nullptr)
	{
		std::stringstream ss(str);
		if (d) ss << delimiter(*d);
		return parse<Params...>(ss);
	}


	namespace __HasInsertOp
	{
		typedef char yes[2];
		typedef char no;

		template<class T>
		no operator<<(const std::ostream& s, const T&);

		yes& test(std::ostream&);
		no test(no);

		template<class T>
		struct Check
		{
			static std::ostream &s;
			static const T& t;
			static bool const value = (sizeof(test(s << t)) == sizeof(yes));
		};

		template<class T>
		class InsertHelper
		{
		public:
			InsertHelper(const T& value) :
				m_value(value)
			{
			}

			friend std::ostream& operator<<(std::ostream& o, const InsertHelper& h)
			{
				return Insert<T, __HasInsertOp::Check<T>::value>::apply(o, h.m_value);
			}

		private:
			const T& m_value;

			template<class U, bool>
			struct Insert
			{
				static std::ostream& apply(std::ostream& o, const U& value)
				{
					o << value;
					return o;
				}
			};

			template<class U>
			struct Insert<U, false>
			{
				static std::ostream& apply(std::ostream& o, const U& value)
				{
					o << typeid(T).name();
					return o;
				}
			};
		};
	}

	template<class T>
	__HasInsertOp::InsertHelper<T> insert(const T& t)
	{
		return __HasInsertOp::InsertHelper<T>(t);
	}

	namespace __HasExtractOp
	{
		typedef char no;
		typedef char yes[2];

		template<class T>
		no operator>>(const std::istream&, T&);

		yes& test(std::istream&);
		no test(no);

		template<class T>
		struct Check
		{
			static std::istream &s;
			static T& t;
			static bool const value = (sizeof(test(s >> t)) == sizeof(yes));
		};

		template<class T>
		class ExtractHelper
		{
		public:
			ExtractHelper(T& value) :
				m_value(value)
			{
			}

			friend std::istream& operator>>(std::istream& o, ExtractHelper& h)
			{
				return Extract<T, __HasExtractOp::Check<T>::value>::apply(o, h.m_value);
			}

		private:
			T& m_value;

			template<class U, bool>
			struct Extract
			{
				static std::istream& apply(std::istream& i, U& value)
				{
					i >> value;
					return i;
				}
			};

			// special case for std::string
			template<>
			struct Extract<std::string, true>
			{
				static std::istream& apply(std::istream& i, std::string& str)
				{
					std::getline(i, str);
					return i;
				}
			};

			template<class U>
			struct Extract<U, false>
			{
				static std::istream& apply(std::istream& i, U& value)
				{
					i.setstate(std::ios::failbit);
					return i;
				}
			};
		};
	}

	template<class T>
	__HasExtractOp::ExtractHelper<T> extract(T& t)
	{
		return __HasExtractOp::ExtractHelper<T>(t);
	}
};
