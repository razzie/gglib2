/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <cstdint>
#include <iostream>
#include <set>
#include "Doboz/Compressor.h"
#include "Doboz/Decompressor.h"
#include "filesystem_impl.hpp"
#include "stringutil.hpp"

static gg::FileSystem s_fs;
gg::IFileSystem& gg::fs = s_fs;


#ifdef _WIN32
#include <Windows.h>

static bool collectFiles(std::wstring dir_name, std::vector<std::wstring>& files)
{
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
}
#endif // _WIN32

static bool packDirectory(std::wstring dir_name, std::wstring archive_name)
{
	std::vector<std::wstring> files;
	if (!collectFiles(dir_name, files))
		return false;

	std::ofstream ar(archive_name, std::ios::out | std::ios::binary);
	if (!ar.is_open())
		return false;

	std::vector<char> buffer;

	for (auto& f : files)
	{
		// open file and get size
		std::ifstream file(f, std::ios::binary | std::ios::in);
		if (!file.is_open())
			return false;

		// get file size
		file.seekg(0, std::ios::end);
		const uint32_t size = static_cast<uint32_t>(file.tellg());
		file.seekg(0, std::ios::beg);

		// allocate memory for original and compressed data
		buffer.resize(size + static_cast<uint32_t>(doboz::Compressor::getMaxCompressedSize(size)));

		// read file data
		if (!file.read(buffer.data(), size))
			return false;
		file.close();

		// compress data
		uint32_t compressed_size;
		doboz::Compressor().compress(&buffer[0], size, &buffer[size], buffer.size() - size, compressed_size);

		// get relative name of file
		std::wstring wname = f.substr(dir_name.size() + 1);
		std::string name;
		for (wchar_t& wc : wname)
		{
			if (wc == L'\\') wc = L'/';
			if ((unsigned)wc <= 255)
				name += (char)wc;
			else
				name += '?';
		}
		const uint16_t name_size = static_cast<uint16_t>(name.size());

		// write data to archive
		ar.write(reinterpret_cast<const char*>(&name_size), sizeof(uint16_t)); // 16bit length of file name
		ar.write(name.c_str(), name.size()); // file name (not zero terminated)
		ar.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t)); // 32bit original size
		ar.write(reinterpret_cast<const char*>(&compressed_size), sizeof(uint32_t)); // 32bit compressed size
		ar.write(&buffer[size], compressed_size); // actual compressed data
		ar.flush();

#ifdef _DEBUG
		std::cout << "File added: " << name << std::endl;
#endif
	}

	ar.close();

	return true;
}

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


gg::FileSystem::FileSystem()
{
}

gg::FileSystem::~FileSystem()
{
}

bool gg::FileSystem::createVirtualDirectoryFile(const std::string& dir_path) const
{
	std::wstring dir = convertString<char, wchar_t>(dir_path);
	return createVirtualDirectoryFile(dir);
}

bool gg::FileSystem::createVirtualDirectoryFile(const std::wstring& dir_path) const
{
#ifdef _WIN32
	std::wstring dir = dir_path;
	if (dir.back() == '/') dir.pop_back();
	return packDirectory(dir, dir + L".pak");
#else
	return false;
#endif // _WIN32
}

bool gg::FileSystem::addVirtualDirectory(const std::string& vdir_path)
{
	VirtualDirectory* vdir = new VirtualDirectory(vdir_path);

	if (vdir->init())
	{
		m_vdirs.emplace(getPathEnd(vdir_path), vdir);
		return true;
	}
	else
	{
		return false;
	}
}

std::shared_ptr<gg::IDirectory> gg::FileSystem::openDirectory(const std::string& dir_name)
{
	std::string vdir_name = getPathRoot(dir_name);

	auto it = m_vdirs.find(vdir_name);
	if (it != m_vdirs.end())
	{
		return it->second->getDirectory(dir_name.substr(dir_name.find('/') + 1));
	}
	else
	{
		return {};
	}
}

std::shared_ptr<gg::IFile> gg::FileSystem::openFile(const std::string& file_name)
{
	std::string vdir_name = getPathRoot(file_name);

	auto it = m_vdirs.find(vdir_name);
	if (it != m_vdirs.end())
	{
		return it->second->getFile(file_name.substr(file_name.find('/') + 1));
	}
	else
	{
		return {};
	}
}


gg::VirtualDirectory::VirtualDirectory(const std::string& vdir_name) :
	m_file(vdir_name, std::ios::in | std::ios::binary),
	m_name(getPathEnd(vdir_name))
{
}

gg::VirtualDirectory::~VirtualDirectory()
{
	m_file.close();
}

bool gg::VirtualDirectory::init()
{
	if (!m_file.is_open())
		return false;

	m_file.seekg(0, std::ios::end);
	const std::streampos end_pos = m_file.tellg();
	m_file.seekg(0, std::ios::beg);

	while (m_file)
	{
		// read file name
		std::string file_name;
		uint16_t name_size;
		m_file.read(reinterpret_cast<char*>(&name_size), sizeof(uint16_t));
		file_name.resize(name_size);
		m_file.read(&file_name[0], name_size);

		// insert to file list
		FileData& fd = m_files[file_name];
		fd.file_name = std::move(file_name);

		// read file data
		m_file.read(reinterpret_cast<char*>(&fd.original_size), sizeof(uint32_t));
		m_file.read(reinterpret_cast<char*>(&fd.compressed_size), sizeof(uint32_t));
		fd.start_pos = m_file.tellg(); // get pointer to compressed content

		// skip compressed content
		m_file.seekg(fd.compressed_size, std::ios_base::cur);

		if (m_file.tellg() == end_pos)
			break;
	}

	return true;
}

const std::string& gg::VirtualDirectory::getName() const
{
	return m_name;
}

std::shared_ptr<gg::IDirectory> gg::VirtualDirectory::getDirectory(const std::string& dir_name)
{
	return std::shared_ptr<IDirectory>(new Directory(this, m_name + '/' + dir_name));
}

std::shared_ptr<gg::IFile> gg::VirtualDirectory::getFile(const std::string& file_name)
{
	auto it = m_files.find(file_name);
	if (it != m_files.end())
	{
		FileData& fd = it->second;
		std::lock_guard<decltype(fd.ptr_mutex)> guard(fd.ptr_mutex);

		if (!fd.ptr.expired())
		{
			return fd.ptr.lock();
		}
		else
		{
			std::shared_ptr<IFile> ptr(new File(this, m_name + '/' + file_name));
			fd.ptr = ptr;
			return ptr;
		}
	}

	return {};
}

bool gg::VirtualDirectory::loadDirectoryData(const std::string& dir_name, std::vector<IDirectory::FileOrDirectory>* files)
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

bool gg::VirtualDirectory::loadFileData(const std::string& file_name, const char** data, size_t* size)
{
	auto it = m_files.find(file_name);
	if (it != m_files.end())
	{
		const FileData& fd = it->second;

		char* compressed_data = new char[fd.compressed_size];
		char* decompressed_data = new char[fd.original_size];

		m_file.seekg(fd.start_pos);
		m_file.read(compressed_data, fd.compressed_size);
		doboz::Decompressor().decompress(compressed_data, fd.compressed_size, decompressed_data, fd.original_size);

		delete[] compressed_data;
		*data = decompressed_data;
		*size = fd.original_size;

		return true;
	}

	return false;
}
