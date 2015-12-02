/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <cstdint>
#include <locale>
#include <set>
#include "Doboz/Compressor.h"
#include "Doboz/Decompressor.h"
#include "encoder.hpp"
#include "resource_impl.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

static gg::ResourceManager s_res;
gg::IResourceManager& gg::res = s_res;

static std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>, wchar_t> convert;


static std::string getPathRoot(const std::string& path)
{
	auto pos = path.find('/');
	return path.substr(0, pos);
}

static std::string getPathEnd(const std::string& path)
{
	auto pos = path.rfind('/');
	return path.substr(pos + 1);
}

/*static void replaceSlashes(std::string& file_name)
{
	for (char& c : file_name)
	{
		if (c == '\\') c = '/';
	}
}*/



gg::ResourcePtr gg::Resource::create(const std::string& res_path)
{
	ResourcePtr res(new Resource(res_path));
	if (res->init())
	{
		res->m_self_ptr = res;
		return res;
	}
	else
	{
		return {};
	}
}

gg::Resource::Resource(const std::string& res_name) :
	m_file(res_name, std::ios::in | std::ios::binary),
	m_name(getPathEnd(res_name))
{
}

gg::Resource::~Resource()
{
	m_file.close();
}

bool gg::Resource::init()
{
	if (!m_file.is_open())
		return false;

	m_file.seekg(0, std::ios::end);
	const std::streampos end_pos = m_file.tellg();
	m_file.seekg(0, std::ios::beg);

	while (m_file)
	{
		// read file name and decrypt it
		std::string file_name;
		uint16_t name_size;
		m_file.read(reinterpret_cast<char*>(&name_size), sizeof(uint16_t));
		file_name.resize(name_size);
		m_file.read(&file_name[0], name_size);
		decode(reinterpret_cast<unsigned char*>(&file_name[0]), file_name.size());

		// insert to file list
		FileData& fd = m_files[file_name];
		fd.file_name = std::move(file_name);

		// read file data
		m_file.read(reinterpret_cast<char*>(&fd.original_size), sizeof(uint32_t));
		m_file.read(reinterpret_cast<char*>(&fd.compressed_size), sizeof(uint32_t));
		fd.start_pos = static_cast<size_t>(m_file.tellg()); // get pointer to compressed content

		// skip compressed content
		m_file.seekg(fd.compressed_size, std::ios_base::cur);

		if (m_file.tellg() == end_pos)
			break;
	}

	return true;
}

const std::string& gg::Resource::getName() const
{
	return m_name;
}

gg::DirectoryPtr gg::Resource::getDirectory(const std::string& dir_name) const
{
	return DirectoryPtr(new Directory(m_self_ptr.lock(), m_name + '/' + dir_name));
}

gg::FilePtr gg::Resource::getFile(const std::string& file_name) const
{
	auto it = m_files.find(file_name);
	if (it != m_files.end())
	{
		return FilePtr(new File(m_self_ptr.lock(), m_name + '/' + file_name));
	}

	return {};
}

bool gg::Resource::loadDirectoryData(const std::string& dir_name, std::vector<IDirectory::FileOrDirectory>* files)
{
	std::set<std::string> directories;
	const size_t name_begin_pos = dir_name.size();

	for (auto& it : m_files)
	{
		FileData& fd = it.second;

		if (fd.file_name.compare(0, name_begin_pos, dir_name) == 0)
		{
			size_t slash_pos = fd.file_name.find('/', name_begin_pos);

			if (slash_pos == std::string::npos) // it's a file
			{
				IDirectory::FileOrDirectory file = {};
				file.name = m_name + '/' + fd.file_name;
				file.size = fd.original_size;
				file.type = IDirectory::FileOrDirectory::Type::FILE;

				files->push_back(file);
			}
			else
			{
				directories.insert(m_name + '/' +
					fd.file_name.substr(name_begin_pos, slash_pos) + '/');
			}
		}
	}

	for (const std::string& dir : directories)
	{
		files->push_back({ dir, 0, IDirectory::FileOrDirectory::Type::DIRECTORY });
	}

	return true;
}

bool gg::Resource::loadFileData(const std::string& file_name, FileContentPtr* data)
{
	auto it = m_files.find(file_name);
	if (it != m_files.end())
	{
		const FileData& fd = it->second;
		std::lock_guard<decltype(fd.ptr_mutex)> guard(fd.ptr_mutex);

		if (!fd.ptr.expired())
		{
			*data = fd.ptr.lock();
		}
		else
		{
			std::vector<char> compressed_data;
			std::vector<char> decompressed_data;
			compressed_data.resize(fd.compressed_size);
			decompressed_data.resize(fd.original_size);

			m_file.seekg(fd.start_pos);
			m_file.read(&compressed_data[0], fd.compressed_size);
			doboz::Decompressor().decompress(&compressed_data[0], fd.compressed_size, &decompressed_data[0], fd.original_size);

			data->reset(new std::vector<char>(std::move(decompressed_data)));
			fd.ptr = *data;
		}

		return true;
	}

	return false;
}



gg::ResourcePool::ResourcePool()
{
}

gg::ResourcePool::~ResourcePool()
{
}

bool gg::ResourcePool::includeResource(const std::string& res_path)
{
	auto res = Resource::create(res_path);
	if (res)
	{
		std::lock_guard<decltype(m_mutex)> guard(m_mutex);
		m_resources.emplace(getPathEnd(res_path), res);
		return true;
	}
	else
	{
		return false;
	}
}

void gg::ResourcePool::releaseResources()
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	m_resources.clear();
}

gg::DirectoryPtr gg::ResourcePool::openDirectory(const std::string& dir_name) const
{
	std::string res_name = getPathRoot(dir_name);

	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	auto it = m_resources.find(res_name);
	if (it != m_resources.end())
	{
		return it->second->getDirectory(dir_name.substr(dir_name.find('/') + 1));
	}
	else
	{
		return{};
	}
}

gg::FilePtr gg::ResourcePool::openFile(const std::string& file_name) const
{
	std::string res_name = getPathRoot(file_name);

	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	auto it = m_resources.find(res_name);
	if (it != m_resources.end())
	{
		return it->second->getFile(file_name.substr(file_name.find('/') + 1));
	}
	else
	{
		return{};
	}
}



gg::ResourceCreatorPtr gg::ResourceCreator::create(const std::string& res_path, bool append_mode)
{
	std::shared_ptr<ResourceCreator> creator(new ResourceCreator(res_path, append_mode));
	if (creator->init())
	{
		creator->m_self_ptr = creator;
		return creator;
	}
	else
	{
		return {};
	}
}

gg::ResourceCreator::ResourceCreator(const std::string& res_path, bool append_mode)
{
	std::ios::openmode mode = std::ios::out | std::ios::binary;

	if (append_mode)
		mode |= std::ios::app;
	else
		mode |= std::ios::trunc;

	m_file.open(res_path, mode);
}

gg::ResourceCreator::~ResourceCreator()
{
	if (m_file.is_open())
		m_file.close();
}

bool gg::ResourceCreator::init()
{
	if (!m_file.is_open())
		return false;

	return true;
}

bool gg::ResourceCreator::collectFiles(std::wstring dir_name, std::vector<std::wstring>& files)
{
#ifdef _WIN32
	if (files.size() > 1000)
		return false;

	if (dir_name.back() != L'\\')
		dir_name += L'\\';

	HANDLE handle;
	WIN32_FIND_DATA find_data;

	handle = FindFirstFile((dir_name + L'*').c_str(), &find_data);
	if (handle == INVALID_HANDLE_VALUE)
		return false;

	do
	{
		if (find_data.cFileName[0] == L'.')
			continue;

		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!collectFiles(dir_name + find_data.cFileName, files))
				return false;
		}
		else
		{
			files.push_back(dir_name + find_data.cFileName);
		}
	} while (FindNextFile(handle, &find_data));

	FindClose(handle);

	return true;
#else
	return false;
#endif // _WIN32
}

bool gg::ResourceCreator::addFile(const std::string& file_path, const std::string& res_file_name)
{
	std::wstring file = convert.from_bytes(file_path);
	return addFile(file, res_file_name);
}

bool gg::ResourceCreator::addFile(const std::wstring& file_path, const std::string& res_file_name)
{
	std::vector<char> buffer;

	// open file and get size
	std::ifstream file(file_path, std::ios::binary | std::ios::in);
	if (!file.is_open())
		return false;

	// get file size
	file.seekg(0, std::ios::end);
	const size_t size = static_cast<size_t>(file.tellg());
	file.seekg(0, std::ios::beg);

	// allocate memory for file data
	buffer.resize(size);

	// read file data
	if (!file.read(buffer.data(), size))
		return false;
	file.close();

	return addFileData(res_file_name, buffer);
}

gg::FilePtr gg::ResourceCreator::addFile(const std::string& res_file_name)
{
	return FilePtr(new MemoryFile(m_self_ptr.lock(), res_file_name));
}

bool gg::ResourceCreator::addDirectory(const std::string& dir_path)
{
	std::wstring dir = convert.from_bytes(dir_path);
	return addDirectory(dir);
}

bool gg::ResourceCreator::addDirectory(const std::wstring& dir_path)
{
	std::wstring dir = dir_path;
	if (dir.back() == '/' || dir.back() == '\\')
		dir.pop_back();

	std::vector<std::wstring> files;
	if (!collectFiles(dir, files))
		return false;

	for (auto& f : files)
	{
		std::string res_file_name = convert.to_bytes(f.substr(dir.size() + 1));
		if (!addFile(f, res_file_name))
			return false;
	}

	return true;
}

bool gg::ResourceCreator::addFileData(const std::string& res_file_name, const std::vector<char>& data)
{
	uint32_t orig_size = static_cast<uint32_t>(data.size());
	uint32_t compressed_size = static_cast<uint32_t>(doboz::Compressor::getMaxCompressedSize(data.size()));

	// buffer for compressed file data
	std::vector<char> buffer;
	buffer.resize(compressed_size);

	// compress data
	doboz::Compressor().compress(&data[0], orig_size, &buffer[0], buffer.size(), compressed_size);

	// encrypt file name
	std::vector<unsigned char> name;
	name.resize(res_file_name.size());
	const uint16_t name_size = static_cast<uint16_t>(name.size());
	for (uint16_t i = 0; i < name_size; ++i)
	{
		char c = res_file_name[i];
		if (c == '\\')
			c = '/';

		name[i] = encode(static_cast<unsigned char>(c));
	}

	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	// write data to archive
	m_file.write(reinterpret_cast<const char*>(&name_size), sizeof(uint16_t)); // 16bit length of file name
	m_file.write(reinterpret_cast<const char*>(name.data()), name.size()); // file name (not zero terminated)
	m_file.write(reinterpret_cast<const char*>(&orig_size), sizeof(uint32_t)); // 32bit original size
	m_file.write(reinterpret_cast<const char*>(&compressed_size), sizeof(uint32_t)); // 32bit compressed size
	m_file.write(buffer.data(), compressed_size); // actual compressed data
	m_file.flush();

	return static_cast<bool>(m_file);
}



gg::FileSerializer::FileSerializer()
{
}

gg::FileSerializer::~FileSerializer()
{
}

gg::FilePtr gg::FileSerializer::openFile(const std::string& file_name, OpenMode mode) const
{
	return SerializableFile::create(file_name, mode);
}

gg::FilePtr gg::FileSerializer::openFile(const std::wstring& file_name, OpenMode mode) const
{
	return SerializableFile::create(file_name, mode);
}



gg::ResourceManager::ResourceManager() :
	m_default_res_pool(new ResourcePool()),
	m_file_serializer(new FileSerializer())
{
}

gg::ResourceManager::~ResourceManager()
{
}

gg::ResourcePoolPtr gg::ResourceManager::createResourcePool() const
{
	return ResourcePoolPtr(new ResourcePool());
}

gg::ResourcePoolPtr gg::ResourceManager::getDefaultResourcePool()
{
	return m_default_res_pool;
}

gg::FileSerializerPtr gg::ResourceManager::getFileSerializer()
{
	return m_file_serializer;
}

gg::ResourceCreatorPtr gg::ResourceManager::createResource(const std::string& res_path, bool append_mode) const
{
	return ResourceCreator::create(res_path, append_mode);
}



gg::Directory::Directory(ResourcePtr res, std::string& name) :
	m_name(name), m_res(res)
{
	m_res->loadDirectoryData(m_name.substr(m_name.find('/') + 1), &m_files);
}

const std::string& gg::Directory::getName() const
{
	return m_name;
}

gg::IDirectory::Iterator gg::Directory::begin() const
{
	return m_files.begin();
}

gg::IDirectory::Iterator gg::Directory::end() const
{
	return m_files.end();
}



gg::File::File(ResourcePtr res, const std::string& name) :
	Stream(IStream::Mode::DESERIALIZE),
	m_name(name),
	m_res(res)
{
}

gg::File::~File()
{
}

const std::string& gg::File::getName() const
{
	return m_name;
}

const char* gg::File::getData() const
{
	if (!m_data)
		m_res->loadFileData(m_name.substr(m_name.find('/') + 1), &m_data);

	return m_data->data();
}

size_t gg::File::getSize() const
{
	if (!m_data)
		m_res->loadFileData(m_name.substr(m_name.find('/') + 1), &m_data);

	return m_data->size();
}

void gg::File::unload()
{
	m_data.reset();
}

size_t gg::File::write(const char*, size_t)
{
	if (getMode() != Mode::SERIALIZE)
		throw SerializationError();

	return 0;
}

size_t gg::File::read(char* buf, size_t len)
{
	if (getMode() != Mode::DESERIALIZE)
		throw SerializationError();

	size_t size = getSize();
	if (m_data_pos + len > size)
		len = size - m_data_pos;

	m_data_pos += len;
	std::memcpy(buf, getData(), len);
	return len;
}



gg::FilePtr gg::SerializableFile::create(const std::string& file_name, IFileSerializer::OpenMode mode)
{
	std::shared_ptr<SerializableFile> ptr(new SerializableFile(file_name, mode));
	if (ptr->init())
		return ptr;
	else
		return {};
}

gg::FilePtr gg::SerializableFile::create(const std::wstring& file_name, IFileSerializer::OpenMode mode)
{
	std::shared_ptr<SerializableFile> ptr(new SerializableFile(file_name, mode));
	if (ptr->init())
		return ptr;
	else
		return{};
}

gg::SerializableFile::SerializableFile(const std::string& file_name, IFileSerializer::OpenMode mode) :
	Stream((mode == IFileSerializer::OpenMode::READ) ? IStream::Mode::DESERIALIZE : IStream::Mode::SERIALIZE),
	m_name(file_name),
	m_size(0)
{
	switch (mode)
	{
	case IFileSerializer::OpenMode::READ:
		m_file.open(file_name, std::ios::binary | std::ios::in);
		break;

	case IFileSerializer::OpenMode::APPEND:
		m_file.open(file_name, std::ios::binary | std::ios::out | std::ios::app);
		break;

	case IFileSerializer::OpenMode::REWRITE:
		m_file.open(file_name, std::ios::binary | std::ios::out | std::ios::trunc);
		break;

	default:
		break;
	}
}

gg::SerializableFile::SerializableFile(const std::wstring& file_name, IFileSerializer::OpenMode mode) :
	Stream((mode == IFileSerializer::OpenMode::READ) ? IStream::Mode::DESERIALIZE : IStream::Mode::SERIALIZE),
	m_name(convert.to_bytes(file_name)),
	m_size(0)
{
	switch (mode)
	{
	case IFileSerializer::OpenMode::READ:
		m_file.open(file_name, std::ios::binary | std::ios::in);
		break;

	case IFileSerializer::OpenMode::APPEND:
		m_file.open(file_name, std::ios::binary | std::ios::out | std::ios::app);
		break;

	case IFileSerializer::OpenMode::REWRITE:
		m_file.open(file_name, std::ios::binary | std::ios::out | std::ios::trunc);
		break;

	default:
		break;
	}
}

gg::SerializableFile::~SerializableFile()
{
	if (m_file.is_open())
		m_file.close();
}

bool gg::SerializableFile::init()
{
	if (m_file.is_open())
	{
		m_size = static_cast<size_t>(m_file.end - m_file.beg);
		return m_file.good();
	}
	else
	{
		return false;
	}
}

const std::string& gg::SerializableFile::getName() const
{
	return m_name;
}

const char* gg::SerializableFile::getData() const
{
	if (m_data.size() < m_size)
	{
		std::streampos curr_pos = m_file.tellg();
		m_file.seekg(m_file.beg);
		m_data.resize(m_size);
		m_file.read(&m_data[0], m_size);
		m_file.seekg(curr_pos);
	}

	return m_data.data();
}

size_t gg::SerializableFile::getSize() const
{
	return m_size;
}

void gg::SerializableFile::unload()
{
	m_data.clear();
	m_data.shrink_to_fit();
}

size_t gg::SerializableFile::write(const char* buf, size_t len)
{
	if (getMode() != Mode::SERIALIZE || !m_file.is_open())
		throw SerializationError();

	m_file.write(buf, len);
	m_size += len;
	return len;
}

size_t gg::SerializableFile::read(char* buf, size_t len)
{
	if (getMode() != Mode::DESERIALIZE || !m_file.is_open())
		throw SerializationError();

	m_file.read(buf, len);
	return static_cast<size_t>(m_file.gcount());
}



gg::MemoryFile::MemoryFile(std::shared_ptr<ResourceCreator> res, const std::string& res_file_name) :
	Stream(IStream::Mode::SERIALIZE),
	m_res(res),
	m_name(res_file_name)
{
}

gg::MemoryFile::~MemoryFile()
{
	m_res->addFileData(m_name, m_data);
}

const std::string& gg::MemoryFile::getName() const
{
	return m_name;
}

const char* gg::MemoryFile::getData() const
{
	return m_data.data();
}

size_t gg::MemoryFile::getSize() const
{
	return m_data.size();
}

void gg::MemoryFile::unload()
{
	m_data.clear();
}

size_t gg::MemoryFile::write(const char* buf, size_t len)
{
	m_data.insert(m_data.end(), buf, buf + len);
	return len;
}

size_t gg::MemoryFile::read(char*, size_t)
{
	throw SerializationError();
}
