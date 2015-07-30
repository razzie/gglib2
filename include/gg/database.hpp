/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#if defined GGDATABASE_BUILD
#	define GG_API __declspec(dllexport)
#else
#	define GG_API __declspec(dllimport)
#endif

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace gg
{
	class IDatabase
	{
	public:
		typedef uint16_t Key;

		enum Access
		{
			NO_ACCESS,
			READ,
			READ_WRITE
		};

		class ICell
		{
		public:
			enum Type : uint16_t
			{
				NONE,
				INT32,
				INT64,
				FLOAT,
				DOUBLE,
				STRING
			};

			virtual ~ICell() = default;
			virtual Type getType() const = 0;
			virtual int32_t getInt32() const = 0;
			virtual int64_t getInt64() const = 0;
			virtual float getFloat() const = 0;
			virtual double getDouble() const = 0;
			virtual std::string getString() const = 0;
			virtual void set(int32_t) = 0;
			virtual void set(int64_t) = 0;
			virtual void set(float) = 0;
			virtual void set(double) = 0;
			virtual void set(const std::string&) = 0;
		};

		class IRow
		{
		public:
			virtual ~IRow() = default;
			virtual Access access() const = 0;
			virtual Key key() const = 0;
			virtual ICell& cell(unsigned) = 0;
			virtual ICell& cell(const std::string& cell_name) = 0;
			virtual const ICell& cell(unsigned) const = 0;
			virtual const ICell& cell(const std::string& cell_name) const = 0;
			virtual void remove() = 0; // removes row after it's not referenced anywhere
		};

		class ITable
		{
		public:
			virtual ~ITable() = default;
			virtual Access access() const = 0;
			virtual const std::string& name() const = 0;
			virtual std::shared_ptr<IRow> createRow(Key) = 0;
			virtual std::shared_ptr<IRow> row(Key, Access = Access::READ_WRITE) = 0;
			virtual void sync() = 0;
			virtual void remove() = 0; // removes table after it's not referenced anywhere
		};

		virtual ~IDatabase() = default;
		virtual const std::string& name() const = 0;
		virtual bool createTable(const std::string& name, const std::vector<std::string>& columns) = 0;
		virtual bool createTable(const std::string& name, unsigned columns) = 0;
		virtual std::shared_ptr<ITable> table(const std::string&, Access = Access::READ_WRITE) = 0;
	};

	class IDatabaseManager
	{
	public:
		virtual ~IDatabaseManager() = default;
		virtual std::shared_ptr<IDatabase> open(const std::string& name) const = 0;
	};

	extern GG_API IDatabaseManager& db;
};
