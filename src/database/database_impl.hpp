/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <fstream>
#include <map>
#include <mutex>
#include "archive_impl.hpp"
#include "gg/database.hpp"

namespace gg
{
	class Database : public IDatabase
	{
	public:
		class AccessError : public IAccessError
		{
		public:
			AccessError(AccessType requested, AccessType actual);
			virtual ~AccessError() = default;
			virtual const char* what();
			virtual AccessType getRequestedAccess() const;
			virtual AccessType getActualAccess() const;

		private:
			AccessType m_requested;
			AccessType m_actual;
		};

		class Cell : public ICell
		{
		public:
			Cell();
			Cell(Cell&&);
			virtual ~Cell() = default;
			virtual Type getType() const;
			virtual int32_t getInt32() const;
			virtual int64_t getInt64() const;
			virtual float getFloat() const;
			virtual double getDouble() const;
			virtual std::string getString() const;
			virtual void set(int32_t);
			virtual void set(int64_t);
			virtual void set(float);
			virtual void set(double);
			virtual void set(const std::string&);
			virtual void serialize(IArchive&);

		private:
			union Data
			{
				int32_t i32;
				int64_t i64;
				float f;
				double d;
			};

			mutable std::mutex m_mutex;
			Type m_type;
			Data m_data;
			std::string m_str_data;
		};

		class Table;
		class RowView;

		class Row : public IRow
		{
		public:
			Row(Table&, Key);
			Row(Row&&);
			virtual ~Row() = default;
			virtual AccessType getAccessType() const;
			virtual Key getKey() const;
			virtual ICell* cell(unsigned);
			virtual ICell* cell(const std::string& cell_name);
			virtual const ICell* cell(unsigned) const;
			virtual const ICell* cell(const std::string& cell_name) const;
			virtual void remove();
			virtual void serialize(IArchive&);

			RowPtr createView(bool write);

		private:
			friend class Cell;
			friend class RowView;
			friend class Table;

			mutable std::recursive_mutex m_mutex;
			Table* m_table;
			Key m_key;
			std::vector<Cell> m_cells;
			unsigned m_writer_views;
			unsigned m_reader_views;
			volatile bool m_force_remove;
		};

		class RowView : public IRow
		{
		public:
			RowView(Row&, bool write);
			virtual ~RowView();
			virtual AccessType getAccessType() const;
			virtual Key getKey() const;
			virtual ICell* cell(unsigned column);
			virtual ICell* cell(const std::string& column);
			virtual const ICell* cell(unsigned column) const;
			virtual const ICell* cell(const std::string& column) const;
			virtual void remove();
			virtual void serialize(IArchive&);

		private:
			Row& m_row;
			AccessType m_access;
			DatabasePtr m_database;
		};

		class TableView;

		class Table : public ITable
		{
		public:
			Table(Database&);
			Table(Database&, const std::string& name, const std::vector<std::string>& columns);
			Table(Table&&);
			virtual ~Table() = default;
			virtual AccessType getAccessType() const;
			virtual const std::string& getName() const;
			virtual RowPtr createAndGetRow(bool write_access = true);
			virtual RowPtr getRow(Key, bool write_access = true);
			virtual RowPtr getNextRow(Key, bool write_access = true);
			virtual void remove();
			virtual void serialize(IArchive&);

			void removeRow(Key);
			TablePtr createView(bool write_access);

		private:
			friend class Row;
			friend class RowView;
			friend class TableView;

			mutable std::recursive_mutex m_mutex;
			Database* m_database;
			std::string m_name;
			std::vector<std::string> m_columns;
			std::map<Key, Row> m_rows;
			Key m_last_row_key;
			unsigned m_writer_views;
			unsigned m_reader_views;
			volatile bool m_force_remove;
		};

		class TableView : public ITable
		{
		public:
			TableView(Table&, bool write);
			virtual ~TableView();
			virtual AccessType getAccessType() const;
			virtual const std::string& getName() const;
			virtual RowPtr createAndGetRow(bool write_access = true);
			virtual RowPtr getRow(Key, bool write_access = true);
			virtual RowPtr getNextRow(Key, bool write_access = true);
			virtual void remove();
			virtual void serialize(IArchive&);

		private:
			Table& m_table;
			AccessType m_access;
			DatabasePtr m_database;
		};

		Database(const std::string& filename);
		virtual ~Database() = default;
		virtual const std::string& getFilename() const;
		virtual TablePtr createAndGetTable(const std::string& table, const std::vector<std::string>& columns, bool write_access = true);
		virtual TablePtr createAndGetTable(const std::string& table, unsigned columns, bool write_access = true);
		virtual TablePtr getTable(const std::string& table, bool write = true);
		virtual void getTableNames(std::vector<std::string>& tables) const;
		virtual bool save();
		virtual void serialize(IArchive&);

		void removeTable(const std::string&);

	private:
		friend class DatabaseManager;

		mutable std::recursive_mutex m_mutex;
		std::string m_filename;
		std::map<std::string, Table> m_tables;
		std::weak_ptr<IDatabase> m_self_ptr;
	};

	class DatabaseManager : public IDatabaseManager
	{
	public:
		DatabaseManager() = default;
		virtual ~DatabaseManager() = default;
		virtual DatabasePtr open(const std::string& filename) const;
	};

	class FileArchive : public Archive
	{
	public:
		FileArchive(const std::string& file, Mode);
		virtual ~FileArchive();
		virtual size_t write(const char* ptr, size_t len);
		virtual size_t read(char* ptr, size_t len);
		operator bool() const;

	private:
		std::fstream m_file;
	};
};
