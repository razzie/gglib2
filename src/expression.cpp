/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <cctype>
#include "expression.hpp"
#include "stringutil.hpp"

static bool isValidLeafExpr(std::string expr)
{
	if (expr.empty() || gg::isNumeric(expr)) return true;

	int dbl_apost_cnt = 0;
	auto it = expr.begin(), end = expr.end();

	for (; it != end; ++it)
	{
		if ((*it == '\\') && (it + 1 != expr.end()) && (*(it + 1) == '"'))
		{
			++it; // it will jump to ", but we want to skip it
			continue;
		}

		if (*it == '"')
		{
			++dbl_apost_cnt;
			continue;
		}

		if (dbl_apost_cnt == 1)
			continue;

		if (dbl_apost_cnt > 2)
			return false;

		if (dbl_apost_cnt == 2 && !std::isspace(*it))
			return false;
	}

	if (dbl_apost_cnt == 0 && gg::containsSpace(expr))
		return false;

	return true;
}

static void makeValidLeafExpr(std::string& expr)
{
	if (expr.empty() || gg::isNumeric(expr)) return;

	for (auto it = expr.begin(); it != expr.end(); ++it)
	{
		if ((*it == '\\') && (it + 1 != expr.end()) && (*(it + 1) == '"'))
		{
			it = expr.erase(it); // it will jump to ", but we want to skip it
			continue;
		}

		if (*it == '"')
		{
			it = expr.erase(it);
			if (it != expr.begin()) --it;
			continue;
		}
	}
}


gg::Expression::Expression(gg::Expression* parent, std::string orig_expr, bool auto_complete) :
	m_expr(false), m_root(parent == nullptr)
{
	std::string expr = gg::trim(orig_expr);

	int open_brackets = 0;
	int dbl_apost_cnt = 0;
	auto expr_begin = expr.begin();
	enum
	{
		EXPR_NONE,
		EXPR_INCOMPLETE,
		EXPR_COMPLETE
	}
	expr_mode = EXPR_NONE;

	for (auto it = expr.begin(); it != expr.end(); ++it)
	{
		if ((*it == '\\') && (it + 1 != expr.end()) && (*(it + 1) == '"'))
		{
			++it; // it will jump to ", and then we skip it
			continue;
		}

		if (*it == '"')
		{
			++dbl_apost_cnt;
			continue;
		}

		if (dbl_apost_cnt % 2) continue;// string mode

		if (*it == '(')
		{
			++open_brackets;

			if (open_brackets == 1 && expr_mode == EXPR_NONE)
			{
				std::string name = std::string(expr.begin(), it);
				if (!auto_complete && !isValidLeafExpr(name))
					throw gg::ExpressionError("invalid expression: " + name);

				expr_mode = EXPR_INCOMPLETE;
				expr_begin = it + 1;
				makeValidLeafExpr(name);
				this->setName(name);
				continue;
			}

			continue;
		}

		if (*it == ')')
		{
			--open_brackets;

			if (open_brackets < 0)
			{
				if (auto_complete) { it = --expr.erase(it); continue; }
				throw gg::ExpressionError("invalid use of )");
			}

			else if (open_brackets == 0 && expr_mode == EXPR_INCOMPLETE)
			{
				m_expr = true;
				std::string child_expr(expr_begin, it);
				if (dbl_apost_cnt || !trim(child_expr).empty())
				{
					this->addChild(child_expr, auto_complete);
				}

				expr_mode = EXPR_COMPLETE;
				continue;
			}

			continue;
		}

		if (*it == ',')
		{
			if (open_brackets == 0)
				throw gg::ExpressionError(", found before expression");

			else if (open_brackets == 1)
			{
				m_expr = true;
				std::string child_expr(expr_begin, it);
				if (dbl_apost_cnt || !trim(child_expr).empty())
				{
					this->addChild(child_expr, auto_complete);
				}

				expr_begin = it + 1;
				dbl_apost_cnt = 0;
				continue;
			}

			continue;
		}

		if (expr_mode == EXPR_COMPLETE && !std::isspace(*it))
		{
			if (auto_complete) { it = --expr.erase(it); continue; }
			else throw gg::ExpressionError("character found after expression");
		}
	}

	if (auto_complete)
	{
		if (expr_mode == EXPR_INCOMPLETE)
		{
			m_expr = true;
			std::string child_expr(expr_begin, expr.end());
			if (dbl_apost_cnt % 2) child_expr += '"';
			if (open_brackets > 0) for (int i = open_brackets; i > 0; --i) child_expr += ')';

			if (dbl_apost_cnt || !trim(child_expr).empty())
			{
				this->addChild(child_expr, auto_complete);
			}
		}
		else if (expr_mode == EXPR_NONE)
		{
			makeValidLeafExpr(expr);
			this->setName(expr);
		}
	}
	else
	{
		if (dbl_apost_cnt % 2)
			throw gg::ExpressionError("missing \"");

		if (open_brackets > 0)
			throw gg::ExpressionError("missing )");

		if (expr_mode == EXPR_NONE)
		{
			if (!isValidLeafExpr(expr))
				throw gg::ExpressionError("invalid expression: " + expr);

			makeValidLeafExpr(expr);
			this->setName(expr);
		}
	}
}

gg::Expression::Expression(std::string expr, bool auto_complete) :
	gg::Expression(nullptr, expr, auto_complete)
{
}

gg::Expression::Expression(gg::Expression&& expr) :
	m_name(std::move(expr.m_name)),
	m_children(std::move(expr.m_children)),
	m_expr(expr.m_expr),
	m_root(expr.m_root)
{
}

gg::Expression::~Expression()
{
}

std::string gg::Expression::getName() const
{
	return m_name;
}

std::string gg::Expression::getExpression() const
{
	std::string expr;

	if (isLeaf())
	{
		if (m_root || gg::isNumeric(m_name)/* || m_name.empty()*/) expr += m_name;
		else expr += '"' + m_name + '"';
	}
	else
	{
		expr += m_name;
		expr += '(';
		auto it = m_children.cbegin(), end = m_children.cend();
		for (; it != end; ++it)
		{
			expr += it->getExpression();
			if (std::next(it, 1) != end) expr += ", ";
		}
		expr += ')';
	}

	return expr;
}

unsigned gg::Expression::getChildCount() const
{
	return m_children.size();
}

bool gg::Expression::isRoot() const
{
	return m_root;
}

bool gg::Expression::isLeaf() const
{
	return (m_children.empty() && !m_expr);
}

void gg::Expression::setName(std::string name)
{
	if (!isLeaf() && gg::containsSpace(name))
		throw gg::ExpressionError("non-leaf expressions cannot contain space");

	m_name = trim(name);
}

void gg::Expression::addChild(std::string expr, bool auto_complete)
{
	m_children.emplace(m_children.end(), this, expr, auto_complete);
}

void gg::Expression::setAsExpression()
{
	m_expr = true;
}

gg::Expression::Iterator gg::Expression::begin()
{
	return m_children.begin();
}

gg::Expression::Iterator gg::Expression::end()
{
	return m_children.end();
}

gg::Expression::ConstIterator gg::Expression::begin() const
{
	return m_children.begin();
}

gg::Expression::ConstIterator gg::Expression::end() const
{
	return m_children.end();
}

gg::Expression::Iterator gg::Expression::insert(Iterator it, std::string expr, bool auto_complete)
{
	return m_children.emplace(it, this, expr, auto_complete);
}

gg::Expression::Iterator gg::Expression::erase(Iterator it)
{
	return m_children.erase(it);
}
