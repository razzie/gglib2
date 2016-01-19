/**
 * Copyright (c) 2014-2016 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once
#pragma warning (disable : 4250)

#include <cstring>
#include <fstream>
#include <map>
#include <mutex>
#include "stream_impl.hpp"
#include "gg/resource.hpp"

namespace gg
{
	class Resource;
	typedef std::shared_ptr<Resource> ResourcePtr;

	typedef std::shared_ptr<std::vector<char>> FileContentPtr;

	class Resource
	{
	public:
		static ResourcePtr create(const std::string& res_path);

		virtual ~Resource();
		virtual const std::string& getName() const;
		virtual DirectoryPtr getDirectory(const std::string& dir_name) const;
		virtual FilePtr getFile(const std::string& file_name) const;
		virtual bool loadDirectoryData(const std::string& dir_name, std::vector<IDirectory::FileOrDirectory>*);
		virtual bool loadFileData(const std::string& file_name, FileContentPtr*);

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
			mutable std::weak_ptr<std::vector<char>> ptr;
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
		virtual bool add(std::shared_ptr<void> object, const std::string& object_name);
		virtual void release(const std::string& object_name);
		virtual std::shared_ptr<void> get(const std::string& object_name) const;

	private:
		mutable std::mutex m_mutex;
		std::map<std::string, std::shared_ptr<Resource>> m_resources;
		std::map<std::string, std::shared_ptr<void>> m_objects;
	};

	class ResourceCreator : public IResourceCreator
	{
	public:
		static ResourceCreatorPtr create(const std::string& res_path, bool append_mode = false);

		virtual ~ResourceCreator();
		virtual bool addFile(const std::string& file_path, const std::string& res_file_name);
		virtual bool addFile(const std::wstring& file_path, const std::string& res_file_name);
		virtual FilePtr addFile(const std::string& res_file_name);
		virtual bool addDirectory(const std::string& dir_path);
		virtual bool addDirectory(const std::wstring& dir_path);
		bool addFileData(const std::string& res_file_name, const std::vector<char>& data);

	private:
		ResourceCreator(const std::string& res_path, bool append_mode);
		virtual bool init();
		bool collectFiles(std::wstring dir_name, std::vector<std::wstring>& files);

		mutable std::mutex m_mutex;
		std::weak_ptr<ResourceCreator> m_self_ptr;
		std::ofstream m_file;
	};

	class FileSerializer : public IFileSerializer
	{
	public:
		FileSerializer();
		virtual ~FileSerializer();
		virtual FilePtr openFile(const std::string& file_name, OpenMode) const;
		virtual FilePtr openFile(const std::wstring& file_name, OpenMode) const;
	};

	class ResourceManager : public IResourceManager
	{
	public:
		ResourceManager();
		virtual ~ResourceManager();
		virtual ResourcePoolPtr createResourcePool() const;
		virtual ResourcePoolPtr getDefaultResourcePool();
		virtual FileSerializerPtr getFileSerializer();
		virtual ResourceCreatorPtr createResource(const std::string& res_path, bool append_mode = false) const;

	private:
		mutable std::mutex m_mutex;
		ResourcePoolPtr m_default_res_pool;
		FileSerializerPtr m_file_serializer;
	};

	class Directory : public IDirectory
	{
	public:
		Directory(ResourcePtr res, std::string& name);
		virtual const std::string& getName() const;
		virtual Iterator begin() const;
		virtual Iterator end() const;

	private:
		std::string m_name;
		std::vector<FileOrDirectory> m_files;
		ResourcePtr m_res;
	};

	class File : public Stream, public IFile
	{
	public:
		File(ResourcePtr res, const std::string& name);
		virtual ~File();
		virtual const std::string& getName() const;
		virtual const char* getData() const;
		virtual size_t getSize() const;
		virtual void unload();
		virtual size_t write(const char* buf, size_t len);
		virtual size_t read(char* buf, size_t len);

	private:
		ResourcePtr m_res;
		std::string m_name;
		mutable FileContentPtr m_data;
		size_t m_data_pos;
	};

	class SerializableFile : public Stream, public IFile
	{
	public:
		static FilePtr create(const std::string& file_name, IFileSerializer::OpenMode);
		static FilePtr create(const std::wstring& file_name, IFileSerializer::OpenMode);

		virtual ~SerializableFile();
		virtual const std::string& getName() const;
		virtual const char* getData() const;
		virtual size_t getSize() const;
		virtual void unload();
		virtual size_t write(const char* buf, size_t len);
		virtual size_t read(char* buf, size_t len);

	private:
		SerializableFile(const std::string& file_name, IFileSerializer::OpenMode);
		SerializableFile(const std::wstring& file_name, IFileSerializer::OpenMode);
		bool init();

		std::string m_name;
		mutable std::fstream m_file;
		size_t m_size;
		mutable std::vector<char> m_data;
	};

	class MemoryFile : public Stream, public IFile
	{
	public:
		MemoryFile(std::shared_ptr<ResourceCreator> res, const std::string& res_file_name);
		virtual ~MemoryFile();
		virtual const std::string& getName() const;
		virtual const char* getData() const;
		virtual size_t getSize() const;
		virtual void unload();
		virtual size_t write(const char* buf, size_t len);
		virtual size_t read(char* buf, size_t len);

	private:
		std::shared_ptr<ResourceCreator> m_res;
		std::string m_name;
		std::vector<char> m_data;
	};
};
