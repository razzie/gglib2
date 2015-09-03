/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <type_traits>

namespace gg
{
	namespace __fallback
	{
		typedef char                      yes;
		typedef struct { char array[2]; } no;

		template<class T>
		no operator<< (std::ostream&, const T&);

		template<class T>
		no operator>> (std::istream&, T&);
	}


	template<class T>
	class HasStreamInserter
	{
		static constexpr __fallback::yes test(std::ostream&);
		static constexpr __fallback::no  test(...);

		static std::ostream &s;
		static std::remove_reference_t<T>& t;

		static constexpr bool check()
		{
			using namespace __fallback;
			return (sizeof(test(s << t)) == sizeof(__fallback::yes));
		}

	public:
		static bool const value = check();
	};

	template<class T>
	class HasStreamExtractor
	{
		static constexpr __fallback::yes test(std::istream&);
		static constexpr __fallback::no  test(...);

		static std::istream &s;
		static std::remove_reference_t<T>& t;

		static constexpr bool check()
		{
			using namespace __fallback;
			return (sizeof(test(s >> t)) == sizeof(__fallback::yes));
		}

	public:
		static bool const value = check();
	};


	template<class T>
	class IsContainer
	{
		typedef char                      yes;
		typedef struct { char array[2]; } no;

		// test C::iterator
		template<class C> static yes test_iter(typename C::iterator*);
		template<class C> static no  test_iter(...);

		static const bool has_iterator = (sizeof(test_iter<T>(0)) == sizeof(yes));

		// test C::begin()
		template<class C>
		using begin_signatire_test =
			std::is_same<decltype(&C::begin), typename C::iterator(C::*)()>;

		template<class C>
		static yes(&test_begin(std::enable_if_t<begin_signatire_test<C>::value>*));

		template<class C>
		static no(&test_begin(...));

		static bool const has_begin = (sizeof(test_begin<T>(0)) == sizeof(yes));

		// test C::end()
		template<class C>
		using end_signatire_test =
			std::is_same<decltype(&C::begin), typename C::iterator(C::*)()>;

		template<class C>
		static yes(&test_end(std::enable_if_t<end_signatire_test<C>::value>*));

		template<class C>
		static no(&test_end(...));

		static bool const has_end = (sizeof(test_end<T>(0)) == sizeof(yes));

	public:
		static bool const value = has_iterator && has_begin && has_end;
	};


	template<class>
	struct IsStdPair : public std::false_type
	{
	};

	template<class T1, class T2>
	struct IsStdPair<std::pair<T1, T2>> : public std::true_type
	{
	};


	class IArchive;

	template<class T>
	class HasSerializer
	{
		typedef char                      yes;
		typedef struct { char array[2]; } no;

		template<class U>
		using signature_test =
			std::is_same<decltype(&serialize), typename void(*)(IArchive&, U&)>;

		template<class U>
		static yes(&test(std::enable_if_t<signature_test<U>::value>*));

		template<class U>
		static no(&test(...));

	public:
		static bool const value = (sizeof(test<T>(0)) == sizeof(yes));
	};
};
