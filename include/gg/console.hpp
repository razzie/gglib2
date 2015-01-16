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
		virtual bool addFunction(const std::string& fname, gg::Function func, gg::VarArray&& defaults) = 0;

		// completes the missing parts of 'cmdline' (if possible) and returns cursor position
		virtual unsigned complete(std::string& cmdline, unsigned cursor_start = 0) const = 0;

		// 'rval' will contain the value of the expression in 'cmdline' or ParseError
		virtual bool exec(const std::string& cmdline, Var* rval = nullptr) const = 0;

	protected:
		IConsole() : std::ostream(nullptr) {}
	};

	extern IConsole& console;
};

#endif // GG_CONSOLE_HPP_INCLUDED
