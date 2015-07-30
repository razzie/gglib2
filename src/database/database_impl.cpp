/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "database_impl.hpp"

static gg::DatabaseManager s_db;
gg::IDatabaseManager& gg::db = s_db;


gg::Database::Cell::Cell()
{
}

gg::Database::Cell::Cell(Cell&& cell)
{
}

gg::IDatabase::ICell::Type gg::Database::Cell::getType() const
{
	return Type();
}

int32_t gg::Database::Cell::getInt32() const
{
	return int32_t();
}

int64_t gg::Database::Cell::getInt64() const
{
	return int64_t();
}

float gg::Database::Cell::getFloat() const
{
	return 0.0f;
}

double gg::Database::Cell::getDouble() const
{
	return 0.0;
}

std::string gg::Database::Cell::getString() const
{
	return std::string();
}

void gg::Database::Cell::set(int32_t)
{
}

void gg::Database::Cell::set(int64_t)
{
}

void gg::Database::Cell::set(float)
{
}

void gg::Database::Cell::set(double)
{
}

void gg::Database::Cell::set(const std::string &)
{
}

void gg::Database::Cell::save(std::fstream &)
{
}

void gg::Database::Cell::load(std::fstream &)
{
}


gg::Database::Row::Row(Table& table, Key key) :
	m_table(table)
{
}

gg::IDatabase::Access gg::Database::Row::access() const
{
	return Access();
}

gg::IDatabase::Key gg::Database::Row::key() const
{
	return Key();
}

gg::IDatabase::ICell& gg::Database::Row::cell(unsigned column)
{
	return m_cells[column];
}

gg::IDatabase::ICell& gg::Database::Row::cell(const std::string& column)
{
	for (size_t i = 0, len = m_table.m_columns.size(); i < len; ++i)
	{
		if (m_table.m_columns[i] == column)
			return m_cells[i];
	}

	return m_cells[0];
}

const gg::IDatabase::ICell& gg::Database::Row::cell(unsigned column) const
{
	return m_cells[column];
}

const gg::IDatabase::ICell& gg::Database::Row::cell(const std::string& column) const
{
	for (size_t i = 0, len = m_table.m_columns.size(); i < len; ++i)
	{
		if (m_table.m_columns[i] == column)
			return m_cells[i];
	}

	return m_cells[0];
}

void gg::Database::Row::remove()
{
}

void gg::Database::Row::save(std::fstream &)
{
}

void gg::Database::Row::load(std::fstream &)
{
}


gg::Database::Table::Table()
{
}

gg::Database::Table::Table(Table&& table)
{
}

gg::IDatabase::Access gg::Database::Table::access() const
{
	return Access();
}

const std::string & gg::Database::Table::name() const
{
	return m_name;
}

std::shared_ptr<gg::IDatabase::IRow> gg::Database::Table::createRow(Key key)
{
	return std::shared_ptr<IRow>();
}

std::shared_ptr<gg::IDatabase::IRow> gg::Database::Table::row(Key key, Access access)
{
	return std::shared_ptr<IRow>();
}

void gg::Database::Table::sync()
{
}

void gg::Database::Table::remove()
{
}

void gg::Database::Table::save(std::fstream& f)
{
}

void gg::Database::Table::load(std::fstream& f)
{
}


gg::Database::Database(const std::string& name)
{

}

const std::string& gg::Database::name() const
{
	return m_name;
}

bool gg::Database::createTable(const std::string& name, const std::vector<std::string>& columns)
{
	return false;
}

bool gg::Database::createTable(const std::string& name, unsigned columns)
{
	return false;
}

std::shared_ptr<gg::IDatabase::ITable> gg::Database::table(const std::string& name, Access access)
{
	return std::shared_ptr<ITable>();
}

void gg::Database::save()
{
}

void gg::Database::load()
{
}


std::shared_ptr<gg::IDatabase> gg::DatabaseManager::open(const std::string& name) const
{
	return{};
}
