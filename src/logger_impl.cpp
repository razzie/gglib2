#include <iomanip>
#include <chrono>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "logger_impl.hpp"

#if defined _MSC_VER
#define THREAD_LOCAL __declspec(thread)
#elif defined __GNUG__
#define THREAD_LOCAL __thread
#endif

static gg::Logger s_logger;
gg::ILogger& gg::log = s_logger;

static THREAD_LOCAL std::string* thread_buffer = nullptr;


gg::Logger::Logger() :
	std::ostream(this),
	m_timestamp(Timestamp::ELAPSED_TIME)
{
	redirect(std::cout);
}

gg::Logger::~Logger()
{
}

void gg::Logger::setTimestamp(gg::ILogger::Timestamp t)
{
	m_timestamp = t;
}

void gg::Logger::redirect(std::ostream& o)
{
	std::shared_ptr<std::ostream> ptr( new std::ostream(o.rdbuf()) );
	redirect(ptr);
}

void gg::Logger::redirect(std::shared_ptr<std::ostream> ptr)
{
	std::lock_guard<gg::FastMutex> guard(m_mutex);

	if (ptr->rdbuf() == this)
		throw std::runtime_error("gg::log redirected to itself");

	m_output = ptr;
}

void gg::Logger::redirect(const std::string& file_name)
{
	std::shared_ptr<std::ostream> ptr(new std::fstream(file_name, std::ios::app));
	if (static_cast<std::fstream&>(*ptr).is_open())
		redirect(ptr);
}

void gg::Logger::write(const std::string& str) const
{
	using namespace std::chrono;

	size_t len = str.length();
	for (; len && str[len-1] == '\n'; --len);

	if (len == 0) return;

	std::lock_guard<gg::FastMutex> guard(m_mutex);
	std::stringstream stamp;

	if (m_timestamp == Timestamp::ELAPSED_TIME)
	{
		uint64_t elapsed = m_timer.peekElapsed();

		stamp << std::setfill('0') << "["
			<< std::setw(2) << elapsed / 60000 << ":"
			<< std::setw(2) << (elapsed / 1000) % 60 << ":"
			<< std::setw(3) << elapsed % 1000 << "] ";
	}
	else if (m_timestamp == Timestamp::SYSTEM_TIME)
	{
		auto sec = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();

		stamp << std::setfill('0') << "["
			<< std::setw(2) << (sec / 3600) % 24 << ":"
			<< std::setw(2) << (sec / 60) % 60 << ":"
			<< std::setw(2) << sec % 60 << "] ";
	}

	*m_output << stamp.rdbuf();
	m_output->write(str.c_str(), len);
	*m_output << std::endl;
}

int gg::Logger::overflow(int c)
{
	if (thread_buffer == nullptr)
	{
		std::lock_guard<gg::FastMutex> guard(m_mutex);
		thread_buffer = &m_buffer[std::this_thread::get_id()];
	}

	char ch = c;
	thread_buffer->append(&ch, 1);

	return c;
}

int gg::Logger::sync()
{
	if (thread_buffer != nullptr)
	{
		write(*thread_buffer);
		thread_buffer->clear();
	}

	return 0;
}
