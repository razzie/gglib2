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

		bool addDirectory(const std::string& dir_name);
		bool addVirtualDirectory(const std::string& archive_name);
		//bool createVirtualDirectory(const std::string& dir_name, const std::string& archive_name);
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

				Type type;
				std::shared_ptr<IFile> file;
				std::shared_ptr<IDirectory> directory;
			};

			typedef std::vector<FileOrDirectory>::iterator Iterator;

			virtual ~IDirectory() {}
			virtual const std::string& getName() const = 0;
			virtual Iterator begin() = 0;
			virtual Iterator end() = 0;
		};

		class IFile
		{
		public:
			virtual ~IFile() {}
			virtual const std::string& getName() const = 0;
			virtual const char* getData() const = 0;
			virtual size_t getSize() const = 0;
			virtual void unload() = 0;
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

			FileStream(const FileStream&) = delete;

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
