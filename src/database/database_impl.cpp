/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <stdexcept>
#include "database_impl.hpp"

static gg::DatabaseManager s_db;
gg::IDatabaseManager& gg::db = s_db;


static void saveString(const std::string& s, std::fstream& f)
{
	uint16_t len = static_cast<uint16_t>(s.size());
	f.write(reinterpret_cast<const char*>(&len), sizeof(uint16_t));
	f.write(s.c_str(), s.size());
}

static void loadString(std::string& s, std::fstream& f)
{
	uint16_t len = 0;
	f.read(reinterpret_cast<char*>(&len), sizeof(uint16_t));
	s.resize(len);
	f.read(&s[0], s.size());
}

static size_t sizeOfString(const std::string& s)
{
	return (sizeof(uint16_t) + s.size());
}


gg::Database::Cell::Cell() :
	m_type(Type::NONE)
{
}

gg::Database::Cell::Cell(Cell&& cell) :
	m_type(cell.m_type),
	m_data(cell.m_data),
	m_str_data(std::move(cell.m_str_data))
{
}

gg::IDatabase::ICell::Type gg::Database::Cell::getType() const
{
	return m_type;
}

int32_t gg::Database::Cell::getInt32() const
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	switch (m_type)
	{
	case Type::INT32:
		return static_cast<int32_t>(m_data.i32);
	case Type::INT64:
		return static_cast<int32_t>(m_data.i64);
	case Type::FLOAT:
		return static_cast<int32_t>(m_data.f);
	case Type::DOUBLE:
		return static_cast<int32_t>(m_data.d);
	case Type::STRING:
		return std::stoi(m_str_data);

	default:
		return 0;
	}
}

int64_t gg::Database::Cell::getInt64() const
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	switch (m_type)
	{
	case Type::INT32:
		return static_cast<int64_t>(m_data.i32);
	case Type::INT64:
		return static_cast<int64_t>(m_data.i64);
	case Type::FLOAT:
		return static_cast<int64_t>(m_data.f);
	case Type::DOUBLE:
		return static_cast<int64_t>(m_data.d);
	case Type::STRING:
		return std::stol(m_str_data);

	default:
		return 0;
	}
}

float gg::Database::Cell::getFloat() const
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	switch (m_type)
	{
	case Type::INT32:
		return static_cast<float>(m_data.i32);
	case Type::INT64:
		return static_cast<float>(m_data.i64);
	case Type::FLOAT:
		return static_cast<float>(m_data.f);
	case Type::DOUBLE:
		return static_cast<float>(m_data.d);
	case Type::STRING:
		return std::stof(m_str_data);

	default:
		return 0.f;
	}
}

double gg::Database::Cell::getDouble() const
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	switch (m_type)
	{
	case Type::INT32:
		return static_cast<double>(m_data.i32);
	case Type::INT64:
		return static_cast<double>(m_data.i64);
	case Type::FLOAT:
		return static_cast<double>(m_data.f);
	case Type::DOUBLE:
		return static_cast<double>(m_data.d);
	case Type::STRING:
		return std::stod(m_str_data);

	default:
		return 0.f;
	}
}

std::string gg::Database::Cell::getString() const
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	switch (m_type)
	{
	case Type::INT32:
		return std::to_string(m_data.i32);
	case Type::INT64:
		return std::to_string(m_data.i64);
	case Type::FLOAT:
		return std::to_string(m_data.f);
	case Type::DOUBLE:
		return std::to_string(m_data.d);
	case Type::STRING:
		return m_str_data;

	default:
		return {};
	}
}

void gg::Database::Cell::set(int32_t i)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	m_type = Type::INT32;
	m_data.i32 = i;
}

void gg::Database::Cell::set(int64_t i)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	m_type = Type::INT64;
	m_data.i64 = i;
}

void gg::Database::Cell::set(float f)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	m_type = Type::FLOAT;
	m_data.f = f;
}

void gg::Database::Cell::set(double d)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	m_type = Type::DOUBLE;
	m_data.d = d;
}

void gg::Database::Cell::set(const std::string& s)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	m_type = Type::STRING;
	m_str_data = s;
}

size_t gg::Database::Cell::size() const
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	switch (m_type)
	{
	case Type::INT32:
		return (sizeof(Type) + sizeof(int32_t));
	case Type::INT64:
		return (sizeof(Type) + sizeof(int64_t));
	case Type::FLOAT:
		return (sizeof(Type) + sizeof(float));
	case Type::DOUBLE:
		return (sizeof(Type) + sizeof(double));
	case Type::STRING:
		return (sizeof(Type) + sizeOfString(m_str_data));

	default:
		return 0;
	}
}

void gg::Database::Cell::save(std::fstream& f) const
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	f.write(reinterpret_cast<const char*>(&m_type), sizeof(Type));
	switch (m_type)
	{
	case Type::INT32:
		f.write(reinterpret_cast<const char*>(&m_data.i32), sizeof(int32_t));
		break;
	case Type::INT64:
		f.write(reinterpret_cast<const char*>(&m_data.i64), sizeof(int64_t));
		break;
	case Type::FLOAT:
		f.write(reinterpret_cast<const char*>(&m_data.f), sizeof(float));
		break;
	case Type::DOUBLE:
		f.write(reinterpret_cast<const char*>(&m_data.d), sizeof(double));
		break;
	case Type::STRING:
		saveString(m_str_data, f);
		break;

	default:
		break;
	}
}

void gg::Database::Cell::load(std::fstream& f)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	f.read(reinterpret_cast<char*>(&m_type), sizeof(Type));
	switch (m_type)
	{
	case Type::INT32:
		f.read(reinterpret_cast<char*>(&m_data.i32), sizeof(int32_t));
		break;
	case Type::INT64:
		f.read(reinterpret_cast<char*>(&m_data.i64), sizeof(int64_t));
		break;
	case Type::FLOAT:
		f.read(reinterpret_cast<char*>(&m_data.f), sizeof(float));
		break;
	case Type::DOUBLE:
		f.read(reinterpret_cast<char*>(&m_data.d), sizeof(double));
		break;
	case Type::STRING:
		loadString(m_str_data, f);
		break;

	default:
		throw std::runtime_error("Invalid cell type");
	}
}


gg::Database::Row::Row(Table& table, Key key) :
	m_table(table),
	m_key(key),
	m_force_remove(false)
{
	m_cells.resize(m_table.m_columns.size());
}

gg::IDatabase::Access gg::Database::Row::access() const
{
	return Access::NO_ACCESS;
}

gg::IDatabase::Key gg::Database::Row::key() const
{
	return m_key;
}

gg::IDatabase::ICell& gg::Database::Row::cell(unsigned column)
{
	if (column > m_cells.size())
		throw std::out_of_range({});

	return m_cells[column];
}

gg::IDatabase::ICell& gg::Database::Row::cell(const std::string& column)
{
	for (size_t i = 0, len = m_table.m_columns.size(); i < len; ++i)
	{
		if (m_table.m_columns[i] == column)
			return m_cells[i];
	}

	throw std::out_of_range({});
}

const gg::IDatabase::ICell& gg::Database::Row::cell(unsigned column) const
{
	if (column > m_cells.size())
		throw std::out_of_range({});

	return m_cells[column];
}

const gg::IDatabase::ICell& gg::Database::Row::cell(const std::string& column) const
{
	for (size_t i = 0, len = m_table.m_columns.size(); i < len; ++i)
	{
		if (m_table.m_columns[i] == column)
			return m_cells[i];
	}

	throw std::out_of_range({});
}

void gg::Database::Row::remove()
{
	m_force_remove = true;
}

size_t gg::Database::Row::size() const
{
	size_t size = sizeof(Key);
	for (const Cell& cell : m_cells)
		size += cell.size();
	return size;
}

void gg::Database::Row::save(std::fstream& f) const
{
	f.write(reinterpret_cast<const char*>(&m_key), sizeof(Key));
	for (const Cell& cell : m_cells)
		cell.save(f);
}

void gg::Database::Row::load(std::fstream& f)
{
	f.read(reinterpret_cast<char*>(&m_key), sizeof(Key));
	for (Cell& cell : m_cells)
		cell.load(f);
}


gg::Database::Table::Table(const std::string& name, const std::vector<std::string>& columns) :
	m_name(name),
	m_force_remove(false)
{
	m_columns.insert(m_columns.end(), columns.begin(), columns.end());
}

gg::IDatabase::Access gg::Database::Table::access() const
{
	return Access::NO_ACCESS;
}

const std::string& gg::Database::Table::name() const
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
	m_force_remove = true;
}

size_t gg::Database::Table::size() const
{
	size_t size = sizeOfString(m_name);

	size += sizeof(uint16_t); // column_count
	for (const std::string& column : m_columns)
		size += sizeOfString(column);

	size += sizeof(uint16_t); // row_count
	for (auto& row : m_rows)
		size += row.second.size();

	return size;
}

void gg::Database::Table::save(std::fstream& f) const
{
	saveString(m_name, f);

	uint16_t column_count = m_columns.size();
	f.write(reinterpret_cast<const char*>(&column_count), sizeof(uint16_t));
	for (const std::string& column : m_columns)
		saveString(column, f);

	uint16_t row_count = m_rows.size();
	f.write(reinterpret_cast<const char*>(&column_count), sizeof(uint16_t));
	for (auto& row : m_rows)
		row.second.save(f);
}

void gg::Database::Table::load(std::fstream& f)
{
	loadString(m_name, f);

	uint16_t column_count = 0;
	f.read(reinterpret_cast<char*>(&column_count), sizeof(uint16_t));
	for (uint16_t i = 0; i < column_count; ++i)
	{
		std::string column;
		loadString(column, f);
		m_columns.emplace_back(std::move(column));
	}

	// TBD
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
