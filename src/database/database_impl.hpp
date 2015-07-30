/**
 * Copyright (c) 2014-2015 G�bor G�rzs�ny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <map>
#include <mutex>
#include "gg/database.hpp"

namespace gg
{
	class Database : public IDatabase
	{
	public:
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

		private:
			union Data
			{
				int32_t i32;
				int64_t i64;
				float f;
				double d;
			};

			std::mutex m_mutex;
			Type m_type;
			Data m_data;
			std::string m_str_data;
		};

		class Row : public IRow
		{
		public:
			friend class Cell;

			Row();
			virtual ~Row() = default;
			virtual Access access() const;
			virtual Key key() const;
			virtual ICell& cell(unsigned);
			virtual ICell& cell(const std::string& cell_name);
			virtual const ICell& cell(unsigned) const;
			virtual const ICell& cell(const std::string& cell_name) const;
			virtual void remove();

		private:
			Key m_key;
			std::vector<Cell> m_cells;
			bool m_force_remove;
		};

		class RowView : public IRow
		{
		};

		class Table : public ITable
		{
		public:
			friend class Row;

			Table();
			virtual ~Table() = default;
			virtual Access access() const;
			virtual const std::string& name() const;
			virtual std::shared_ptr<IRow> createRow(Key);
			virtual std::shared_ptr<IRow> row(Key, Access = Access::READ_WRITE);
			virtual void remove();

		private:
			std::mutex m_mutex;
			std::vector<std::string> m_columns;
			std::map<Key, Row> m_rows;
			bool m_force_remove;
		};

		class TableView : public ITable
		{
		};

		virtual ~Database() = default;
		virtual const std::string& name() const;
		virtual bool createTable(const std::string& name, const std::vector<std::string>& columns);
		virtual std::shared_ptr<ITable> table(const std::string&, Access = Access::READ_WRITE);
	};

	class DatabaseManager : public IDatabaseManager
	{
	public:
		DatabaseManager() = default;
		virtual ~DatabaseManager() = default;
		virtual std::shared_ptr<IDatabase> open(const std::string& name) const;
	};
};
