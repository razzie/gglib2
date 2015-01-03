/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

/**
 * HOW TO USE:
 * -----------
 *
 * Let's say we have a 'textures.pak' named virtual directory in 'media' folder.
 * It contains 3 files: 'ground.png', 'water.png' and 'sky.png'.
 *
 * 1st step:
 * Add the virtual directory by calling: gg::fs::addVirtualDirectory("media/textures.pak");
 *
 * 2nd step:
 * We wan't to access 'sky.png' for example. As it is a part of 'textures.pak' virtual directory,
 * we should open the file by using the 'textures.pak/sky.png' name.
 *
 * Important:
 * Do NOT use backslash '\' characters in a file path. Always use slash '/' instead.
 */

#ifndef GG_FILESYSTEM_HPP_INCLUDED
#define GG_FILESYSTEM_HPP_INCLUDED

#include <memory>
#include <string>
#include <vector>

namespace gg
{
	namespace fs
	{
		class IDirectory;
		class IFile;

		bool addVirtualDirectory(const std::string& vdir_path);
		std::shared_ptr<IDirectory> openDirectory(const std::string& dir_name);
		std::shared_ptr<IFile> openFile(const std::string& file_name);

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

			virtual ~IDirectory() {}
			virtual const std::string& getName() const = 0;
			virtual Iterator begin() = 0;
			virtual Iterator end() = 0;
			virtual ConstIterator begin() const = 0;
			virtual ConstIterator end() const = 0;
		};

		class IFile
		{
		public:
			virtual ~IFile() {}
			virtual const std::string& getName() const = 0;
			virtual const char* getData() const = 0;
			virtual size_t getSize() const = 0;
			virtual void unload() = 0; // can cause crash if other thread is reading file data!
		};

		class FileStream : public std::istream, public std::streambuf
		{
		private:
			std::shared_ptr<IFile> m_file;
			size_t m_pos;

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

		public:
			FileStream(const std::string& file_name) :
				std::istream(this), m_file(openFile(file_name)),
				m_pos(0)
			{
			}

			std::shared_ptr<IFile> getFile() const
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
		};
	};
};

#endif // GG_FILESYSTEM_HPP_INCLUDED
