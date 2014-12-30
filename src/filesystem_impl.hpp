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
			virtual bool init() = 0;
			virtual const std::string& getName() const = 0;
			virtual std::shared_ptr<IDirectory> getDirectory(const std::string& dir_name) = 0;
			virtual std::shared_ptr<IFile> getFile(const std::string& file_name) = 0;
			virtual bool loadDirectoryData(const std::string& dir_name, std::vector<IDirectory::FileOrDirectory>*) = 0;
			virtual bool loadFileData(const std::string& file_name, const char**, size_t*) = 0;
		};

		class DirectoryArchive : public IArchive
		{
		private:
			mutable std::mutex m_mutex;
			std::string m_name;
			std::string m_full_path;
			std::map<std::string, std::weak_ptr<IFile>> m_files;

		public:
			DirectoryArchive(const std::string& dir_name);
			virtual bool init();
			virtual const std::string& getName() const;
			virtual std::shared_ptr<IDirectory> getDirectory(const std::string& dir_name);
			virtual std::shared_ptr<IFile> getFile(const std::string& file_name);
			virtual bool loadDirectoryData(const std::string& dir_name, std::vector<IDirectory::FileOrDirectory>*);
			virtual bool loadFileData(const std::string& file_name, const char**, size_t*);
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
				std::weak_ptr<IFile> ptr;
			};

			mutable std::mutex m_mutex;
			mutable std::ifstream m_file;
			std::string m_name;
			std::map<std::string, FileData> m_files;

		public:
			VirtualArchive(const std::string& archive_name);
			virtual ~VirtualArchive();
			virtual bool init();
			virtual const std::string& getName() const;
			virtual std::shared_ptr<IDirectory> getDirectory(const std::string& dir_name);
			virtual std::shared_ptr<IFile> getFile(const std::string& file_name);
			virtual bool loadDirectoryData(const std::string& dir_name, std::vector<IDirectory::FileOrDirectory>*);
			virtual bool loadFileData(const std::string& file_name, const char**, size_t*);
		};

		class Directory : public IDirectory
		{
		private:
			std::string m_name;
			std::vector<FileOrDirectory> m_files;
			IArchive* m_archive;

		public:
			Directory(IArchive* archive, std::string& name) :
				m_name(name), m_archive(archive)
			{
				m_archive->loadDirectoryData(m_name.substr(m_name.find('/') + 1), &m_files);
			}

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

			virtual ConstIterator begin() const
			{
				return m_files.begin();
			}

			virtual ConstIterator end() const
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
					m_archive->loadFileData(m_name.substr(m_name.find('/') + 1), &m_data, &m_size);

				return m_data;
			}

			virtual size_t getSize() const
			{
				if (m_data == nullptr)
					m_archive->loadFileData(m_name.substr(m_name.find('/') + 1), &m_data, &m_size);

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
