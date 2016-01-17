/**
 * Copyright (c) 2014-2016 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <vector>

namespace gg
{
	class Expression final
	{
	public:
		typedef std::vector<Expression>::iterator Iterator;
		typedef std::vector<Expression>::const_iterator ConstIterator;

		Expression(std::string expr, bool auto_complete = false);
		Expression(Expression&& e);
		~Expression();
		Expression& operator=(Expression&&);
		std::string getName() const;
		std::string getExpression() const;
		unsigned getChildCount() const;
		bool isRoot() const;
		bool isLeaf() const;
		void setName(std::string name);
		void addChild(std::string expr, bool auto_complete = false);
		void setAsExpression();
		Iterator begin();
		Iterator end();
		ConstIterator begin() const;
		ConstIterator end() const;
		Iterator insert(Iterator, std::string expr, bool auto_complete = false);
		Iterator erase(Iterator);

	protected:
		friend class std::allocator<Expression>;
		Expression(Expression* parent, std::string expr, bool auto_complete);

	private:
		std::string m_name;
		std::vector<Expression> m_children;
		bool m_expr;
		bool m_root;
	};

	class ExpressionError : public std::exception
	{
	public:
		ExpressionError(std::string error) : m_error(error)
		{
		}

		virtual ~ExpressionError()
		{
		}

		virtual const char* what() const
		{
			return m_error.c_str();
		}

	private:
		std::string m_error;
	};
};
