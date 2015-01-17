/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_CONSOLE_IMPL_HPP_INCLUDED
#define GG_CONSOLE_IMPL_HPP_INCLUDED

#include <map>
#include <string>
#include <thread>
#include <vector>
#include "gg/fastmutex.hpp"
#include "expression.hpp"
#include "gg/console.hpp"

namespace gg
{
	class Console : public IConsole, public std::streambuf
	{
	public:
		Console();
		virtual ~Console();
		virtual bool bindToWindow(WindowHandle);
		virtual bool addFunction(const std::string& fname, Function func, VarArray&& defaults);
		virtual unsigned complete(std::string& expression, unsigned cursor_start = 0) const;
		virtual bool exec(const std::string& expression, std::ostream& output, Var* rval = nullptr) const;
		void write(const std::string&);
		void write(std::string&&);

	protected:
		// inherited from std::streambuf
		int overflow(int c = std::char_traits<char>::eof());
		int sync();

	private:
		struct FunctionData
		{
			Function function;
			VarArray defaults;
			Expression signature;

			struct Comparator
			{
				bool operator()(const std::string&, const std::string&) const;
			};
		};

		class SafeRedirect
		{
		public:
			SafeRedirect(Console&, std::ostream&);
			~SafeRedirect();

		private:
			Console& m_console;
		};

		mutable FastMutex m_mutex;
		std::map<std::string, FunctionData, FunctionData::Comparator> m_functions;
		std::string m_cmd;
		std::string::iterator m_cmd_pos;
		std::vector<std::string> m_cmd_history;
		std::vector<std::string>::iterator m_cmd_history_pos;
		WindowHandle m_hwnd;
		std::vector<std::string> m_output;
		std::map<std::thread::id, std::vector<std::ostream*>> m_redirect_stack;
		std::map<std::thread::id, std::string> m_buffer;

		bool isValidFunctionName(const std::string&) const;
		void findMatchingFunctions(const std::string&, std::vector<std::string>&) const;
		bool completeFn(std::string&, bool print = false) const;
		bool completeFn(std::string&, const std::vector<std::string>&, bool print = false) const;
		void completeExpr(std::string&, bool print = false) const;
		bool evaluate(const Expression&, Var&) const;
		static void jumpToNextArg(const std::string&, std::string::const_iterator& in_out_pos);
	};
};

#endif // GG_CONSOLE_IMPL_HPP_INCLUDED
