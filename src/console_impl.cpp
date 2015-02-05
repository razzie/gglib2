/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <cctype>
#include <locale>
#include <sstream>
#include "stringutil.hpp"
#include "renderer/renderer.hpp"
#include "console_impl.hpp"

#if defined _MSC_VER
#define THREAD_LOCAL __declspec(thread)
#elif defined __GNUG__
#define THREAD_LOCAL __thread
#else
#define THREAD_LOCAL thread_local
#endif

static gg::Console s_console;
gg::IConsole& gg::console = s_console;

static THREAD_LOCAL std::string* thread_buffer = nullptr;


#ifdef _WIN32
#include <windows.h>

static WNDPROC orig_wnd_proc;
static HWND console_hwnd = 0;

static LRESULT CALLBACK consoleWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN && wParam == VK_F11)
	{
		s_console.switchRendering();
		return 0;
	}

	if (!s_console.isRendering())
	{
		return CallWindowProc(orig_wnd_proc, hwnd, uMsg, wParam, lParam);
	}

	switch (uMsg)
	{
	case WM_CHAR:
	case WM_KEYUP:
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_RETURN:
			s_console.handleSpecialKeyInput(gg::Console::SpecialKey::ENTER);
			break;

		case VK_TAB:
			s_console.handleSpecialKeyInput(gg::Console::SpecialKey::TAB);
			break;

		case VK_BACK:
			s_console.handleSpecialKeyInput(gg::Console::SpecialKey::BACKSPACE);
			break;

		case VK_DELETE:
			s_console.handleSpecialKeyInput(gg::Console::SpecialKey::DEL);
			break;

		case VK_HOME:
			s_console.handleSpecialKeyInput(gg::Console::SpecialKey::HOME);
			break;

		case VK_END:
			s_console.handleSpecialKeyInput(gg::Console::SpecialKey::END);
			break;

		case VK_LEFT:
			s_console.handleSpecialKeyInput(gg::Console::SpecialKey::LEFT);
			break;

		case VK_RIGHT:
			s_console.handleSpecialKeyInput(gg::Console::SpecialKey::RIGHT);
			break;

		case VK_UP:
			s_console.handleSpecialKeyInput(gg::Console::SpecialKey::UP);
			break;

		case VK_DOWN:
			s_console.handleSpecialKeyInput(gg::Console::SpecialKey::DOWN);
			break;

		default:
			{
				BYTE kbs[256];
				GetKeyboardState(kbs);

				if (wParam == 'C' && kbs[VK_CONTROL] & 0x00000080)
				{
					s_console.handleSpecialKeyInput(gg::Console::SpecialKey::CTRL_C);
				}
				else
				{
					WORD ch;

					if (kbs[VK_CONTROL] & 0x00000080)
					{
						kbs[VK_CONTROL] &= 0x0000007f;
						ToAscii(wParam, MapVirtualKey(wParam, MAPVK_VK_TO_VSC), kbs, &ch, 0);
						kbs[VK_CONTROL] |= 0x00000080;
					}
					else
					{
						ToAscii(wParam, MapVirtualKey(wParam, MAPVK_VK_TO_VSC), kbs, &ch, 0);
					}

					s_console.handleCharInput((unsigned char)ch);
				}
			}
			break;
		}
		break;

	default:
		return CallWindowProc(orig_wnd_proc, hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

static void unhookWnd()
{
	if (!console_hwnd) return;

	SetWindowLongPtr(console_hwnd, GWLP_WNDPROC, (LONG_PTR)orig_wnd_proc);
	console_hwnd = 0;
}

static void hookWnd(HWND hwnd)
{
	if (console_hwnd)
		unhookWnd();

	orig_wnd_proc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);
	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)consoleWndProc);
	console_hwnd = hwnd;
}

#endif // _WIN32


bool gg::Console::FunctionData::Comparator
	::operator()(const std::string& a, const std::string& b) const
{
	return (gg::strcmpi(a, b) < 0);
}

gg::Console::OutputData::OutputData(gg::Console& console, std::string&& str, OutputType type) :
	type(type),
	text(str),
	textobj(nullptr),
	output_num(++console.m_output_counter),
	dirty(true)
{
}

gg::Console::OutputData::~OutputData()
{
	if (textobj != nullptr)
		delete textobj;
}


gg::Console::Console() :
	std::ostream(this),
	m_cmd_pos(m_cmd.begin()),
	m_cmd_textobj(nullptr),
	m_cmd_dirty(false),
	m_output_counter(0),
	m_render(false)
{
	static_cast<IConsole*>(this)->addFunction("clear", [&](){ gg::console.clear(); });
}

gg::Console::~Console()
{
	if (m_cmd_textobj != nullptr)
		delete m_cmd_textobj;

	m_output.clear();
}

bool gg::Console::init()
{
	IRenderer::injectHooks();
	IRenderer::setRenderCallback(std::bind(&Console::render, this, std::placeholders::_1));
	return true;
}

bool gg::Console::addFunction(const std::string& fname, gg::Function func, gg::VarArray&& defaults)
{
	std::stringstream ss;
	ss << defaults;
	Expression sign(ss.str());

	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
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

bool gg::Console::exec(const std::string& expression, gg::Var* val) const
{
	std::stringstream tmp_output;

	try
	{
		Var v;
		Expression e(expression);
		return evaluate(e, (val == nullptr) ? v : *val);
	}
	catch (ExpressionError& err)
	{
		if (val != nullptr)
			val->construct<std::string>(err.what());
	}

	return false;
}

void gg::Console::clear()
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	m_output.clear();
}

void gg::Console::write(const std::string& str, OutputType type)
{
	std::string str_copy(str);
	write(std::move(str_copy), type);
}

void gg::Console::write(std::string&& str, OutputType type)
{
	while (!str.empty() && str.back() == '\n') str.pop_back();
	if (str.empty()) return;

	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	m_output.emplace_back(*this, std::move(str), type);
}

void gg::Console::render(gg::IRenderer* renderer)
{
	#ifdef _WIN32
	if (!console_hwnd || console_hwnd != (HWND)renderer->getWindowHandle())
		hookWnd((HWND)renderer->getWindowHandle());
	#endif

	if (!m_render) return;

	//renderer->drawRectangle(10, 10, 10, 10, 0xffff0000);

	std::lock_guard<std::recursive_mutex> guard(m_mutex);

	Color shadow_color = 0xff000000;
	unsigned curr_height = 5;

	for (auto& output : m_output)
	{
		if (output.textobj == nullptr)
		{
			output.textobj = renderer->createTextObject();
			output.dirty = true;
		}

		if (output.dirty)
		{
			Color color;

			switch (output.type)
			{
			case OutputType::NORMAL:
				color = 0xffeeeeee;
				break;
			case OutputType::FUNCTION_SUCCESS:
				color = 0xffddffdd;
				break;
			case OutputType::FUNCTION_FAIL:
				color = 0xffffdddd;
				break;
			}

			if (output.output_num % 2)
				color -= 0x00222222;

			output.textobj->setText(output.text);
			output.textobj->setColor(color);
		}

		renderer->drawTextObject(output.textobj, 6, curr_height + 1, &shadow_color);
		renderer->drawTextObject(output.textobj, 5, curr_height);

		curr_height += output.textobj->getHeight() + 2;
	}

	if (m_cmd_textobj == nullptr)
	{
		m_cmd_textobj = renderer->createTextObject();
		m_cmd_textobj->setColor(0xffffffff);
		m_cmd_dirty = true;
	}

	if (m_cmd_dirty)
	{
		m_cmd_textobj->setText(m_cmd);
	}

	renderer->drawCaret(m_cmd_textobj, 5, curr_height, std::distance(m_cmd.begin(), m_cmd_pos), 0xff000000);
	renderer->drawTextObject(m_cmd_textobj, 5, curr_height + 1, &shadow_color);
	renderer->drawTextObject(m_cmd_textobj, 5, curr_height);
}

void gg::Console::switchRendering()
{
	m_render = !m_render;
}

bool gg::Console::isRendering() const
{
	return m_render;
}

void gg::Console::handleSpecialKeyInput(gg::Console::SpecialKey key)
{
	switch (key)
	{
	case SpecialKey::CONSOLE_TRIGGER:
		switchRendering();
		break;

	case SpecialKey::CTRL_C:
		m_cmd.clear();
		m_cmd_pos = m_cmd.end();
		m_cmd_dirty = true;
		break;

	case SpecialKey::ENTER:
		{
			if (m_cmd.empty()) break;

			std::string tmp_cmd;

			try
			{
				// moving currently typed command to temporary buffer
				std::swap(m_cmd, tmp_cmd);
				m_cmd_pos = m_cmd.end();
				m_cmd_dirty = true;

				// command evaluation
				Var v;
				Expression e(tmp_cmd);
				flush();
				bool result = evaluate(e, v);
				flush();

				// saving command to history
				m_cmd_history.emplace_back(tmp_cmd);
				m_cmd_history_pos = m_cmd_history.end();
				
				// printing the expression with success or fail coloring
				if (result)
				{
					if (!v.isEmpty())
					{
						std::string result_str;
						v.convert<std::string>(result_str);
						tmp_cmd += " -> ";
						tmp_cmd += std::move(result_str);
					}
					write(std::move(tmp_cmd), OutputType::FUNCTION_SUCCESS);
				}
				else
				{
					write(std::move(tmp_cmd), OutputType::FUNCTION_FAIL);
				}
			}
			catch (ExpressionError& err)
			{
				write(err.what(), OutputType::FUNCTION_FAIL);

				// in case of expression error let's put back the incomplete command
				std::swap(m_cmd, tmp_cmd);
				m_cmd_pos = m_cmd.begin();
				jumpToNextArg(m_cmd, m_cmd_pos);
			}
			catch (std::exception& e)
			{
				std::string err = typeid(e).name();
				err += " : ";
				err += e.what();
				write(std::move(err), OutputType::FUNCTION_FAIL);
			}
		}
		break;

	case SpecialKey::TAB:
		completeExpr(m_cmd, true);
		jumpToNextArg(m_cmd, m_cmd_pos);
		m_cmd_dirty = true;
		break;

	case SpecialKey::BACKSPACE:
		if (m_cmd_pos != m_cmd.begin())
		{
			m_cmd_pos = m_cmd.erase(m_cmd_pos - 1);
			m_cmd_dirty = true;
		}
		break;

	case SpecialKey::DEL:
		if (m_cmd_pos != m_cmd.end())
		{
			m_cmd_pos = m_cmd.erase(m_cmd_pos);
			m_cmd_dirty = true;
		}
		break;

	case SpecialKey::HOME:
		m_cmd_pos = m_cmd.begin();
		break;

	case SpecialKey::END:
		m_cmd_pos = m_cmd.end();
		break;

	case SpecialKey::LEFT:
		if (m_cmd_pos != m_cmd.begin())
		{
			--(m_cmd_pos);
		}
		break;

	case SpecialKey::RIGHT:
		if (m_cmd_pos != m_cmd.end())
		{
			++(m_cmd_pos);
		}
		break;

	case SpecialKey::UP:
		if (m_cmd_history_pos != m_cmd_history.begin())
		{
			m_cmd = *(--(m_cmd_history_pos));
			m_cmd_pos = m_cmd.begin();
			jumpToNextArg(m_cmd, m_cmd_pos);
			m_cmd_dirty = true;
		}
		break;

	case SpecialKey::DOWN:
		if (m_cmd_history.empty() || m_cmd_history_pos == m_cmd_history.end() - 1)
		{
			m_cmd_history_pos = m_cmd_history.end();
			m_cmd.erase();
			m_cmd_pos = m_cmd.end();
			m_cmd_dirty = true;
		}
		else if (m_cmd_history_pos != m_cmd_history.end())
		{
			m_cmd = *(++(m_cmd_history_pos));
			m_cmd_pos = m_cmd.begin();
			jumpToNextArg(m_cmd, m_cmd_pos);
			m_cmd_dirty = true;
		}
		else if (!m_cmd.empty())
		{
			m_cmd_history.emplace_back(std::move(m_cmd));
			m_cmd_history_pos = m_cmd_history.end();
			m_cmd.clear();
			m_cmd_pos = m_cmd.begin();
			m_cmd_dirty = true;
		}
		break;
	}
}

void gg::Console::handleCharInput(unsigned char ch)
{
	if (std::isprint(ch))
	{
		if (m_cmd_pos != m_cmd.end() && *(m_cmd_pos) == '0' && std::isdigit(ch) &&
			(m_cmd_pos == m_cmd.begin() || *(m_cmd_pos - 1) == '(' ||
			*(m_cmd_pos - 1) == ',' || *(m_cmd_pos - 1) == ' '))
		{
			*(m_cmd_pos) = ch;
			++(m_cmd_pos);
		}
		else
		{
			m_cmd_pos = m_cmd.insert(m_cmd_pos, ch) + 1;
		}
	}
}

int gg::Console::overflow(int c)
{
	if (thread_buffer == nullptr)
	{
		std::lock_guard<decltype(m_mutex)> guard(m_mutex);
		thread_buffer = &m_buffer[std::this_thread::get_id()];
	}

	char ch = c;
	thread_buffer->append(&ch, 1);

	return c;
}

int gg::Console::sync()
{
	if (thread_buffer != nullptr &&
		!thread_buffer->empty())
	{
		write(std::move(*thread_buffer));
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
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

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

			std::lock_guard<decltype(m_mutex)> guard(m_mutex);
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

				std::lock_guard<decltype(m_mutex)> guard(m_mutex);
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
				std::lock_guard<decltype(m_mutex)> guard(m_mutex);
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
