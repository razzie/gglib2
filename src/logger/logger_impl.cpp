#include <iomanip>
#include <chrono>
#include <fstream>
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#endif
#include "logger_impl.hpp"

#if defined _MSC_VER
#define THREAD_LOCAL __declspec(thread)
#elif defined __GNUG__
#define THREAD_LOCAL __thread
#else
#define THREAD_LOCAL thread_local
#endif

static gg::Logger s_logger;
gg::ILogger& gg::log = s_logger;

static THREAD_LOCAL std::string* thread_buffer = nullptr;


static int getTimeZoneBiasInMinutes() // bias = UTC - local time
{
#ifdef _WIN32
	TIME_ZONE_INFORMATION tzi;
	GetTimeZoneInformation(&tzi);
	return tzi.Bias;
#else
	return 0;
#endif
}

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
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

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

	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
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
		auto timezone_bias = getTimeZoneBiasInMinutes();

		sec -= timezone_bias * 60;

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
		std::lock_guard<decltype(m_mutex)> guard(m_mutex);
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


gg::OstreamManipulator<const char*> gg::format(const char* f)
{
	OstreamManipulator<const char*>::Manipulator m =
		[](std::ostream& os, const char* fmt) -> std::ostream&
	{
		std::locale& loc = os.getloc();
		int i = 0;
		while (fmt[i] != 0)
		{
			if (fmt[i] != '%')
			{
				os << fmt[i];
				i++;
			}
			else
			{
				i++;
				if (fmt[i] == '%')
				{
					os << fmt[i];
					i++;
				}
				else
				{
					bool ok = true;
					int istart = i;
					bool more = true;
					int width = 0;
					int precision = 6;
					std::ios_base::fmtflags flags;
					char fill = ' ';
					bool alternate = false;
					while (more)
					{
						switch (fmt[i])
						{
						case '+':
							flags |= std::ios::showpos;
							break;
						case '-':
							flags |= std::ios::left;
							break;
						case '0':
							flags |= std::ios::internal;
							fill = '0';
							break;
						case '#':
							alternate = true;
							break;
						case ' ':
							break;
						default:
							more = false;
							break;
						}
						if (more) i++;
					}
					if (std::isdigit(fmt[i], loc))
					{
						width = std::atoi(fmt + i);
						do i++;
						while (std::isdigit(fmt[i], loc));
					}
					if (fmt[i] == '.')
					{
						i++;
						precision = std::atoi(fmt + i);
						while (std::isdigit(fmt[i], loc)) i++;
					}
					switch (fmt[i])
					{
					case 'd':
						flags |= std::ios::dec;
						break;
					case 'x':
						flags |= std::ios::hex;
						if (alternate) flags |= std::ios::showbase;
						break;
					case 'X':
						flags |= std::ios::hex | std::ios::uppercase;
						if (alternate) flags |= std::ios::showbase;
						break;
					case 'o':
						flags |= std::ios::hex;
						if (alternate) flags |= std::ios::showbase;
						break;
					case 'f':
						flags |= std::ios::fixed;
						if (alternate) flags |= std::ios::showpoint;
						break;
					case 'e':
						flags |= std::ios::scientific;
						if (alternate) flags |= std::ios::showpoint;
						break;
					case 'E':
						flags |= std::ios::scientific | std::ios::uppercase;
						if (alternate) flags |= std::ios::showpoint;
						break;
					case 'g':
						if (alternate) flags |= std::ios::showpoint;
						break;
					case 'G':
						flags |= std::ios::uppercase;
						if (alternate) flags |= std::ios::showpoint;
						break;
					default:
						ok = false;
						break;
					}
					i++;
					if (fmt[i] != 0) ok = false;
					if (ok)
					{
						os.unsetf(std::ios::adjustfield | std::ios::basefield |
							std::ios::floatfield);
						os.setf(flags);
						os.width(width);
						os.precision(precision);
						os.fill(fill);
					}
					else i = istart;
				}
			}
		}
		return os;
	};

	return OstreamManipulator<const char*>(m, f);
}
