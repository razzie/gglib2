/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <cstdint>
#include <set>
#include "Doboz/Decompressor.h"
#include "filesystem_impl.hpp"

std::mutex s_vdirs_mutex;
std::map<std::string, gg::VirtualDirectory*> s_vdirs;

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


bool gg::addVirtualDirectory(const std::string& vdir_path)
{
	VirtualDirectory* vdir = new VirtualDirectory(vdir_path);

	if (vdir->init())
	{
		std::lock_guard<std::mutex> guard(s_vdirs_mutex);
		s_vdirs.emplace(getPathEnd(vdir_path), vdir);
		return true;
	}
	else
	{
		return false;
	}
}

std::shared_ptr<gg::IDirectory> gg::openDirectory(const std::string& dir_name)
{
	std::string vdir_name = getPathRoot(dir_name);

	auto it = s_vdirs.find(vdir_name);
	if (it != s_vdirs.end())
	{
		return it->second->getDirectory(dir_name.substr(dir_name.find('/') + 1));
	}
	else
	{
		return {};
	}
}

std::shared_ptr<gg::IFile> gg::openFile(const std::string& file_name)
{
	std::string vdir_name = getPathRoot(file_name);

	auto it = s_vdirs.find(vdir_name);
	if (it != s_vdirs.end())
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
		uint16_t name_size;
		FileData fd = {};

		// read file data
		m_file.read(reinterpret_cast<char*>(&name_size), sizeof(uint16_t));
		fd.file_name.resize(name_size);
		m_file.read(&fd.file_name[0], name_size);
		m_file.read(reinterpret_cast<char*>(&fd.original_size), sizeof(uint32_t));
		m_file.read(reinterpret_cast<char*>(&fd.compressed_size), sizeof(uint32_t));
		fd.start_pos = m_file.tellg(); // get pointer to compressed content

		// skip compressed content
		m_file.seekg(fd.compressed_size, std::ios_base::cur);

		// store file data
		m_files.emplace(fd.file_name, fd);

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
	std::lock_guard<std::mutex> guard(m_mutex);

	auto it = m_files.find(file_name);
	if (it != m_files.end())
	{
		FileData& fd = it->second;

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
	std::lock_guard<std::mutex> guard(m_mutex);

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
	std::lock_guard<std::mutex> guard(m_mutex);

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
