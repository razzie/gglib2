/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <locale>
#include <sstream>
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
	std::ostream(this),
	m_cmd_pos(m_cmd.begin()),
	m_hwnd(nullptr),
	m_driver_type(DriverType::UNKNOWN)
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
	std::stringstream ss;
	ss << defaults;
	Expression sign(ss.str());

	std::lock_guard<FastMutex> guard(m_mutex);
	return m_functions.emplace(fname, FunctionData{ func, defaults, std::move(sign) }).second;
}

unsigned gg::Console::complete(std::string& expression, unsigned cursor_start) const
{
	const size_t len = expression.size();
	std::string::iterator pos = std::next(expression.begin(), (cursor_start > len) ? len : cursor_start);
	jumpToNextArg(expression, pos);
	completeExpr(expression, false);
	return std::distance(expression.begin(), pos);
}

bool gg::Console::exec(const std::string& expression, gg::Var* rval) const
{
	try
	{
		Var v;
		Expression e(expression);
		return evaluate(e, (rval == nullptr) ? v : *rval);
	}
	catch (ExpressionError& err)
	{
		if (rval != nullptr)
			rval->construct<std::string>(err.what());
	}

	return false;
}

void gg::Console::write(const std::string& str)
{
	if (str.empty()) return;

	std::lock_guard<gg::FastMutex> guard(m_mutex);
	m_output.push_back(str);
	std::string& s = m_output.back();
	if (s.back() == '\n') s.back() = '\0';
}

void gg::Console::write(std::string&& str)
{
	if (str.empty()) return;

	std::lock_guard<gg::FastMutex> guard(m_mutex);
	m_output.push_back(str);
	std::string& s = m_output.back();
	if (s.back() == '\n') s.back() = '\0';
}

int gg::Console::overflow(int c)
{
	if (thread_buffer == nullptr)
	{
		std::lock_guard<gg::FastMutex> guard(m_mutex);
		thread_buffer = &m_buffer[std::this_thread::get_id()];
	}

	char ch = c;
	thread_buffer->append(&ch, 1);

	return c;
}

int gg::Console::sync()
{
	if (thread_buffer != nullptr)
	{
		if (thread_buffer->back() == '\n')
			thread_buffer->back() = '\0';

		write(*thread_buffer);
		thread_buffer->clear();
	}

	return 0;
}

bool gg::Console::isValidFunctionName(const std::string& fname) const
{
	if (fname.size() == 0) return false;

	std::locale loc;
	auto it = fname.begin(), end = fname.end();

	if (!std::isalpha(*it, loc) && (*it) != '_') return false;

	for (; it != end; ++it)
		if (!std::isalnum(*it, loc) && (*it) != '_') return false;

	return true;
}

void gg::Console::findMatchingFunctions(const std::string& prefix, std::vector<std::string>& out_matches) const
{
	std::lock_guard<FastMutex> guard(m_mutex);

	std::string fn = gg::trim(prefix);
	size_t len = fn.size();

	for (auto& it : m_functions)
	{
		if (gg::strncmpi(it.first, fn, len) == 0)
			out_matches.push_back(it.first);
	}
}

bool gg::Console::completeFn(std::string& fn, bool print) const
{
	std::vector<std::string> matches;
	findMatchingFunctions(fn, matches);
	return completeFn(fn, matches, print);
}

bool gg::Console::completeFn(std::string& fn, const std::vector<std::string>& matches, bool print) const
{
	if (matches.empty()) return false;

	if (matches.size() == 1)
	{
		fn = matches[0];
		return true;
	}

	fn = gg::trim(fn);

	std::locale loc;
	std::function<bool(bool)> callback =
		[&](bool rvalue) -> bool
	{
		if (print)
		{
			Console& out = const_cast<Console&>(*this);
			for (auto& s : matches)
			{
				if (s.size() > 0)
					out << "\n> " << s;
			}
			out << std::flush;
		}

		return rvalue;
	};

	size_t max_len = 0;
	size_t min_len = ~0;
	for_each(matches.begin(), matches.end(),
		[&](const std::string& s)
	{
		size_t len = s.size();
		if (len > max_len) max_len = len;
		if (len < min_len) min_len = len;
	});

	size_t fn_len = fn.size();
	if (fn_len == max_len) return callback(true);
	if (fn_len == min_len) return callback(false);

	for (size_t pos = fn_len; pos <= min_len; ++pos)
	{
		char c = (matches[0])[pos];
		for (auto it = matches.begin() + 1, end = matches.end(); it != end; ++it)
		{
			if (std::tolower((*it)[pos], loc) != std::tolower(c, loc)) return callback(false);
		}
		fn += c;
	}

	return callback(true);
}

static void completeExprSignature(gg::Expression& e, const gg::Expression& sign)
{
	auto e_it = e.begin();
	auto sign_it = sign.begin(), sign_end = sign.end();

	unsigned e_cnt = e.getChildCount();
	unsigned sign_cnt = sign.getChildCount();

	if (e_cnt < sign_cnt)
	{
		std::advance(sign_it, e_cnt);
		for (; sign_it != sign_end; ++sign_it) e.addChild(sign_it->getExpression());
	}
	else if (e_cnt > sign_cnt)
	{
		std::advance(e_it, sign_cnt);
		for (; e_it != e.end(); ++e_it) e.erase(e_it);
	}
}

void gg::Console::completeExpr(std::string& expr, bool print) const
{
	Expression e(expr, true);

	if (e.isLeaf())
	{
		std::string name = e.getName();

		if (completeFn(name, print))
		{
			e.setName(name);

			std::lock_guard<FastMutex> guard(m_mutex);
			auto pos = m_functions.find(name);
			if (pos != m_functions.end())
			{
				completeExprSignature(e, pos->second.signature);
				e.setAsExpression();
			}
		}
	}
	else
	{
		for (Expression& child : e)
		{
			if (!child.isLeaf() && // it's an expression
				(child.isRoot() || !child.getName().empty())) // not an array arg
			{
				std::string name = child.getName();
				completeFn(name, print);
				child.setName(name);

				std::lock_guard<FastMutex> guard(m_mutex);
				auto pos = m_functions.find(name);
				if (pos != m_functions.end()) completeExprSignature(e, pos->second.signature);
			}
		}
	}

	expr = e.getExpression();
}

bool gg::Console::evaluate(const gg::Expression& expr, gg::Var& rval) const
{
	std::string name = expr.getName();

	if (expr.isLeaf() && !expr.isRoot())
	{
		rval = name;
		return true;
	}
	else
	{
		if (!isValidFunctionName(name) && !name.empty())
			return false;

		VarArray va;

		for (const Expression& arg : expr)
		{
			Var rval;
			if (evaluate(arg, rval))
				va.push_back(std::move(rval));
			else
				return false;
		}

		if (name.empty())
		{
			rval = std::move(va);
			return true;
		}
		else
		{
			const Function* func = nullptr;

			{
				std::lock_guard<FastMutex> guard(m_mutex);
				auto pos = m_functions.find(name);
				if (pos != m_functions.end()) func = &(pos->second.function);
			}

			if (func != nullptr)
			{
				rval = (*func)(va);
				return true;
			}
		}
	}

	return false;
}

void gg::Console::jumpToNextArg(const std::string& expr, std::string::const_iterator& pos)
{
	for (; pos != expr.end(); ++pos)
	{
		if (*pos == '(' || *pos == ',')
		{
			++pos;
			if (pos != expr.end())
			{
				if (*pos == ' ') for (; pos != expr.end() && *pos == ' '; ++pos);
				if (*pos == '"') ++pos;
			}
			break;
		}
	}
}

gg::Console::DriverType gg::Console::getDriverType()
{
	return DriverType::GDI;
}
