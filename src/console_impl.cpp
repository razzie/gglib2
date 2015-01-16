/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "stringutil.hpp"
#include "console_impl.hpp"

#if defined _MSC_VER
#define THREAD_LOCAL __declspec(thread)
#elif defined __GNUG__
#define THREAD_LOCAL __thread
#endif

static gg::Console s_console;
gg::IConsole& gg::console = s_console;

static THREAD_LOCAL std::string* thread_buffer = nullptr;


bool gg::Console::FunctionData::Comparator
	::operator()(const std::string& a, const std::string& b) const
{
	return (gg::strcmpi(a, b) < 0);
}

gg::Console::Console() :
	std::ostream(this)
{

}

gg::Console::~Console()
{
}

bool gg::Console::bindToWindow(gg::WindowHandle hwnd)
{
	return false;
}

bool gg::Console::addFunction(const std::string& fname, gg::Function func, gg::VarArray&& defaults)
{
	return false;
}

unsigned gg::Console::complete(std::string& expression, unsigned cursor_start) const
{
	return 0;
}

bool gg::Console::exec(const std::string& expression, gg::Var* rval) const
{
	return false;
}

int gg::Console::overflow(int c)
{
	return c;
}

int gg::Console::sync()
{
	return 0;
}

bool gg::Console::isValidFunctionName(const std::string& fname) const
{
	return false;
}

void gg::Console::findMatchingFunctions(const std::string& prefix, std::vector<std::string>& out_matches) const
{
	return;
}

bool gg::Console::completeFn(std::string& fname, bool print) const
{
	return false;
}

bool gg::Console::completeFn(std::string& fname, const std::vector<std::string>& in_matches, bool print) const
{
	return false;
}

void gg::Console::completeExpr(std::string& expr, bool print) const
{
	return;
}

bool gg::Console::evaluate(const gg::Expression& expr, gg::Var& rval) const
{
	return false;
}

gg::Console::DriverType gg::Console::getDriverType()
{
	return DriverType::GDI;
}
