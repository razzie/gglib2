#include "filesystem_impl.hpp"

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


bool gg::fs::addDirectory(const std::string& dir_name)
{
	IArchive* ar = new DirectoryArchive(dir_name);

	if (ar->init())
	{
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
		return std::shared_ptr<gg::fs::IDirectory>(new gg::fs::Directory(it->second, dir_name));
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
		return std::shared_ptr<gg::fs::File>(new gg::fs::File(it->second, file_name));
	}
	else
	{
		return {};
	}
}


gg::fs::DirectoryArchive::DirectoryArchive(const std::string& dir_name) :
	m_name(getPathEnd(dir_name)), m_dir_name(dir_name)
{
}

bool gg::fs::DirectoryArchive::init() const
{
	return false;
}

const std::string& gg::fs::DirectoryArchive::getName() const
{
	return m_name;
}

bool gg::fs::DirectoryArchive::loadDirectoryData(std::vector<IDirectory::FileOrDirectory>* files) const
{
	return false;
}

bool gg::fs::DirectoryArchive::loadFileData(const std::string& file_name, const char** data, size_t* size) const
{
	return false;
}


gg::fs::VirtualArchive::VirtualArchive(const std::string& archive_name) :
	m_name(getPathEnd(archive_name))
{
}

gg::fs::VirtualArchive::~VirtualArchive()
{
}

bool gg::fs::VirtualArchive::init() const
{
	return false;
}

const std::string& gg::fs::VirtualArchive::getName() const
{
	return m_name;
}

bool gg::fs::VirtualArchive::loadDirectoryData(std::vector<IDirectory::FileOrDirectory>* files) const
{
	return false;
}

bool gg::fs::VirtualArchive::loadFileData(const std::string& file_name, const char** data, size_t* size) const
{
	return false;
}
