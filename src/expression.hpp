/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_EXPRESSION_HPP_INCLUDED
#define GG_EXPRESSION_HPP_INCLUDED

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
		void setName(std::string name);
		std::string getName() const;
		std::string getExpression() const;
		void setAsExpression();
		bool isRoot() const;
		bool isLeaf() const;
		Iterator begin();
		Iterator end();
		ConstIterator begin() const;
		ConstIterator end() const;

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

#endif // GG_EXPRESSION_HPP_INCLUDED
