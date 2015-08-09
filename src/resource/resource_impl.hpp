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
#include "gg/resource.hpp"

namespace gg
{
	class Resource
	{
	public:
		Resource(const std::string& vdir_path);
		virtual ~Resource();
		virtual bool init();
		virtual const std::string& getName() const;
		virtual DirectoryPtr getDirectory(const std::string& dir_name);
		virtual FilePtr getFile(const std::string& file_name);
		virtual bool loadDirectoryData(const std::string& dir_name, std::vector<IDirectory::FileOrDirectory>*);
		virtual bool loadFileData(const std::string& file_name, const char**, size_t*);

	private:
		struct FileData
		{
			std::string file_name;
			size_t start_pos;
			size_t original_size = 0;
			size_t compressed_size = 0;
			mutable std::mutex ptr_mutex;
			std::weak_ptr<IFile> ptr;
		};

		std::ifstream m_file;
		std::string m_name;
		std::map<std::string, FileData> m_files;
	};

	class ResourceManager : public IResourceManager
	{
	public:
		ResourceManager();
		virtual ~ResourceManager();
		virtual bool createResource(const std::string& dir_path) const;
		virtual bool createResource(const std::wstring& dir_path) const;
		virtual bool addResource(const std::string& vdir_path);
		virtual DirectoryPtr openDirectory(const std::string& dir_name);
		virtual FilePtr openFile(const std::string& file_name);

	private:
		std::map<std::string, Resource*> m_resources;
	};

	class Directory : public IDirectory
	{
	public:
		Directory(Resource* res, std::string& name) :
			m_name(name), m_res(res)
		{
			m_res->loadDirectoryData(m_name.substr(m_name.find('/') + 1), &m_files);
		}

		virtual const std::string& getName() const
		{
			return m_name;
		}

		virtual Iterator begin()
		{
			return m_files.begin();
		}

		virtual Iterator end()
		{
			return m_files.end();
		}

		virtual ConstIterator begin() const
		{
			return m_files.begin();
		}

		virtual ConstIterator end() const
		{
			return m_files.end();
		}

	private:
		std::string m_name;
		std::vector<FileOrDirectory> m_files;
		Resource* m_res;
	};

	class File : public IFile
	{
	public:
		File(Resource* res, const std::string& name) :
			m_name(name), m_data(nullptr), m_size(0), m_res(res)
		{
		}

		virtual ~File()
		{
			unload();
		}

		virtual const std::string& getName() const
		{
			return m_name;
		}

		virtual const char* getData() const
		{
			if (m_data == nullptr)
			{
				std::lock_guard<decltype(m_mutex)> guard(m_mutex);

				if (m_data == nullptr)
					m_res->loadFileData(m_name.substr(m_name.find('/') + 1), &m_data, &m_size);
			}

			return m_data;
		}

		virtual size_t getSize() const
		{
			if (m_data == nullptr)
			{
				std::lock_guard<decltype(m_mutex)> guard(m_mutex);

				if (m_data == nullptr)
					m_res->loadFileData(m_name.substr(m_name.find('/') + 1), &m_data, &m_size);
			}

			return m_size;
		}

		virtual void unload()
		{
			std::lock_guard<decltype(m_mutex)> guard(m_mutex);

			if (m_data != nullptr)
			{
				delete[] m_data;
				m_data = nullptr;
				m_size = 0;
			}
		}

	private:
		mutable std::mutex m_mutex;
		std::string m_name;
		mutable const char* m_data;
		mutable size_t m_size;
		Resource* m_res;
	};
};
