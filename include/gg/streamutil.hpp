#ifndef GG_STREAMOPS_HPP_INCLUDED
#define GG_STREAMOPS_HPP_INCLUDED

#include <iosfwd>
#include <locale>
#include <stdexcept>

namespace gg
{
	template<class T>
	class StreamManipulator
	{
	public:
		typedef std::ios&(*Manipulator)(std::ios&, T);

		StreamManipulator(Manipulator m, T v) :
			m_manip(m), m_value(v)
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

		IstreamManipulator(Manipulator m, T v) :
			m_manip(m), m_value(v)
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

		OstreamManipulator(Manipulator m, T v) :
			m_manip(m), m_value(v)
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


	OstreamManipulator<const char*> format(const char*);

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
				i.ignore(std::numeric_limits<std::streamsize>::max(), d);
				return i;
			};

		return IstreamManipulator<char>(m, d);
	}

	inline std::istream& nextLine(std::istream& i)
	{
		return (i << next('\n'));
	}


	template<class Arg>
	std::tuple<Arg> parse(std::istream& i, char* d = nullptr)
	{
		Arg a;
		if (d) i << delimiter(*d);
		if (!istream_extract(i, a)) throw std::runtime_error("can't extract arg");
		return std::tuple<Arg> { a };
	}

	template<class Arg1, class Arg2, class... Args>
	std::tuple<Arg1, Arg2, Args...> parse(std::istream& i, char* d = nullptr)
	{
		if (d) i << delimiter(*d);
		auto a = parse<Arg1>(i);
		auto b = parse<Arg2, Args...>(i);
		return std::tuple_cat(a, b);
	}

	template<class... Args>
	std::tuple<Args...> parse(std::string str, char* d = nullptr)
	{
		std::stringstream ss(str);
		if (d) ss << delimiter(*d);
		return parse<Args...>(ss);
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

		public:
			InsertHelper(const T& value) :
				m_value(value)
			{
			}

			friend std::ostream& operator<<(std::ostream& o, const InsertHelper& h)
			{
				return Insert<T, __HasInsertOp::Check<T>::value>::apply(o, h.m_value);
			}
		};
	}

	template<class T>
	__HasInsertOp::InsertHelper<T> insert(const T& v)
	{
		return __HasInsertOp::InsertHelper<T>(v);
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

			template<class U>
			struct Extract<U, false>
			{
				static std::istream& apply(std::istream& i, U& value)
				{
					i.setstate(std::ios::failbit);
					return i;
				}
			};

		public:
			ExtractHelper(T& value) :
				m_value(value)
			{
			}

			friend std::istream& operator>>(std::istream& o, ExtractHelper& h)
			{
				return Extract<T, __HasExtractOp::Check<T>::value>::apply(o, h.m_value);
			}
		};
	}

	template<class T>
	__HasExtractOp::ExtractHelper<T> extract(T& v)
	{
		return __HasExtractOp::ExtractHelper<T>(v);
	}
};

#endif // GG_STREAMOPS_HPP_INCLUDED
