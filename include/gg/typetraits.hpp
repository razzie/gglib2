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
	template<class F>
	class LambdaSignature
	{
		template<class T>
		struct RemoveClass { };

		template<class C, class R, class... Params>
		struct RemoveClass<R(C::*)(Params...)>
		{
			using Type = std::remove_pointer_t<R(*)(Params...)>;
		};

		template<class C, class R, class... Params>
		struct RemoveClass<R(C::*)(Params...) const>
		{
			using Type = std::remove_pointer_t<R(*)(Params...)>;
		};

		template<class T>
		struct GetSignatureImpl
		{
			using Type = typename RemoveClass<
				decltype(&std::remove_reference_t<T>::operator())>::Type;
		};

		template<class R, class... Params>
		struct GetSignatureImpl<R(*)(Params...)>
		{
			using Type = std::remove_pointer_t<R(*)(Params...)>;
		};

	public:
		using Type = typename GetSignatureImpl<F>::Type;
	};

	template<class F>
	using GetLambdaSignature = typename LambdaSignature<F>::Type;


	class Any;

	namespace __fallback
	{
		template<class T, class = std::enable_if_t<std::is_same<T, Any>::value>>
		std::false_type operator<< (std::ostream&, const T&);

		template<class T>
		std::true_type operator>> (std::istream&, T&);
	}


	template<class T>
	class HasStreamInserter
	{
		static std::true_type  test(std::ostream&);
		static std::false_type test(...);

		static std::ostream &s;
		static std::remove_reference_t<T>& t;

		static constexpr bool check()
		{
			using namespace __fallback;
			return decltype(test(s << t))::value;
		}

	public:
		static bool const value = check();
	};

	template<class T>
	class HasStreamExtractor
	{
		static std::true_type  test(std::istream&);
		static std::false_type test(...);

		static std::istream &s;
		static std::remove_reference_t<T>& t;

		static constexpr bool check()
		{
			using namespace __fallback;
			return decltype(test(s >> t))::value;
		}

	public:
		static const bool value = check();
	};


	template<class T>
	class IsContainer
	{
		typedef char                      Yes;
		typedef struct { char array[2]; } No;

		// test C::iterator
		template<class C>
		static constexpr Yes test_iter(typename C::iterator*);

		template<class C>
		static constexpr No  test_iter(...);

		static const bool has_iterator = (sizeof(test_iter<T>(0)) == sizeof(Yes));

		// test C::begin()
		template<class C>
		using begin_signatire_test =
			std::is_same<decltype(&C::begin), typename C::iterator(C::*)()>;

		template<class C>
		static constexpr Yes(&test_begin(std::enable_if_t<begin_signatire_test<C>::value>*));

		template<class C>
		static constexpr No(&test_begin(...));

		static constexpr bool has_begin = (sizeof(test_begin<T>(0)) == sizeof(Yes));

		// test C::end()
		template<class C>
		using end_signatire_test =
			std::is_same<decltype(&C::begin), typename C::iterator(C::*)()>;

		template<class C>
		static constexpr Yes(&test_end(std::enable_if_t<end_signatire_test<C>::value>*));

		template<class C>
		static constexpr No(&test_end(...));

		static constexpr bool has_end = (sizeof(test_end<T>(0)) == sizeof(Yes));

	public:
		static constexpr bool value = has_iterator && has_begin && has_end;
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
		static IArchive& ar;
		static T& t;

		template<class U, class R = decltype(::serialize(ar, t))>
		static std::true_type test(void*);

		template<class>
		static std::false_type test(...);

	public:
		static constexpr bool value = decltype(test<T>(nullptr))::value;
	};


	template<class... Params>
	class ParameterPack
	{
		template<int N, class... T>
		struct ParamImpl;

		template<class T0, class... T>
		struct ParamImpl<0, T0, T...>
		{
			typedef T0 Type;
		};

		template<int N, class T0, class... T>
		struct ParamImpl<N, T0, T...>
		{
			typedef typename ParamImpl<N - 1, T...>::Type Type;
		};

	public:
		template<size_t N>
		using Param = typename ParamImpl<N, Params...>;

		const size_t size = sizeof...(Params);
	};
};
