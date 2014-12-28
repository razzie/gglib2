#ifndef GG_FILESYSTEM_IMPL_HPP_INCLUDED
#define GG_FILESYSTEM_IMPL_HPP_INCLUDED

#include <fstream>
#include <map>
#include <mutex>
#include "gg/filesystem.hpp"

namespace gg
{
	namespace fs
	{
		class IArchive
		{
		public:
			virtual ~IArchive() {}
			virtual bool init() const = 0;
			virtual const std::string& getName() const = 0;
			virtual bool loadDirectoryData(std::vector<IDirectory::FileOrDirectory>*) const = 0;
			virtual bool loadFileData(const std::string& file_name, const char**, size_t*) const = 0;
		};

		class DirectoryArchive : public IArchive
		{
		private:
			std::mutex m_mutex;
			std::string m_name;
			std::string m_dir_name;

		public:
			DirectoryArchive(const std::string& dir_name);
			DirectoryArchive(const DirectoryArchive&) = delete;
			virtual bool init() const;
			virtual const std::string& getName() const;
			virtual bool loadDirectoryData(std::vector<IDirectory::FileOrDirectory>*) const;
			virtual bool loadFileData(const std::string& file_name, const char**, size_t*) const;
		};

		class VirtualArchive : public IArchive
		{
		private:
			struct FileData
			{
				std::string file_name;
				size_t start_pos;
				size_t original_size;
				size_t compressed_size;
			};

			mutable std::mutex m_mutex;
			std::ifstream m_file;
			std::string m_name;
			std::vector<FileData> m_filedata;
			std::map<std::string, std::vector<FileData>::iterator> m_filemap;

		public:
			VirtualArchive(const std::string& archive_name);
			VirtualArchive(const VirtualArchive&) = delete;
			virtual ~VirtualArchive();
			virtual bool init() const;
			virtual const std::string& getName() const;
			virtual bool loadDirectoryData(std::vector<IDirectory::FileOrDirectory>*) const;
			virtual bool loadFileData(const std::string& file_name, const char**, size_t*) const;
		};

		class Directory : public IDirectory
		{
		private:
			std::string m_name;
			std::vector<FileOrDirectory> m_files;
			IArchive* m_archive;

		public:
			Directory(IArchive* archive, const std::string& name) :
				m_name(name), m_archive(archive)
			{
				m_archive->loadDirectoryData(&m_files);
			}

			Directory(const Directory&) = delete;

			virtual const std::string& getName() const
			{
				return m_name;
			}

			virtual Iterator begin()
			{
				return m_files.begin();
			}

			virtual Iterator end()
			{
				return m_files.end();
			}
		};

		class File : public IFile
		{
		private:
			std::string m_name;
			mutable const char* m_data;
			mutable size_t m_size;
			IArchive* m_archive;

		public:
			File(IArchive* archive, const std::string& name) :
				m_name(name), m_data(nullptr), m_size(0), m_archive(archive)
			{
			}

			File(const File&) = delete;

			virtual ~File()
			{
				unload();
			}

			virtual const std::string& getName() const
			{
				return m_name;
			}

			virtual const char* getData() const
			{
				if (m_data == nullptr)
					m_archive->loadFileData(m_name, &m_data, &m_size);

				return m_data;
			}

			virtual size_t getSize() const
			{
				if (m_data == nullptr)
					m_archive->loadFileData(m_name, &m_data, &m_size);

				return m_size;
			}

			virtual void unload()
			{
				if (m_data != nullptr)
				{
					delete[] m_data;
					m_data = nullptr;
					m_size = 0;
				}
			}
		};
	};
};

#endif // GG_FILESYSTEM_IMPL_HPP_INCLUDED
