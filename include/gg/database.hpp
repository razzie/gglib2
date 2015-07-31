/**
 * Copyright (c) 2014-2015 G�bor G�rzs�ny (www.gorzsony.com)
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

		enum AccessType
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
			virtual AccessType getAccessType() const = 0;
			virtual Key getKey() const = 0;
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
			virtual AccessType getAccessType() const = 0;
			virtual const std::string& getName() const = 0;
			virtual std::shared_ptr<IRow> createRow() = 0;
			virtual std::shared_ptr<IRow> getRow(Key, bool write = true) = 0;
			virtual std::shared_ptr<IRow> getNextRow(Key, bool write = true) = 0;
			virtual void sync() = 0;
			virtual void remove() = 0; // removes table after it's not referenced anywhere
		};

		virtual ~IDatabase() = default;
		virtual const std::string& getFilename() const = 0;
		virtual bool createTable(const std::string& table, const std::vector<std::string>& columns) = 0;
		virtual bool createTable(const std::string& table, unsigned columns) = 0;
		virtual std::shared_ptr<ITable> getTable(const std::string& table, bool write = true) = 0;
		virtual void getTableNames(std::vector<std::string>& tables) const = 0;
	};

	class IDatabaseManager
	{
	public:
		virtual ~IDatabaseManager() = default;
		virtual std::shared_ptr<IDatabase> open(const std::string& filename) const = 0;
	};

	extern GG_API IDatabaseManager& db;
};
