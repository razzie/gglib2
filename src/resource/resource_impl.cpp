/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <cstdint>
#include <set>
#include "Doboz/Compressor.h"
#include "Doboz/Decompressor.h"
#include "resource_impl.hpp"
#include "stringutil.hpp"

static gg::ResourceManager s_res;
gg::IResourceManager& gg::res = s_res;


/*static void gen_shuffle_array()
{
	srand(time(NULL));

	int a = (rand() % 227) + 1;
	int b = (rand() % 223) + 1;
	int c = (rand() % 211) + 1;

	int skip = (a * 256 * 256) + (b * 256) + c;
	skip &= ~0xc0000000;

	std::cout << "{" << std::endl;

	int pos = 0;
	for (int i = 0; i < 256; ++i)
	{
		pos += skip;
		pos %= 257; // first prime greater than 256
		std::cout << pos-1 << ", ";

		if (i % 16 == 15)
			std::cout << std::endl;
	}

	std::cout << "}" << std::endl;
}*/

static const unsigned char shuffle[256] =
{
	125, 251, 120, 246, 115, 241, 110, 236, 105, 231, 100, 226, 95, 221, 90, 216,
	85, 211, 80, 206, 75, 201, 70, 196, 65, 191, 60, 186, 55, 181, 50, 176,
	45, 171, 40, 166, 35, 161, 30, 156, 25, 151, 20, 146, 15, 141, 10, 136,
	5, 131, 0, 126, 252, 121, 247, 116, 242, 111, 237, 106, 232, 101, 227, 96,
	222, 91, 217, 86, 212, 81, 207, 76, 202, 71, 197, 66, 192, 61, 187, 56,
	182, 51, 177, 46, 172, 41, 167, 36, 162, 31, 157, 26, 152, 21, 147, 16,
	142, 11, 137, 6, 132, 1, 127, 253, 122, 248, 117, 243, 112, 238, 107, 233,
	102, 228, 97, 223, 92, 218, 87, 213, 82, 208, 77, 203, 72, 198, 67, 193,
	62, 188, 57, 183, 52, 178, 47, 173, 42, 168, 37, 163, 32, 158, 27, 153,
	22, 148, 17, 143, 12, 138, 7, 133, 2, 128, 254, 123, 249, 118, 244, 113,
	239, 108, 234, 103, 229, 98, 224, 93, 219, 88, 214, 83, 209, 78, 204, 73,
	199, 68, 194, 63, 189, 58, 184, 53, 179, 48, 174, 43, 169, 38, 164, 33,
	159, 28, 154, 23, 149, 18, 144, 13, 139, 8, 134, 3, 129, 255, 124, 250,
	119, 245, 114, 240, 109, 235, 104, 230, 99, 225, 94, 220, 89, 215, 84, 210,
	79, 205, 74, 200, 69, 195, 64, 190, 59, 185, 54, 180, 49, 175, 44, 170,
	39, 165, 34, 160, 29, 155, 24, 150, 19, 145, 14, 140, 9, 135, 4, 130,
};

/*static void gen_deshuffle_array(const unsigned char* shuffle)
{
	std::cout << "{" << std::endl;

	int pos;
	for (int i = 0; i < 256; ++i)
	{
		// find position of i in original array
		for (pos = 0; shuffle[pos] != i; ++pos);

		std::cout << pos << ", ";

		if (i % 16 == 15)
			std::cout << std::endl;
	}

	std::cout << "}" << std::endl;
}*/

static const unsigned char deshuffle[256] =
{
	50, 101, 152, 203, 254, 48, 99, 150, 201, 252, 46, 97, 148, 199, 250, 44,
	95, 146, 197, 248, 42, 93, 144, 195, 246, 40, 91, 142, 193, 244, 38, 89,
	140, 191, 242, 36, 87, 138, 189, 240, 34, 85, 136, 187, 238, 32, 83, 134,
	185, 236, 30, 81, 132, 183, 234, 28, 79, 130, 181, 232, 26, 77, 128, 179,
	230, 24, 75, 126, 177, 228, 22, 73, 124, 175, 226, 20, 71, 122, 173, 224,
	18, 69, 120, 171, 222, 16, 67, 118, 169, 220, 14, 65, 116, 167, 218, 12,
	63, 114, 165, 216, 10, 61, 112, 163, 214, 8, 59, 110, 161, 212, 6, 57,
	108, 159, 210, 4, 55, 106, 157, 208, 2, 53, 104, 155, 206, 0, 51, 102,
	153, 204, 255, 49, 100, 151, 202, 253, 47, 98, 149, 200, 251, 45, 96, 147,
	198, 249, 43, 94, 145, 196, 247, 41, 92, 143, 194, 245, 39, 90, 141, 192,
	243, 37, 88, 139, 190, 241, 35, 86, 137, 188, 239, 33, 84, 135, 186, 237,
	31, 82, 133, 184, 235, 29, 80, 131, 182, 233, 27, 78, 129, 180, 231, 25,
	76, 127, 178, 229, 23, 74, 125, 176, 227, 21, 72, 123, 174, 225, 19, 70,
	121, 172, 223, 17, 68, 119, 170, 221, 15, 66, 117, 168, 219, 13, 64, 115,
	166, 217, 11, 62, 113, 164, 215, 9, 60, 111, 162, 213, 7, 58, 109, 160,
	211, 5, 56, 107, 158, 209, 3, 54, 105, 156, 207, 1, 52, 103, 154, 205,
};


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

		// get relative name of file and encrypt it
		std::wstring wname = f.substr(dir_name.size() + 1);
		std::vector<unsigned char> name;
		name.resize(wname.size());
		for (size_t i = 0; i < wname.size(); ++i)
		{
			if (wname[i] == L'\\') wname[i] = L'/';
			if (static_cast<unsigned>(wname[i]) <= 255)
				name[i] = shuffle[static_cast<unsigned char>(wname[i])];
			else
				name[i] = shuffle[static_cast<unsigned char>('?')];
		}
		const uint16_t name_size = static_cast<uint16_t>(name.size());

		// write data to archive
		ar.write(reinterpret_cast<const char*>(&name_size), sizeof(uint16_t)); // 16bit length of file name
		ar.write(reinterpret_cast<const char*>(name.data()), name.size()); // file name (not zero terminated)
		ar.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t)); // 32bit original size
		ar.write(reinterpret_cast<const char*>(&compressed_size), sizeof(uint32_t)); // 32bit compressed size
		ar.write(&buffer[size], compressed_size); // actual compressed data
		ar.flush();
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


gg::ResourceManager::ResourceManager()
{
}

gg::ResourceManager::~ResourceManager()
{
}

bool gg::ResourceManager::createResource(const std::string& dir_path) const
{
	std::wstring dir = convertString<char, wchar_t>(dir_path);
	return createResource(dir);
}

bool gg::ResourceManager::createResource(const std::wstring& dir_path) const
{
#ifdef _WIN32
	std::wstring dir = dir_path;
	if (dir.back() == '/') dir.pop_back();
	return packDirectory(dir, dir + L".res");
#else
	return false;
#endif // _WIN32
}

bool gg::ResourceManager::addResource(const std::string& res_path)
{
	Resource* res = new Resource(res_path);

	if (res->init())
	{
		m_resources.emplace(getPathEnd(res_path), res);
		return true;
	}
	else
	{
		return false;
	}
}

std::shared_ptr<gg::IDirectory> gg::ResourceManager::openDirectory(const std::string& dir_name)
{
	std::string vdir_name = getPathRoot(dir_name);

	auto it = m_resources.find(vdir_name);
	if (it != m_resources.end())
	{
		return it->second->getDirectory(dir_name.substr(dir_name.find('/') + 1));
	}
	else
	{
		return {};
	}
}

std::shared_ptr<gg::IFile> gg::ResourceManager::openFile(const std::string& file_name)
{
	std::string vdir_name = getPathRoot(file_name);

	auto it = m_resources.find(vdir_name);
	if (it != m_resources.end())
	{
		return it->second->getFile(file_name.substr(file_name.find('/') + 1));
	}
	else
	{
		return {};
	}
}


gg::Resource::Resource(const std::string& vdir_name) :
	m_file(vdir_name, std::ios::in | std::ios::binary),
	m_name(getPathEnd(vdir_name))
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
		for (uint16_t i = 0; i < name_size; ++i)
			file_name[i] = static_cast<char>( deshuffle[static_cast<unsigned char>(file_name[i])] );

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

const std::string& gg::Resource::name() const
{
	return m_name;
}

std::shared_ptr<gg::IDirectory> gg::Resource::getDirectory(const std::string& dir_name)
{
	return std::shared_ptr<IDirectory>(new Directory(this, m_name + '/' + dir_name));
}

std::shared_ptr<gg::IFile> gg::Resource::getFile(const std::string& file_name)
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

bool gg::Resource::loadFileData(const std::string& file_name, const char** data, size_t* size)
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
