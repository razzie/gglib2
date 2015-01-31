/**
 * Copyright (c) 2014-2015 G�bor G�rzs�ny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_CONSOLE_HPP_INCLUDED
#define GG_CONSOLE_HPP_INCLUDED

#include <iostream>
#include <string>
#include <type_traits>
#include "gg/config.hpp"
#include "gg/var.hpp"
#include "gg/function.hpp"

namespace gg
{
	class IConsole : public virtual std::ostream
	{
	public:
		virtual ~IConsole() {}

		// should be called BEFORE initializing OpenGL or Direct3D context
		virtual bool init() = 0;

		// add a function which can be used inside the console
		virtual bool addFunction(const std::string& fname, Function func, VarArray&& defaults) = 0;

		template<class F>
		bool addFunction(const std::string& fname, F func)
		{
			VarArray va;
			SignatureParams<getLambdaSignature<F>>::setDefaults(va);
			return addFunction(fname, func, std::move(va));
		}

		// completes the missing parts of 'expression' (if possible) and returns cursor position
		virtual unsigned complete(std::string& expression, unsigned cursor_start = 0) const = 0;

		// if returned true: 'expression' was evaluated and its value was copied to 'rval'
		// if returned false: 'rval' contains the error message as an 'std::string'
		// messages sent to 'gg::console' are redirected to 'output' (if specified) while execution
		virtual bool exec(const std::string& expression, Var* rval = nullptr, std::ostream* output = nullptr) const = 0;

		// clears existing output lines
		virtual void clear() = 0;

	protected:
		IConsole() : std::ostream(nullptr) {}

		template<class...>
		class SignatureParams;

		template<class R, class... Params>
		class SignatureParams<R(Params...)>
		{
		public:
			static void setDefaults(VarArray& va)
			{
				setDefaultsImpl<0, Params...>(va);
			}

		private:
			template<int /* placeholder */, class P0, class... Ps>
			static void setDefaultsImpl(gg::VarArray& va)
			{
				if (std::is_integral<P0>::value)
					va.push_back(std::string("0"));
				else if (std::is_floating_point<P0>::value)
					va.push_back(std::string("0.0"));
				else if (std::is_same<P0, gg::VarArray>::value)
					va.push_back(std::string("()"));
				else
					va.push_back(std::string("\"\""));

				setDefaultsImpl<0, Ps...>(va);
			}

			template<int>
			static void setDefaultsImpl(gg::VarArray&) {}
		};
	};

	extern GG_API IConsole& console;
};

#endif // GG_CONSOLE_HPP_INCLUDED
