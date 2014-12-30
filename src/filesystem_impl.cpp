#include <cstdint>
#include "Doboz/Decompressor.h"
#include "filesystem_impl.hpp"

std::mutex s_archives_mutex;
std::map<std::string, gg::fs::IArchive*> s_archives;

static std::string getPathRoot(const std::string& path)
{
	auto it = path.find('/');
	return path.substr(0, it);
}

static std::string getPathEnd(const std::string& path)
{
	auto it = path.rfind('/');
	return path.substr(it);
}

/*static void replaceSlashes(std::string& file_name)
{
	for (char& c : file_name)
	{
		if (c == '\\') c = '/';
	}
}*/


bool gg::fs::addDirectory(const std::string& dir_name)
{
	IArchive* ar = new DirectoryArchive(dir_name);

	if (ar->init())
	{
		std::lock_guard<std::mutex> guard(s_archives_mutex);
		s_archives.emplace(getPathRoot(dir_name), ar);
		return true;
	}
	else
	{
		return false;
	}
}

bool gg::fs::addVirtualDirectory(const std::string& archive_name)
{
	IArchive* ar = new VirtualArchive(archive_name);

	if (ar->init())
	{
		std::lock_guard<std::mutex> guard(s_archives_mutex);
		s_archives.emplace(getPathEnd(archive_name), ar);
		return true;
	}
	else
	{
		return false;
	}
}

std::shared_ptr<gg::fs::IDirectory> openDirectory(const std::string& dir_name)
{
	std::string ar_name = getPathRoot(dir_name);

	auto it = s_archives.find(ar_name);
	if (it != s_archives.end())
	{
		return it->second->getDirectory(dir_name.substr(dir_name.find('/') + 1));
	}
	else
	{
		return {};
	}
}

std::shared_ptr<gg::fs::IFile> openFile(const std::string& file_name)
{
	std::string ar_name = getPathRoot(file_name);

	auto it = s_archives.find(ar_name);
	if (it != s_archives.end())
	{
		return it->second->getFile(file_name.substr(file_name.find('/') + 1));
	}
	else
	{
		return {};
	}
}


gg::fs::DirectoryArchive::DirectoryArchive(const std::string& dir_name) :
	m_name(getPathEnd(dir_name)), m_full_path(dir_name)
{
}

bool gg::fs::DirectoryArchive::init()
{
	return false;
}

const std::string& gg::fs::DirectoryArchive::getName() const
{
	return m_name;
}

std::shared_ptr<gg::fs::IDirectory> gg::fs::DirectoryArchive::getDirectory(const std::string& dir_name)
{
	return std::shared_ptr<IDirectory>(new Directory(this, m_name + '/' + dir_name));
}

std::shared_ptr<gg::fs::IFile> gg::fs::DirectoryArchive::getFile(const std::string& file_name)
{
	std::lock_guard<std::mutex> guard(m_mutex);

	auto it = m_files.find(file_name);
	if (it != m_files.end())
	{
		if (it->second.expired())
		{
			std::shared_ptr<IFile> ptr(new File(this, m_name + '/' + file_name));
			it->second = ptr;
			return ptr;
		}
		else
		{
			return it->second.lock();
		}
	}
	else
	{
		std::shared_ptr<IFile> ptr(new File(this, m_name + '/' + file_name));
		m_files.emplace(file_name, ptr);
		return ptr;
	}
}

bool gg::fs::DirectoryArchive::loadDirectoryData(const std::string& dir_name, std::vector<IDirectory::FileOrDirectory>* files)
{
	return false;
}

bool gg::fs::DirectoryArchive::loadFileData(const std::string& file_name, const char** data, size_t* size)
{
	return false;
}


gg::fs::VirtualArchive::VirtualArchive(const std::string& archive_name) :
	m_file(archive_name, std::ios::in | std::ios::binary),
	m_name(getPathEnd(archive_name))
{
}

gg::fs::VirtualArchive::~VirtualArchive()
{
	m_file.close();
}

bool gg::fs::VirtualArchive::init()
{
	if (!m_file.is_open())
		return false;

	while (!m_file.eof())
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
	}

	return true;
}

const std::string& gg::fs::VirtualArchive::getName() const
{
	return m_name;
}

std::shared_ptr<gg::fs::IDirectory> gg::fs::VirtualArchive::getDirectory(const std::string& dir_name)
{
	return std::shared_ptr<IDirectory>(new Directory(this, m_name + '/' + dir_name));
}

std::shared_ptr<gg::fs::IFile> gg::fs::VirtualArchive::getFile(const std::string& file_name)
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

bool gg::fs::VirtualArchive::loadDirectoryData(const std::string& dir_name, std::vector<IDirectory::FileOrDirectory>* files)
{
	std::lock_guard<std::mutex> guard(m_mutex);

	for (auto& it : m_files)
	{
		FileData& fd = it.second;

		if (fd.file_name.compare(0, dir_name.size(), dir_name) == 0)
		{
			IDirectory::FileOrDirectory file = {};
			file.type = IDirectory::FileOrDirectory::Type::FILE;
			file.file = getFile(fd.file_name);

			files->push_back(file);
		}
	}

	return true;
}

bool gg::fs::VirtualArchive::loadFileData(const std::string& file_name, const char** data, size_t* size)
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
