/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

/**
 * Resources are archives that act like a directory.
 *
 *
 * HOW TO USE:
 * -----------
 *
 * In this example we have a 'textures.res' named resource in 'media' folder.
 * It contains 3 files: 'ground.png', 'water.png' and 'sky.png'.
 *
 * 1st step:
 * Add the resource by calling 'gg::res.addResource("media/textures.res")'.
 *
 * 2nd step:
 * Let's assume the program needs to open 'sky.png', which is a part of 'textures.res'
 * resource. It can be done by calling: 'auto file = gg::res.openFile("textures.res/sky.png")'
 *
 * Important:
 * - Do NOT add resources concurrently.
 * - Thread-safe file and directory operations are supported only AFTER finishing the addition
 *   of resources.
 * - Do NOT use backslash '\' characters in a file path. Always use slash '/' instead.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#if defined GGRESOURCE_BUILD
#	define GG_API __declspec(dllexport)
#else
#	define GG_API __declspec(dllimport)
#endif

namespace gg
{
	class IDirectory;
	class IFile;

	typedef std::shared_ptr<IDirectory> DirectoryPtr;
	typedef std::shared_ptr<IFile> FilePtr;

	class IResourceManager
	{
	public:
		virtual ~IResourceManager() = default;
		virtual bool createResource(const std::string& dir_path) const = 0;
		virtual bool addResource(const std::string& res_path) = 0;
		virtual DirectoryPtr openDirectory(const std::string& dir_name) = 0;
		virtual FilePtr openFile(const std::string& file_name) = 0;
	};

	extern GG_API IResourceManager& res;

	class IDirectory
	{
	public:
		struct FileOrDirectory
		{
			enum Type
			{
				FILE,
				DIRECTORY
			};

			std::string name;
			uint32_t size;
			Type type;
		};

		typedef std::vector<FileOrDirectory>::iterator Iterator;
		typedef std::vector<FileOrDirectory>::const_iterator ConstIterator;

		virtual ~IDirectory() = default;
		virtual const std::string& getName() const = 0;
		virtual Iterator begin() = 0;
		virtual Iterator end() = 0;
		virtual ConstIterator begin() const = 0;
		virtual ConstIterator end() const = 0;
	};

	class IFile
	{
	public:
		virtual ~IFile() = default;
		virtual const std::string& getName() const = 0;
		virtual const char* getData() const = 0;
		virtual size_t getSize() const = 0;
		virtual void unload() = 0; // can cause crash if other thread is reading file data!
	};

	class FileStream : public std::istream, public std::streambuf
	{
	public:
		FileStream(FilePtr file) :
			std::istream(this), m_file(file), m_pos(0)
		{
		}

		FileStream(const std::string& file_name) :
			FileStream(res.openFile(file_name))
		{
		}

		FilePtr getFile() const
		{
			return m_file;
		}

		operator bool() const
		{
			return static_cast<bool>(m_file);
		}

		void reset()
		{
			m_pos = 0;
		}

	protected:
		// underflow & uflow are inherited from std::streambuf

		virtual int underflow() // get character without advancing position
		{
			if (m_pos >= m_file->getSize())
				return std::char_traits<char>::eof();
			else
				return static_cast<int>(m_file->getData()[m_pos]);
		}

		virtual int uflow() // get character and advance position
		{
			int c = underflow();
			++m_pos;
			return c;
		}

	private:
		FilePtr m_file;
		size_t m_pos;
	};
};
