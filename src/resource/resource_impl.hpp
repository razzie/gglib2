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
	class Resource;
	typedef std::shared_ptr<Resource> ResourcePtr;

	class Resource
	{
	public:
		static ResourcePtr create(const std::string& res_path);

		virtual ~Resource();
		virtual const std::string& getName() const;
		virtual DirectoryPtr getDirectory(const std::string& dir_name) const;
		virtual FilePtr getFile(const std::string& file_name) const;
		virtual bool loadDirectoryData(const std::string& dir_name, std::vector<IDirectory::FileOrDirectory>*);
		virtual bool loadFileData(const std::string& file_name, const char**, size_t*);

	private:
		Resource(const std::string& res_path);
		virtual bool init();

		struct FileData
		{
			std::string file_name;
			size_t start_pos;
			size_t original_size = 0;
			size_t compressed_size = 0;
			mutable std::mutex ptr_mutex;
			mutable std::weak_ptr<IFile> ptr;
		};

		std::weak_ptr<Resource> m_self_ptr;
		std::ifstream m_file;
		std::string m_name;
		std::map<std::string, FileData> m_files;
	};

	class ResourcePool : public IResourcePool
	{
	public:
		ResourcePool();
		virtual ~ResourcePool();
		virtual bool includeResource(const std::string& res_path);
		virtual void releaseResources();
		virtual DirectoryPtr openDirectory(const std::string& dir_name) const;
		virtual FilePtr openFile(const std::string& file_name) const;

	private:
		mutable std::mutex m_mutex;
		std::map<std::string, std::shared_ptr<Resource>> m_resources;
	};

	class ResourceCreator : public IResourceCreator
	{
	public:
		static ResourceCreatorPtr create(const std::string& res_path, bool append_mode = false);
		static ResourceCreatorPtr create(const std::wstring& res_path, bool append_mode = false);

		virtual ~ResourceCreator();
		virtual bool addFile(const std::string& file_path, const std::string& res_file_name);
		virtual bool addDirectory(const std::string& dir_path);
		virtual bool addFile(const std::wstring& file_path, const std::string& res_file_name);
		virtual bool addDirectory(const std::wstring& dir_path);

	private:
		ResourceCreator(const std::string& res_path, bool append_mode);
		ResourceCreator(const std::wstring& res_path, bool append_mode);
		virtual bool init();
		bool collectFiles(std::wstring dir_name, std::vector<std::wstring>& files);

		mutable std::mutex m_mutex;
		std::ofstream m_file;
	};

	class ResourceManager : public IResourceManager
	{
	public:
		ResourceManager();
		virtual ~ResourceManager();
		virtual ResourcePoolPtr createResourcePool() const;
		virtual ResourcePoolPtr getDefaultResourcePool();
		virtual ResourceCreatorPtr createResource(const std::string& res_path, bool append_mode = false) const;
		virtual ResourceCreatorPtr createResource(const std::wstring& res_path, bool append_mode = false) const;

	private:
		mutable std::mutex m_mutex;
		ResourcePoolPtr m_default_res_pool;
	};

	class Directory : public IDirectory
	{
	public:
		Directory(ResourcePtr res, std::string& name) :
			m_name(name), m_res(res)
		{
			m_res->loadDirectoryData(m_name.substr(m_name.find('/') + 1), &m_files);
		}

		virtual const std::string& getName() const
		{
			return m_name;
		}

		virtual Iterator begin() const
		{
			return m_files.begin();
		}

		virtual Iterator end() const
		{
			return m_files.end();
		}

	private:
		std::string m_name;
		std::vector<FileOrDirectory> m_files;
		ResourcePtr m_res;
	};

	class File : public IFile
	{
	public:
		File(ResourcePtr res, const std::string& name) :
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
		ResourcePtr m_res;
	};
};
