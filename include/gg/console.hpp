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
#include "gg/var.hpp"

namespace gg
{
	typedef void* WindowHandle;

	class IConsole : public virtual std::ostream
	{
	public:
		virtual ~IConsole() {}

		// binds the console to a window and unbinds it from the previous one (if any)
		virtual bool bindToWindow(WindowHandle) = 0;

		// completes the missing parts of 'cmdline' (if possible) and returns cursor position
		virtual unsigned complete(std::string& cmdline) const = 0;

		// returns the value of the expression in 'cmdline' or ParseError
		virtual Var exec(const std::string& cmdline) const = 0;

	protected:
		IConsole() : std::ostream(nullptr) {}
	};

	extern IConsole& console;
};

#endif // GG_CONSOLE_HPP_INCLUDED
