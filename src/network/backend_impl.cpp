/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifdef _WIN32
#include "gg/timer.hpp"
#include "backend_impl.hpp"


static uint16_t getPortFromSockaddr(SOCKADDR_STORAGE* sockaddr)
{
	switch (sockaddr->ss_family)
	{
	case AF_INET:
		return ntohs(reinterpret_cast<SOCKADDR_IN*>(sockaddr)->sin_port);
	case AF_INET6:
		return ntohs(reinterpret_cast<SOCKADDR_IN6*>(sockaddr)->sin6_port);
	default:
		return 0;
	}
}

static std::string getHostFromSockaddr(SOCKADDR_STORAGE* sockaddr)
{
	/*char str[INET6_ADDRSTRLEN];

	switch (sockaddr->ss_family)
	{
	case AF_INET:
	InetNtop(AF_INET, reinterpret_cast<SOCKADDR_IN*>(sockaddr)->sin_addr, str, sizeof(str));
	return str;
	case AF_INET6:
	InetNtop(AF_INET6, reinterpret_cast<SOCKADDR_IN6*>(sockaddr)->sin6_addr, str, sizeof(str));
	return str;
	default:
	return {};
	}*/

	char str[NI_MAXHOST];
	str[0] = '\0';
	getnameinfo(reinterpret_cast<SOCKADDR*>(sockaddr), sizeof(SOCKADDR_STORAGE),
		str, sizeof(str), NULL, 0, NI_NUMERICHOST);
	return str;
}


gg::ConnectionBackend::ConnectionBackend(const std::string& host, uint16_t port, bool tcp) :
	m_socket(INVALID_SOCKET),
	m_host(host),
	m_port(port),
	m_address(host + ":" + std::to_string(port)),
	m_tcp(tcp),
	m_connected(false)
{
}

gg::ConnectionBackend::~ConnectionBackend()
{
	disconnect();
}

bool gg::ConnectionBackend::connect(void*)
{
	if (m_connected) return false;

	std::string port = std::to_string(m_port);
	struct addrinfo hints, *result = NULL, *ptr = NULL;

	std::memset(&m_sockaddr, 0, sizeof(SOCKADDR_STORAGE));

	std::memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = m_tcp ? SOCK_STREAM : SOCK_DGRAM;
	hints.ai_protocol = m_tcp ? IPPROTO_TCP : IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	// resolve the local address and port to be used by the server
	if (getaddrinfo(m_host.c_str(), port.c_str(), &hints, &result) != 0)
	{
		return false;
	}

	m_socket = INVALID_SOCKET;

	// attempt to connect to the first possible address in the list returned by getaddrinfo
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		m_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (m_socket == INVALID_SOCKET)
		{
			continue;
		}

		if (m_tcp)
		{
			if (::connect(m_socket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
			{
				closesocket(m_socket);
				m_socket = INVALID_SOCKET;
				continue;
			}
		}
		else
		{
			if (bind(m_socket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
			{
				closesocket(m_socket);
				m_socket = INVALID_SOCKET;
				continue;
			}
		}

		// everything is OK if we get here
		break;
	}

	freeaddrinfo(result);

	if (m_socket == INVALID_SOCKET)
	{
		return false;
	}

	m_connected = true;
	return true;
}

void gg::ConnectionBackend::disconnect()
{
	if (m_connected)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		m_connected = false;
	}
}

bool gg::ConnectionBackend::isAlive() const
{
	return m_connected;
}

const std::string& gg::ConnectionBackend::getAddress() const
{
	return m_address;
}

size_t gg::ConnectionBackend::availableData()
{
	return size_t();
}

size_t gg::ConnectionBackend::waitForData(size_t len, uint32_t timeoutMs)
{
	if (!m_connected)
		return 0;

	if (timeoutMs)
	{
		size_t available_bytes = 0;
		Timer timer;

		while ((available_bytes = availableData()) < len)
		{
			int time_left = static_cast<int>(timer.peekElapsed()) - static_cast<int>(timeoutMs);
			if (time_left < 0)
				return available_bytes;

			fd_set set;
			FD_ZERO(&set);
			FD_SET(m_socket, &set);

			struct timeval timeout;
			timeout.tv_sec = time_left / 1000;
			timeout.tv_usec = (time_left % 1000) * 1000;

			int rc = select(m_socket + 1, &set, NULL, NULL, &timeout); // wait for incoming data
			if (rc == SOCKET_ERROR)
			{
				disconnect();
				return 0;
			}
		}

		return available_bytes;
	}
	else
	{
		return availableData();
	}
}

size_t gg::ConnectionBackend::peek(char* ptr, size_t len)
{
	if (!m_connected)
		return 0;

	fd_set set;
	FD_ZERO(&set);
	FD_SET(m_socket, &set);

	struct timeval timeout = { 0 };

	int rc = select(m_socket + 1, &set, NULL, NULL, &timeout);
	if (rc == SOCKET_ERROR)
	{
		disconnect();
		return 0;
	}
	else if (rc > 0)
	{
		return recv(m_socket, ptr, len, MSG_PEEK);
	}
	else
	{
		return 0;
	}
}

size_t gg::ConnectionBackend::read(char* ptr, size_t len)
{
	if (!m_connected)
		return 0;

	fd_set set;
	FD_ZERO(&set);
	FD_SET(m_socket, &set);

	struct timeval timeout = { 0 };

	int rc = select(m_socket + 1, &set, NULL, NULL, &timeout);
	if (rc == SOCKET_ERROR)
	{
		disconnect();
		return 0;
	}
	else if (rc > 0)
	{
		return recv(m_socket, ptr, len, 0);
	}
	else
	{
		return 0;
	}
}

size_t gg::ConnectionBackend::write(const char* ptr, size_t len)
{
	if (!m_connected)
		return 0;

	int rc;

	if (m_tcp)
		rc = send(m_socket, ptr, len, 0);
	else
		rc = sendto(m_socket, ptr, len, 0,
			reinterpret_cast<struct sockaddr*>(&m_sockaddr),
			sizeof(SOCKADDR_STORAGE));

	if (rc == SOCKET_ERROR)
	{
		disconnect();
		return 0;
	}
	else
	{
		return rc;
	}
}


gg::ClientBackendTCP::ClientBackendTCP(SOCKET socket, SOCKADDR_STORAGE& sockaddr) :
	m_socket(socket),
	m_sockaddr(sockaddr),
	m_connected(true)
{
	m_address = getHostFromSockaddr(&sockaddr) + ":" + std::to_string(getPortFromSockaddr(&sockaddr));
}

gg::ClientBackendTCP::~ClientBackendTCP()
{
	disconnect();
}

bool gg::ClientBackendTCP::connect(void*)
{
	return false;
}

void gg::ClientBackendTCP::disconnect()
{
	if (m_connected)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		m_connected = false;
	}
}

bool gg::ClientBackendTCP::isAlive() const
{
	return m_connected;
}

const std::string& gg::ClientBackendTCP::getAddress() const
{
	return m_address;
}

size_t gg::ClientBackendTCP::availableData()
{
	u_long bytes_available = 0;
	ioctlsocket(m_socket, FIONREAD, &bytes_available);
	return static_cast<size_t>(bytes_available);
}

size_t gg::ClientBackendTCP::waitForData(size_t len, uint32_t timeoutMs)
{
	if (!m_connected)
		return 0;

	if (timeoutMs)
	{
		size_t available_bytes = 0;
		Timer timer;

		while ((available_bytes = availableData()) < len)
		{
			int time_left = static_cast<int>(timer.peekElapsed()) - static_cast<int>(timeoutMs);
			if (time_left < 0)
				return available_bytes;

			fd_set set;
			FD_ZERO(&set);
			FD_SET(m_socket, &set);

			struct timeval timeout;
			timeout.tv_sec = time_left / 1000;
			timeout.tv_usec = (time_left % 1000) * 1000;

			int rc = select(m_socket + 1, &set, NULL, NULL, &timeout); // wait for incoming data
			if (rc == SOCKET_ERROR)
			{
				disconnect();
				return 0;
			}
		}

		return available_bytes;
	}
	else
	{
		return availableData();
	}
}

size_t gg::ClientBackendTCP::peek(char* ptr, size_t len)
{
	if (!m_connected)
		return 0;

	fd_set set;
	FD_ZERO(&set);
	FD_SET(m_socket, &set);

	struct timeval timeout = { 0 };

	int rc = select(m_socket + 1, &set, NULL, NULL, &timeout);
	if (rc == SOCKET_ERROR)
	{
		disconnect();
		return 0;
	}
	else if (rc > 0)
	{
		return recv(m_socket, ptr, len, MSG_PEEK);
	}
	else
	{
		return 0;
	}
}

size_t gg::ClientBackendTCP::read(char* ptr, size_t len)
{
	if (!m_connected)
		return 0;

	fd_set set;
	FD_ZERO(&set);
	FD_SET(m_socket, &set);

	struct timeval timeout = { 0 };

	int rc = select(m_socket + 1, &set, NULL, NULL, &timeout);
	if (rc == SOCKET_ERROR)
	{
		disconnect();
		return 0;
	}
	else if (rc > 0)
	{
		return recv(m_socket, ptr, len, 0);
	}
	else
	{
		return 0;
	}
}

size_t gg::ClientBackendTCP::write(const char* ptr, size_t len)
{
	if (!m_connected)
		return 0;

	int rc = send(m_socket, ptr, len, 0);
	if (rc == SOCKET_ERROR)
	{
		disconnect();
		return 0;
	}
	else
	{
		return rc;
	}
}


gg::ClientBackendUDP::ClientBackendUDP(SOCKET socket, SOCKADDR_STORAGE& sockaddr) :
	m_socket(socket),
	m_sockaddr(sockaddr),
	m_data_len(0),
	m_data_pos(0)
{
	m_address = getHostFromSockaddr(&sockaddr) + ":" + std::to_string(getPortFromSockaddr(&sockaddr));
}

gg::ClientBackendUDP::~ClientBackendUDP()
{
}

bool gg::ClientBackendUDP::connect(void*)
{
	return false;
}

void gg::ClientBackendUDP::disconnect()
{
}

bool gg::ClientBackendUDP::isAlive() const
{
	return false;
}

const std::string& gg::ClientBackendUDP::getAddress() const
{
	return m_address;
}

size_t gg::ClientBackendUDP::availableData()
{
	return m_data_len - m_data_pos;
}

size_t gg::ClientBackendUDP::waitForData(size_t len, uint32_t)
{
	return availableData();
}

size_t gg::ClientBackendUDP::peek(char* ptr, size_t len)
{
	if (len > m_data_len - m_data_pos)
		len = m_data_len - m_data_pos;

	std::memcpy(ptr, &m_data[m_data_pos], len);
	return len;
}

size_t gg::ClientBackendUDP::read(char* ptr, size_t len)
{
	if (len > m_data_len - m_data_pos)
		len = m_data_len - m_data_pos;

	std::memcpy(ptr, &m_data[m_data_pos], len);
	m_data_pos += len;
	return len;
}

size_t gg::ClientBackendUDP::write(const char* ptr, size_t len)
{
	if (len > Archive::BUF_SIZE - m_data_len)
		len = Archive::BUF_SIZE - m_data_len;

	std::memcpy(&m_data[m_data_len], ptr, len);
	m_data_len += len;
	return len;
}


gg::ServerBackend::ServerBackend(uint16_t port, bool tcp) :
	m_socket(INVALID_SOCKET),
	m_port(port),
	m_tcp(tcp),
	m_started(false)
{
}

gg::ServerBackend::~ServerBackend()
{
	stop();
}

bool gg::ServerBackend::start(void*)
{
	if (m_started) return false;

	std::string port = std::to_string(m_port);
	struct addrinfo hints, *result = NULL, *ptr = NULL;

	std::memset(&m_sockaddr, 0, sizeof(SOCKADDR_STORAGE));

	std::memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = m_tcp ? SOCK_STREAM : SOCK_DGRAM;
	hints.ai_protocol = m_tcp ? IPPROTO_TCP : IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	// resolve the local address and port to be used by the server
	if (getaddrinfo(NULL, port.c_str(), &hints, &result) != 0)
	{
		return false;
	}

	m_socket = INVALID_SOCKET;

	// attempt to connect to the first possible address in the list returned by getaddrinfo
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		m_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (m_socket == INVALID_SOCKET)
		{
			continue;
		}

		int no = 0;
		setsockopt(m_socket, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&no, sizeof(no));
		setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)ptr->ai_addr, (int)ptr->ai_addrlen);

		if (bind(m_socket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			continue;
		}

		if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			continue;
		}

		// everything is OK if we get here
		break;
	}

	freeaddrinfo(result);

	if (m_socket == INVALID_SOCKET)
	{
		return false;
	}

	m_started = true;
	return true;
}

void gg::ServerBackend::stop()
{
	closesocket(m_socket);
	m_socket = INVALID_SOCKET;
	m_started = false;
}

bool gg::ServerBackend::isAlive() const
{
	return m_started;
}

std::unique_ptr<gg::IConnectionBackend> gg::ServerBackend::getNextConnection(uint32_t timeoutMs)
{
	std::unique_ptr<IConnectionBackend> client;

	if (!m_started)
		return {};

	SOCKADDR_STORAGE addr;
	int addrlen = sizeof(SOCKADDR_STORAGE);

	fd_set set;
	FD_ZERO(&set);
	FD_SET(m_socket, &set);

	struct timeval timeout;
	timeout.tv_sec = timeoutMs / 1000;
	timeout.tv_usec = (timeoutMs % 1000) * 1000;

	int rc = select(m_socket + 1, &set, NULL, NULL, &timeout);
	if (rc == SOCKET_ERROR)
	{
		stop();
	}
	else if (rc > 0)
	{
		if (m_tcp) // we are TCP
		{
			SOCKET sock = accept(m_socket, reinterpret_cast<struct sockaddr*>(&addr), &addrlen);
			if (sock == INVALID_SOCKET)
			{
				stop();
				return {};
			}

			client.reset(new ClientBackendTCP(sock, addr));
		}
		else // we are UDP
		{
			char buf[2048];
			int len = recvfrom(m_socket, buf, sizeof(buf), 0, reinterpret_cast<struct sockaddr*>(&addr), &addrlen);

			client.reset(new ClientBackendUDP(m_socket, addr));
			client->write(buf, len);
		}
	}

	return std::move(client);
}

#endif // _WIN32
