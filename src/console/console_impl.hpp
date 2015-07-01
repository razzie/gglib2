/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <list>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "expression.hpp"
#include "gg/console.hpp"

namespace gg
{
	class IRenderer;
	class ITextObject;

	class Console : public IConsole, public std::streambuf
	{
	public:
		enum OutputType
		{
			NORMAL,
			FUNCTION_SUCCESS,
			FUNCTION_FAIL
		};

		enum SpecialKey
		{
			CONSOLE_TRIGGER,
			ENTER,
			CTRL_C,
			TAB,
			BACKSPACE,
			DEL,
			HOME,
			END,
			LEFT,
			RIGHT,
			UP,
			DOWN
		};

		Console();
		virtual ~Console();
		virtual bool init();
		virtual bool addFunction(const std::string& fname, Function func, Any::Array&& defaults);
		virtual unsigned complete(std::string& expression, unsigned cursor_start = 0) const;
		virtual bool exec(const std::string& expression, Any* val = nullptr) const;
		virtual void clear();
		void write(const std::string&, OutputType = OutputType::NORMAL);
		void write(std::string&&, OutputType = OutputType::NORMAL);
		void render(IRenderer*);
		void switchRendering();
		bool isRendering() const;
		void handleSpecialKeyInput(SpecialKey);
		void handleCharInput(unsigned char);

	protected:
		// inherited from std::streambuf
		int overflow(int c = std::char_traits<char>::eof());
		int sync();

	private:
		struct FunctionData
		{
			Function function;
			Any::Array defaults;
			Expression signature;

			struct Comparator
			{
				bool operator()(const std::string&, const std::string&) const;
			};
		};

		struct OutputData
		{
			OutputType type;
			std::string text;
			ITextObject* textobj;
			unsigned output_num;
			bool dirty;

			OutputData(Console& console, std::string&& text, OutputType);
			~OutputData();
		};

		mutable std::recursive_mutex m_mutex;
		std::map<std::string, FunctionData, FunctionData::Comparator> m_functions;
		std::string m_cmd;
		std::string::iterator m_cmd_pos;
		std::vector<std::string> m_cmd_history;
		std::vector<std::string>::iterator m_cmd_history_pos;
		ITextObject* m_cmd_textobj;
		bool m_cmd_dirty;
		std::map<std::thread::id, std::string> m_buffer;
		std::list<OutputData> m_output;
		unsigned m_output_counter;
		bool m_render;

		bool isValidFunctionName(const std::string&) const;
		void findMatchingFunctions(const std::string&, std::vector<std::string>&) const;
		bool completeFn(std::string&, bool print = false) const;
		bool completeFn(std::string&, const std::vector<std::string>&, bool print = false) const;
		void completeExpr(std::string&, bool print = false) const;
		bool evaluate(const Expression&, Any&) const;
		static void jumpToNextArg(const std::string&, std::string::const_iterator& in_out_pos);
	};
};
