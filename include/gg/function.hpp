/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_FUNCTION_HPP_INCLUDED
#define GG_FUNCTION_HPP_INCLUDED

#include <functional>
#include <type_traits>
#include <stdexcept>
#include "gg/var.hpp"

namespace gg
{
	template<class F>
	class LambdaSignature
	{
		template<class T>
		struct removeClass { };

		template<class C, class R, class... Params>
		struct removeClass<R(C::*)(Params...)>
		{
			using type = typename std::remove_pointer<R(*)(Params...)>::type;
		};

		template<class C, class R, class... Params>
		struct removeClass<R(C::*)(Params...) const>
		{
			using type = typename std::remove_pointer<R(*)(Params...)>::type;
		};

		template<class T>
		struct getSignatureImpl
		{
			using type = typename removeClass<
				decltype(&std::remove_reference<T>::type::operator())>::type;
		};

		template<class R, class... Params>
		struct getSignatureImpl<R(*)(Params...)>
		{
			using type = typename std::remove_pointer<R(*)(Params...)>::type;
		};

	public:
		using type = typename getSignatureImpl<F>::type;
	};

	template<class F>
	using getLambdaSignature = typename LambdaSignature<F>::type;


	class Function
	{
	public:
		Function()
		{
		}

		template<class F>
		Function(F func) :
			m_func(convert(func))
		{
		}

		Function(const Function& func) :
			m_func(func.m_func)
		{
		}

		Function(Function&& func) :
			m_func(std::move(func.m_func))
		{
		}

		template<class F>
		Function& operator=(F func)
		{
			m_func = convert(func);
			return *this;
		}

		Function& operator=(const Function& func)
		{
			m_func = func.m_func;
			return *this;
		}

		Function& operator=(Function&& func)
		{
			m_func = std::move(func.m_func);
			return *this;
		}

		Var operator()(const VarArray& va) const
		{
			return m_func(va);
		}

		template<class... Params>
		Var operator()(Params... params) const
		{
			return m_func({ std::forward<Params>(params)... });
		}

		operator bool() const
		{
			return static_cast<bool>(m_func);
		}
	
	private:
		template<class R>
		static R call(std::function<R()> func, const gg::VarArray& va, unsigned skipParams = 0)
		{
			if (va.size() > skipParams)
				throw std::runtime_error("too many parameters");

			return func();
		}

		template<class R, class Param>
		static R call(std::function<R(Param)> func, const gg::VarArray& va, unsigned skipParams = 0)
		{
			if (va.size() > skipParams + 1)
				throw std::runtime_error("too many parameters");

			Param param;
			va[skipParams].convert<Param>(param);

			return func(param);
		}

		template<class R, class Param0, class... Params>
		static R call(std::function<R(Param0, Params...)> func, const gg::VarArray& va, unsigned skipParams = 0)
		{
			if (va.size() <= skipParams + 1)
				throw std::runtime_error("not enough parameters");

			Param0 param0;
			va[skipParams].convert<Param0>(param0);

			std::function<R(Params...)> lambda =
				[&](Params... params) -> R { return func(param0, params...); };

			return call(lambda, va, skipParams + 1);
		}


		template<class R, class... Params>
		static std::function<Var(VarArray)> convert(std::function<R(Params...)> func)
		{
			return ([=](const VarArray& va) -> Var { return call(func, va); });
		}

		template<class... Params>
		static std::function<Var(VarArray)> convert(std::function<void(Params...)> func)
		{
			return ([=](const VarArray& va) -> Var { call(func, va); return{}; });
		}

		template<class F>
		static std::function<Var(VarArray)> convert(F func)
		{
			return convert(std::function<getLambdaSignature<F>>(func));
		}


		std::function<Var(VarArray)> m_func;
	};
};

#endif // GG_FUNCTION_HPP_INCLUDED
