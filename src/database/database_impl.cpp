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



gg::Database::AccessError::AccessError(AccessType requested, AccessType actual) :
	m_requested(requested),
	m_actual(actual)
{
}

const char* gg::Database::AccessError::what()
{
	return "Access error";
}

gg::IDatabase::AccessType gg::Database::AccessError::getRequestedAccess() const
{
	return m_requested;
}

gg::IDatabase::AccessType gg::Database::AccessError::getActualAccess() const
{
	return m_actual;
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

size_t gg::Database::Cell::getSize() const
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
	m_writer_views(0),
	m_reader_views(0),
	m_force_remove(false)
{
	m_cells.resize(m_table.m_columns.size());
}

gg::Database::Row::Row(Row&& row) :
	m_table(row.m_table),
	m_key(row.m_key),
	m_cells(std::move(row.m_cells)),
	m_writer_views(0),
	m_reader_views(0),
	m_force_remove(false)
{
}

gg::IDatabase::AccessType gg::Database::Row::getAccessType() const
{
	return AccessType::NO_ACCESS;
}

gg::IDatabase::Key gg::Database::Row::getKey() const
{
	return m_key;
}

gg::IDatabase::ICell* gg::Database::Row::cell(unsigned column)
{
	if (column > m_cells.size())
		return nullptr;

	return &m_cells[column];
}

gg::IDatabase::ICell* gg::Database::Row::cell(const std::string& column)
{
	for (size_t i = 0, len = m_table.m_columns.size(); i < len; ++i)
	{
		if (m_table.m_columns[i] == column)
			return &m_cells[i];
	}

	return nullptr;
}

const gg::IDatabase::ICell* gg::Database::Row::cell(unsigned column) const
{
	if (column > m_cells.size())
		return nullptr;

	return &m_cells[column];
}

const gg::IDatabase::ICell* gg::Database::Row::cell(const std::string& column) const
{
	for (size_t i = 0, len = m_table.m_columns.size(); i < len; ++i)
	{
		if (m_table.m_columns[i] == column)
			return &m_cells[i];
	}

	return nullptr;
}

void gg::Database::Row::remove()
{
	m_force_remove = true;
}

std::shared_ptr<gg::IDatabase::IRow> gg::Database::Row::createView(bool write_access)
{
	if (m_force_remove)
		return {};
	else
		return std::shared_ptr<gg::IDatabase::IRow>(new RowView(*this, write_access));
}

size_t gg::Database::Row::getSize() const
{
	size_t size = sizeof(Key);
	for (const Cell& cell : m_cells)
		size += cell.getSize();
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



gg::Database::RowView::RowView(Row& row, bool write_access) :
	m_row(row),
	m_access(write_access ? AccessType::READ_WRITE : AccessType::READ)
{
	std::lock_guard<decltype(m_row.m_mutex)> guard(m_row.m_mutex);

	if (m_access == AccessType::READ_WRITE)
	{
		if (m_row.m_reader_views > 0)
			m_access = AccessType::READ;
		else if (m_row.m_writer_views > 0)
			m_access = AccessType::NO_ACCESS;
		else
			++m_row.m_writer_views;
	}
	
	if (m_access == AccessType::READ)
	{
		if (m_row.m_writer_views > 0)
			m_access = AccessType::NO_ACCESS;
		else
			++m_row.m_reader_views;
	}
}

gg::Database::RowView::~RowView()
{
	std::lock_guard<decltype(m_row.m_mutex)> guard(m_row.m_mutex);

	if (m_access == AccessType::READ_WRITE)
		--m_row.m_writer_views;
	else if (m_access == AccessType::READ)
		--m_row.m_reader_views;

	// remove row if we are the last viewers and it's marked to be removed
	if (m_row.m_writer_views == 0 && m_row.m_reader_views == 0
		&& m_row.m_force_remove)
	{
		m_row.m_table.removeRow(m_row.m_key);
	}
}

gg::IDatabase::AccessType gg::Database::RowView::getAccessType() const
{
	return m_access;
}

gg::IDatabase::Key gg::Database::RowView::getKey() const
{
	return m_row.m_key;
}

gg::IDatabase::ICell* gg::Database::RowView::cell(unsigned column)
{
	if (m_access != AccessType::READ_WRITE)
		throw AccessError(AccessType::READ_WRITE, m_access);

	return m_row.cell(column);
}

gg::IDatabase::ICell* gg::Database::RowView::cell(const std::string& column)
{
	if (m_access != AccessType::READ_WRITE)
		throw AccessError(AccessType::READ_WRITE, m_access);

	return m_row.cell(column);
}

const gg::IDatabase::ICell* gg::Database::RowView::cell(unsigned column) const
{
	if (m_access == AccessType::NO_ACCESS)
		throw AccessError(AccessType::READ, m_access);

	return m_row.cell(column);
}

const gg::IDatabase::ICell* gg::Database::RowView::cell(const std::string& column) const
{
	if (m_access == AccessType::NO_ACCESS)
		throw AccessError(AccessType::READ, m_access);

	return m_row.cell(column);
}

void gg::Database::RowView::remove()
{
	m_row.remove();
}



gg::Database::Table::Table(Database& database) :
	m_database(database),
	m_last_row_key(0),
	m_writer_views(0),
	m_reader_views(0),
	m_force_remove(false)
{
}

gg::Database::Table::Table(Database& database, const std::string& name, const std::vector<std::string>& columns) :
	m_database(database),
	m_name(name),
	m_last_row_key(0),
	m_writer_views(0),
	m_reader_views(0),
	m_force_remove(false)
{
	m_columns.insert(m_columns.end(), columns.begin(), columns.end());
}

gg::Database::Table::Table(Table&& table) :
	m_database(table.m_database),
	m_name(std::move(table.m_name)),
	m_columns(std::move(table.m_columns)),
	m_last_row_key(table.m_last_row_key),
	m_writer_views(0),
	m_reader_views(0),
	m_force_remove(false)
{
}

gg::IDatabase::AccessType gg::Database::Table::getAccessType() const
{
	return AccessType::NO_ACCESS;
}

const std::string& gg::Database::Table::getName() const
{
	return m_name;
}

std::shared_ptr<gg::IDatabase::IRow> gg::Database::Table::createAndGetRow(bool write_access)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	++m_last_row_key;

	auto it = m_rows.emplace(m_last_row_key, Row{ *this, m_last_row_key });
	if (it.second) // successful insert
		return (it.first->second).createView(write_access);
	else
		return {};
}

std::shared_ptr<gg::IDatabase::IRow> gg::Database::Table::getRow(Key key, bool write_access)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	auto it = m_rows.find(key);
	if (it != m_rows.end())
		return it->second.createView(write_access);
	else
		return {};
}

std::shared_ptr<gg::IDatabase::IRow> gg::Database::Table::getNextRow(Key key, bool write_access)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	auto it = m_rows.upper_bound(key);
	if (it != m_rows.end())
		return it->second.createView(write_access);
	else
		return{};
}

void gg::Database::Table::remove()
{
	m_force_remove = true;
}

void gg::Database::Table::removeRow(Key key)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	m_rows.erase(key);
}

std::shared_ptr<gg::IDatabase::ITable> gg::Database::Table::createView(bool write_access)
{
	if (m_force_remove)
		return {};
	else
		return std::shared_ptr<gg::IDatabase::ITable>(new TableView(*this, write_access));
}

size_t gg::Database::Table::getSize() const
{
	size_t size = sizeOfString(m_name);

	size += sizeof(uint16_t); // column_count
	for (const std::string& column : m_columns)
		size += sizeOfString(column);

	size += sizeof(uint16_t); // row_count
	for (auto& row : m_rows)
		size += row.second.getSize();

	return size;
}

void gg::Database::Table::save(std::fstream& f) const
{
	saveString(m_name, f);

	uint16_t column_count = static_cast<uint16_t>(m_columns.size());
	f.write(reinterpret_cast<const char*>(&column_count), sizeof(uint16_t));
	for (const std::string& column : m_columns)
		saveString(column, f);

	uint16_t row_count = static_cast<uint16_t>(m_rows.size());
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

	uint16_t row_count = 0;
	f.read(reinterpret_cast<char*>(&row_count), sizeof(uint16_t));
	for (uint16_t i = 0; i < row_count; ++i)
	{
		Row row(*this, 0);
		row.load(f);
		m_rows.emplace(row.getKey(), std::move(row));
	}
}



gg::Database::TableView::TableView(Table& table, bool write_access) :
	m_table(table),
	m_access(write_access ? AccessType::READ_WRITE : AccessType::READ)
{
	std::lock_guard<decltype(m_table.m_mutex)> guard(m_table.m_mutex);

	if (m_access == AccessType::READ_WRITE)
	{
		if (m_table.m_reader_views > 0)
			m_access = AccessType::READ;
		else if (m_table.m_writer_views > 0)
			m_access = AccessType::NO_ACCESS;
		else
			++m_table.m_writer_views;
	}

	if (m_access == AccessType::READ)
	{
		if (m_table.m_writer_views > 0)
			m_access = AccessType::NO_ACCESS;
		else
			++m_table.m_reader_views;
	}
}

gg::Database::TableView::~TableView()
{
	std::lock_guard<decltype(m_table.m_mutex)> guard(m_table.m_mutex);

	if (m_access == AccessType::READ_WRITE)
		--m_table.m_writer_views;
	else if (m_access == AccessType::READ)
		--m_table.m_reader_views;

	// remove row if we are the last viewers and it's marked to be removed
	if (m_table.m_writer_views == 0 && m_table.m_reader_views == 0
		&& m_table.m_force_remove)
	{
		m_table.m_database.removeTable(m_table.m_name);
	}
}

gg::IDatabase::AccessType gg::Database::TableView::getAccessType() const
{
	return m_access;
}

const std::string& gg::Database::TableView::getName() const
{
	return m_table.m_name;
}

std::shared_ptr<gg::IDatabase::IRow> gg::Database::TableView::createAndGetRow(bool write_access)
{
	if (m_access != AccessType::READ_WRITE)
		throw AccessError(AccessType::READ_WRITE, m_access);

	return m_table.createAndGetRow(write_access);
}

std::shared_ptr<gg::IDatabase::IRow> gg::Database::TableView::getRow(Key key, bool write_access)
{
	if (write_access)
	{
		if (m_access != AccessType::READ_WRITE)
			throw AccessError(AccessType::READ_WRITE, m_access);
	}
	else
	{
		if (m_access == AccessType::NO_ACCESS)
			throw AccessError(AccessType::READ, m_access);
	}

	return m_table.getRow(key, write_access);
}

std::shared_ptr<gg::IDatabase::IRow> gg::Database::TableView::getNextRow(Key key, bool write_access)
{
	if (write_access)
	{
		if (m_access != AccessType::READ_WRITE)
			throw AccessError(AccessType::READ_WRITE, m_access);
	}
	else
	{
		if (m_access == AccessType::NO_ACCESS)
			throw AccessError(AccessType::READ, m_access);
	}

	return m_table.getNextRow(key, write_access);
}

void gg::Database::TableView::remove()
{
	m_table.remove();
}



gg::Database::Database(const std::string& name)
{

}

const std::string& gg::Database::getFilename() const
{
	return m_filename;
}

std::shared_ptr<gg::IDatabase::ITable> gg::Database::createAndGetTable(const std::string& name, const std::vector<std::string>& columns, bool write_access)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	auto it = m_tables.emplace(name, Table{ *this, name, columns });
	if (it.second) // successful insert
		return (it.first->second).createView(write_access);
	return {};
}

std::shared_ptr<gg::IDatabase::ITable> gg::Database::createAndGetTable(const std::string& name, unsigned columns, bool write_access)
{
	return createAndGetTable(name, std::vector<std::string>(columns), write_access);
}

std::shared_ptr<gg::IDatabase::ITable> gg::Database::getTable(const std::string& name, bool write_access)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	auto it = m_tables.find(name);
	if (it != m_tables.end())
		return it->second.createView(write_access);
	else
		return {};
}

void gg::Database::getTableNames(std::vector<std::string>& tables) const
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	for (auto& it : m_tables)
		tables.push_back(it.first);
}

bool gg::Database::sync()
{
	// TBD: backup

	try
	{
		save(m_file);
	}
	catch (std::exception&)
	{
		return false;
	}

	return true;
}

void gg::Database::removeTable(const std::string& table)
{

}

void gg::Database::save(std::fstream& f) const
{
}

void gg::Database::load(std::fstream& f)
{
}



std::shared_ptr<gg::IDatabase> gg::DatabaseManager::open(const std::string& filename) const
{
	return std::shared_ptr<gg::IDatabase>(new Database(filename));
}
