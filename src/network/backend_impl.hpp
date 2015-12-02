/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once
#pragma warning (disable : 4250)

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
typedef int SOCKET;
typedef int SOCKADDR_STORAGE;
#endif // _WIN32

#include "network_impl.hpp"

namespace gg
{
	class ConnectionBackend : public IConnectionBackend
	{
	public:
		ConnectionBackend(const std::string& host, uint16_t port, bool tcp = true);
		virtual ~ConnectionBackend();
		virtual bool connect(void* user_data = nullptr);
		virtual void disconnect();
		virtual bool isAlive() const;
		virtual const std::string& getAddress() const;
		virtual size_t availableData();
		virtual size_t waitForData(size_t len, uint32_t timeoutMs = 0);
		virtual size_t peek(char* ptr, size_t len);
		virtual size_t read(char* ptr, size_t len);
		virtual size_t write(const char* ptr, size_t len);

	private:
		SOCKET m_socket;
		SOCKADDR_STORAGE m_sockaddr;
		std::string m_host;
		uint16_t m_port;
		std::string m_address; // host + port
		bool m_tcp;
		bool m_connected;
	};

	class ClientBackendTCP : public IConnectionBackend
	{
	public:
		ClientBackendTCP(SOCKET, SOCKADDR_STORAGE&);
		virtual ~ClientBackendTCP();
		virtual bool connect(void* user_data = nullptr);
		virtual void disconnect();
		virtual bool isAlive() const;
		virtual const std::string& getAddress() const;
		virtual size_t availableData();
		virtual size_t waitForData(size_t len, uint32_t timeoutMs = 0);
		virtual size_t peek(char* ptr, size_t len);
		virtual size_t read(char* ptr, size_t len);
		virtual size_t write(const char* ptr, size_t len);

	private:
		SOCKET m_socket;
		SOCKADDR_STORAGE m_sockaddr;
		std::string m_address; // host + port
		bool m_connected;
	};

	class ClientBackendUDP : public IConnectionBackend
	{
	public:
		ClientBackendUDP(SOCKET, SOCKADDR_STORAGE&);
		virtual ~ClientBackendUDP();
		virtual bool connect(void* user_data = nullptr);
		virtual void disconnect();
		virtual bool isAlive() const;
		virtual const std::string& getAddress() const;
		virtual size_t availableData();
		virtual size_t waitForData(size_t len, uint32_t timeoutMs = 0);
		virtual size_t peek(char* ptr, size_t len);
		virtual size_t read(char* ptr, size_t len);
		virtual size_t write(const char* ptr, size_t len);

	private:
		SOCKET m_socket;
		SOCKADDR_STORAGE m_sockaddr;
		std::string m_address; // host + port
		char m_data[Stream::BUF_SIZE];
		size_t m_data_len;
		size_t m_data_pos;
	};

	class ServerBackend : public IServerBackend
	{
	public:
		ServerBackend(uint16_t port, bool tcp = true);
		virtual ~ServerBackend();
		virtual bool start(void* user_data = nullptr);
		virtual void stop();
		virtual bool isAlive() const;
		virtual ConnectionBackendPtr getNextConnection(uint32_t timeoutMs = 0);

	private:
		SOCKET m_socket;
		SOCKADDR_STORAGE m_sockaddr;
		uint16_t m_port;
		bool m_tcp;
		bool m_started;
	};
};
