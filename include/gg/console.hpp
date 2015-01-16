/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
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
#include "gg/var.hpp"
#include "gg/function.hpp"

namespace gg
{
	typedef void* WindowHandle;

	class IConsole : public virtual std::ostream
	{
	public:
		struct ParseError
		{
			std::string error;
		};

		virtual ~IConsole() {}

		// binds the console to a window and unbinds it from the previous one (if any)
		virtual bool bindToWindow(WindowHandle) = 0;

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

		// evaluates 'expression' and returns its value (or ParseError) in 'rval'
		virtual bool exec(const std::string& expression, Var* rval = nullptr) const = 0;

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
					va.push_back(std::string("(())"));
				else
					va.push_back(std::string("\"\""));

				setDefaultsImpl<0, Ps...>(va);
			}

			template<int>
			static void setDefaultsImpl(gg::VarArray&) {}
		};
	};

	extern IConsole& console;
};

#endif // GG_CONSOLE_HPP_INCLUDED
