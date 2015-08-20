/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifdef _WIN32
#	include <Windows.h>
#endif

#include <stdexcept>
#include "database_impl.hpp"

static gg::DatabaseManager s_db;
gg::IDatabaseManager& gg::db = s_db;


template<class T>
void serialize(gg::IArchive& ar, std::vector<T>& v)
{
	uint16_t items;

	if (ar.getMode() == gg::IArchive::Mode::SERIALIZE)
	{
		items = static_cast<uint16_t>(v.size());
		ar & items;
		for (auto& i : v)
			ar & i;
	}
	else
	{
		ar & items;
		for (uint16_t i = 0; i < items; ++i)
		{
			v.emplace_back();
			ar & v.back();
		}
	}
}

/*template<class Key, class Value>
void serialize(gg::IArchive& ar, std::map<Key, Value>& m)
{
	uint16_t items;

	if (ar.getMode() == gg::IArchive::Mode::SERIALIZE)
	{
		items = static_cast<uint16_t>(m.size());
		ar & items;
		for (auto& i : m)
			ar & i.first & i.second;
	}
	else
	{
		ar & items;
		for (uint16_t i = 0; i < items; ++i)
		{
			Key key;
			Value value;
			ar & key & value;
			m.emplace(key, value);
		}
	}
}*/

void serialize(gg::IArchive& ar, gg::IDatabase::ICell::Type& type)
{
	uint16_t& v = reinterpret_cast<uint16_t&>(type);
	ar & v;
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

void gg::Database::Cell::serialize(IArchive& ar)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	ar & m_type;

	switch (m_type)
	{
	case Type::INT32:
		ar & m_data.i32;
		break;
	case Type::INT64:
		ar & m_data.i64;
		break;
	case Type::FLOAT:
		ar & m_data.f;
		break;
	case Type::DOUBLE:
		ar & m_data.d;
		break;
	case Type::STRING:
		ar & m_str_data;
		break;

	default:
		break;
	}
}



gg::Database::Row::Row(Table& table, Key key) :
	m_table(&table),
	m_key(key),
	m_writer_views(0),
	m_reader_views(0),
	m_force_remove(false)
{
	m_cells.resize(m_table->m_columns.size());
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
	for (size_t i = 0, len = m_table->m_columns.size(); i < len; ++i)
	{
		if (m_table->m_columns[i] == column)
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
	for (size_t i = 0, len = m_table->m_columns.size(); i < len; ++i)
	{
		if (m_table->m_columns[i] == column)
			return &m_cells[i];
	}

	return nullptr;
}

void gg::Database::Row::remove()
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	m_force_remove = true;
}

void gg::Database::Row::serialize(IArchive& ar)
{
	ar & m_key;
	for (Cell& cell : m_cells)
		cell.serialize(ar);
}

std::shared_ptr<gg::IDatabase::IRow> gg::Database::Row::createView(bool write_access)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	if (m_force_remove)
		return {};
	else
		return std::shared_ptr<gg::IDatabase::IRow>(new RowView(*this, write_access));
}



gg::Database::RowView::RowView(Row& row, bool write_access) :
	m_row(row),
	m_access(write_access ? AccessType::READ_WRITE : AccessType::READ),
	m_database(row.m_table->m_database->m_self_ptr.lock())
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
	if (m_access == AccessType::NO_ACCESS)
		return;

	std::lock_guard<decltype(m_row.m_mutex)> guard(m_row.m_mutex);

	if (m_access == AccessType::READ_WRITE)
		--m_row.m_writer_views;
	else if (m_access == AccessType::READ)
		--m_row.m_reader_views;

	// remove row if we are the last viewers and it's marked to be removed
	if (m_row.m_writer_views == 0 && m_row.m_reader_views == 0
		&& m_row.m_force_remove)
	{
		m_row.m_table->removeRow(m_row.m_key);
	}
}

gg::IDatabase::AccessType gg::Database::RowView::getAccessType() const
{
	return m_access;
}

gg::IDatabase::Key gg::Database::RowView::getKey() const
{
	if (m_access == AccessType::NO_ACCESS)
		throw AccessError(AccessType::READ, m_access);

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
	if (m_access != AccessType::READ_WRITE)
		throw AccessError(AccessType::READ_WRITE, m_access);

	m_row.remove();
}

void gg::Database::RowView::serialize(IArchive& ar)
{
	if (ar.getMode() == IArchive::Mode::SERIALIZE && m_access == AccessType::NO_ACCESS)
		throw AccessError(AccessType::READ, m_access);

	if (ar.getMode() == IArchive::Mode::DESERIALIZE && m_access != AccessType::READ_WRITE)
		throw AccessError(AccessType::READ_WRITE, m_access);

	m_row.serialize(ar);
}



gg::Database::Table::Table(Database& database) :
	m_database(&database),
	m_last_row_key(0),
	m_writer_views(0),
	m_reader_views(0),
	m_force_remove(false)
{
}

gg::Database::Table::Table(Database& database, const std::string& name, const std::vector<std::string>& columns) :
	m_database(&database),
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
	m_rows(std::move(table.m_rows)),
	m_last_row_key(table.m_last_row_key),
	m_writer_views(0),
	m_reader_views(0),
	m_force_remove(false)
{
	for (auto& it : m_rows)
		it.second.m_table = this;
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

	std::shared_ptr<IRow> row;

	do
	{
		auto it = m_rows.upper_bound(key);
		if (it != m_rows.end())
		{
			row = it->second.createView(write_access);
			if (!row)
				key = it->second.getKey();
		}
		else
		{
			return {};
		}
	} while (!row);

	return row;
}

void gg::Database::Table::remove()
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	m_force_remove = true;
}

void gg::Database::Table::removeRow(Key key)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	m_rows.erase(key);
}

std::shared_ptr<gg::IDatabase::ITable> gg::Database::Table::createView(bool write_access)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	if (m_force_remove)
		return {};
	else
		return std::shared_ptr<gg::IDatabase::ITable>(new TableView(*this, write_access));
}

void gg::Database::Table::serialize(IArchive& ar)
{
	ar & m_name & m_columns;

	uint16_t row_count;
	if (ar.getMode() == IArchive::Mode::DESERIALIZE)
	{
		ar & row_count;
		for (uint16_t i = 0; i < row_count; ++i)
		{
			Row row(*this, 0);
			row.serialize(ar);
			m_rows.emplace(row.getKey(), std::move(row));
		}
	}
	else
	{
		row_count = static_cast<uint16_t>(m_rows.size());
		ar & row_count;
		for (auto& it : m_rows)
		{
			it.second.serialize(ar);
		}
	}
}



gg::Database::TableView::TableView(Table& table, bool write_access) :
	m_table(table),
	m_access(write_access ? AccessType::READ_WRITE : AccessType::READ),
	m_database(table.m_database->m_self_ptr.lock())
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
	if (m_access == AccessType::NO_ACCESS)
		return;

	std::lock_guard<decltype(m_table.m_mutex)> guard(m_table.m_mutex);

	if (m_access == AccessType::READ_WRITE)
		--m_table.m_writer_views;
	else if (m_access == AccessType::READ)
		--m_table.m_reader_views;

	// remove row if we are the last viewers and it's marked to be removed
	if (m_table.m_writer_views == 0 && m_table.m_reader_views == 0
		&& m_table.m_force_remove)
	{
		m_table.m_database->removeTable(m_table.m_name);
	}
}

gg::IDatabase::AccessType gg::Database::TableView::getAccessType() const
{
	return m_access;
}

const std::string& gg::Database::TableView::getName() const
{
	if (m_access == AccessType::NO_ACCESS)
		throw AccessError(AccessType::READ, m_access);

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
	if (m_access != AccessType::READ_WRITE)
		throw AccessError(AccessType::READ_WRITE, m_access);

	m_table.remove();
}

void gg::Database::TableView::serialize(IArchive& ar)
{
	if (ar.getMode() == IArchive::Mode::SERIALIZE && m_access == AccessType::NO_ACCESS)
		throw AccessError(AccessType::READ, m_access);

	if (ar.getMode() == IArchive::Mode::DESERIALIZE && m_access != AccessType::READ_WRITE)
		throw AccessError(AccessType::READ_WRITE, m_access);

	m_table.serialize(ar);
}



gg::Database::Database(const std::string& filename) :
	m_filename(filename)
{
	FileArchive ar(filename, IArchive::Mode::DESERIALIZE);
	if (ar)
	{
		serialize(ar);
	}
	else
	{
		// if trying to read from non-existing file, create one
		std::fstream file(filename, std::ios::out | std::ios::binary);
		if (!file.is_open())
			throw std::runtime_error("Cannot open or create database: " + filename);
	}
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

bool gg::Database::save()
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

#ifdef _WIN32
	std::string backup = m_filename + ".backup";
	if (CopyFileA(m_filename.c_str(), backup.c_str(), FALSE) == 0)
		return false;
#endif // _WIN32

	try
	{
		FileArchive ar(m_filename, IArchive::Mode::SERIALIZE);
		if (!ar)
			return false;

		serialize(ar);
	}
	catch (std::exception&)
	{
#ifdef _WIN32
		MoveFileExA(backup.c_str(), m_filename.c_str(), MOVEFILE_REPLACE_EXISTING);
#endif
		return false;
	}

#ifdef _WIN32
	DeleteFileA(backup.c_str());
#endif
	return true;
}

void gg::Database::removeTable(const std::string& table)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	m_tables.erase(table);
}

void gg::Database::serialize(IArchive& ar)
{
	uint16_t table_count;
	if (ar.getMode() == IArchive::Mode::DESERIALIZE)
	{
		try
		{
			ar & table_count;
			for (uint16_t i = 0; i < table_count; ++i)
			{
				Table table(*this);
				table.serialize(ar);
				m_tables.emplace(table.getName(), std::move(table));
			}
		}
		catch (ISerializationError&)
		{
			m_tables.clear();
		}
	}
	else
	{
		table_count = static_cast<uint16_t>(m_tables.size());
		ar & table_count;
		for (auto& it : m_tables)
		{
			it.second.serialize(ar);
		}
	}
}



std::shared_ptr<gg::IDatabase> gg::DatabaseManager::open(const std::string& filename) const
{
	try
	{
		std::shared_ptr<gg::Database> db(new Database(filename));
		db->m_self_ptr = db;
		return db;
	}
	catch (std::exception&)
	{
		return {};
	}
}




gg::FileArchive::FileArchive(const std::string& file, Mode mode) :
	Archive(mode)
{
	std::ios::openmode flags = std::ios::binary;

	if (mode == Mode::SERIALIZE) // write
		flags |= std::ios::out | std::ios::trunc;
	else // read
		flags |= std::ios::in;

	m_file.open(file, flags);
}

gg::FileArchive::~FileArchive()
{
	if (m_file.is_open())
		m_file.close();
}

size_t gg::FileArchive::write(const char* ptr, size_t len)
{
	m_file.write(ptr, len);
	return len;
}

size_t gg::FileArchive::read(char* ptr, size_t len)
{
	//return static_cast<size_t>(m_file.readsome(ptr, len));
	auto start_pos = m_file.tellg();
	m_file.read(ptr, len);
	auto end_pos = m_file.tellg();
	return static_cast<size_t>(end_pos - start_pos);
}

gg::FileArchive::operator bool() const
{
	return (m_file.good() && m_file.is_open());
}
