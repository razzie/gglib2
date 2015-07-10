/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "backend_impl.hpp"


static uint16_t getPortFromSockaddr(SOCKADDR_STORAGE* sockaddr)
{
	switch (sockaddr->ss_family)
	{
	case AF_INET:
		return reinterpret_cast<SOCKADDR_IN*>(sockaddr)->sin_port;
	case AF_INET6:
		return reinterpret_cast<SOCKADDR_IN6*>(sockaddr)->sin6_port;
	default:
		return 0;
	}
}

static std::string&& getAddrFromSockaddr(SOCKADDR_STORAGE* sockaddr)
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


gg::ConnectionBackend::ConnectionBackend(const std::string& address, uint16_t port) :
	m_socket(INVALID_SOCKET),
	m_address(address),
	m_port(port),
	m_connected(false)
{
}

gg::ConnectionBackend::~ConnectionBackend()
{
}

bool gg::ConnectionBackend::connect(void*)
{
	return false;
}

void gg::ConnectionBackend::disconnect()
{
}

size_t gg::ConnectionBackend::availableData() const
{
	return size_t();
}

size_t gg::ConnectionBackend::waitForData(size_t len, uint32_t timeoutMs) const
{
	return size_t();
}

size_t gg::ConnectionBackend::peek(char* ptr, size_t len) const
{
	return size_t();
}

size_t gg::ConnectionBackend::read(char* ptr, size_t len)
{
	return size_t();
}

size_t gg::ConnectionBackend::write(const char* ptr, size_t len)
{
	return size_t();
}


gg::ClientBackend::ClientBackend(SOCKET socket, SOCKADDR_STORAGE* sockaddr) :
	m_socket(socket),
	m_sockaddr(*sockaddr),
	m_connected(false)
{
}

gg::ClientBackend::~ClientBackend()
{
}

bool gg::ClientBackend::connect(void*)
{
	return false;
}

void gg::ClientBackend::disconnect()
{
}

size_t gg::ClientBackend::availableData() const
{
	return size_t();
}

size_t gg::ClientBackend::waitForData(size_t len, uint32_t timeoutMs) const
{
	return size_t();
}

size_t gg::ClientBackend::peek(char* ptr, size_t len) const
{
	return size_t();
}

size_t gg::ClientBackend::read(char* ptr, size_t len)
{
	return size_t();
}

size_t gg::ClientBackend::write(const char* ptr, size_t len)
{
	return size_t();
}


gg::ServerBackend::ServerBackend(uint16_t port) :
	m_socket(INVALID_SOCKET),
	m_started(false)
{
}

gg::ServerBackend::~ServerBackend()
{
}

bool gg::ServerBackend::start(void*)
{
	return false;
}

void gg::ServerBackend::stop()
{
}

std::unique_ptr<gg::IConnectionBackend>&& gg::ServerBackend::getNextConnection(uint32_t timeoutMs)
{
	return std::move(std::unique_ptr<IConnectionBackend>());
}
