/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <Windows.h>
#include <ShlObj.h>
#include "Doboz/Compressor.h"

const wchar_t* browseFolder()
{
	struct Callback
	{
		static int CALLBACK proc(HWND hwnd, UINT uMsg, LPARAM, LPARAM lpData)
		{
			if (uMsg == BFFM_INITIALIZED)
			{
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
			}

			return 0;
		}
	};

	static wchar_t path[MAX_PATH];
	_wgetcwd(path, MAX_PATH);

	BROWSEINFO bi = { 0 };
	bi.lpszTitle = L"Select folder to be packed (gglib format)";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = Callback::proc;
	bi.lParam = (LPARAM)path;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0)
	{
		SHGetPathFromIDList(pidl, path);

		IMalloc* imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(pidl);
			imalloc->Release();
		}

		return path;
	}

	return nullptr;
}

bool collectFiles(std::wstring dir_name, std::vector<std::wstring>& files)
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
	}
	while (FindNextFile(handle, &find_data));

	FindClose(handle);

	return true;
}

bool packDirectory(std::wstring dir_name, std::wstring archive_name)
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

		std::cout << "File added: " << name << std::endl;
	}

	ar.close();

	return true;
}

//int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
int main()
{
	std::wstring dir = browseFolder();
	packDirectory(dir, dir + L".pak");

	return 0;
}
