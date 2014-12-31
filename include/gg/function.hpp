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
	class Function
	{
	private:
		template<class T>
		struct removeClass { };

		template<class C, class R, class... Args>
		struct removeClass<R(C::*)(Args...)>
		{
			using type = typename std::remove_pointer<R(*)(Args...)>::type;
		};

		template<class C, class R, class... Args>
		struct removeClass<R(C::*)(Args...) const>
		{
			using type = typename std::remove_pointer<R(*)(Args...)>::type;
		};

		template<class T>
		struct getSignatureImpl
		{
			using type = typename removeClass<
				decltype(&std::remove_reference<T>::type::operator())>::type;
		};

		template<class R, class... Args>
		struct getSignatureImpl<R(*)(Args...)>
		{
			using type = typename std::remove_pointer<R(*)(Args...)>::type;
		};

		template<class T>
		using getSignature = typename getSignatureImpl<T>::type;


		template<class R>
		static R call(std::function<R()> func, const gg::VarArray& va, unsigned skipArgs = 0)
		{
			if (va.size() > skipArgs)
				throw std::runtime_error("too long argument list");

			return func();
		}

		template<class R, class Arg>
		static R call(std::function<R(Arg)> func, const gg::VarArray& va, unsigned skipArgs = 0)
		{
			if (va.size() > skipArgs + 1)
				throw std::runtime_error("too long argument list");

			Arg arg;
			va[skipArgs].convert<Arg>(&arg);

			return func(arg);
		}

		template<class R, class Arg0, class... Args>
		static R call(std::function<R(Arg0, Args...)> func, const gg::VarArray& va, unsigned skipArgs = 0)
		{
			if (va.size() <= skipArgs + 1)
				throw std::runtime_error("too short argument list");

			Arg0 arg0;
			va[skipArgs].convert<Arg0>(&arg0);

			std::function<R(Args...)> lambda =
				[&](Args... args) -> R { return func(arg0, args...); };

			return call(lambda, va, skipArgs + 1);
		}


		template<class R, class... Args>
		static std::function<Var(VarArray)> convert(std::function<R(Args...)> func)
		{
			return ([=](const VarArray& va) -> Var { return call(func, va); });
		}

		template<class... Args>
		static std::function<Var(VarArray)> convert(std::function<void(Args...)> func)
		{
			return ([=](const VarArray& va) -> Var { call(func, va); return{}; });
		}

		template<class F>
		static std::function<Var(VarArray)> convert(F func)
		{
			return convert(std::function<getSignature<F>>(func));
		}


		std::function<Var(VarArray)> m_func;

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

		operator bool() const
		{
			return static_cast<bool>(m_func);
		}
	};
};

#endif // GG_FUNCTION_HPP_INCLUDED
