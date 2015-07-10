/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include "gg/network.hpp"

namespace gg
{
	class ConnectionBackend : public IConnectionBackend
	{
	public:
		ConnectionBackend(const std::string& address, uint16_t port);
		virtual ~ConnectionBackend();
		virtual bool connect(void* user_data = nullptr);
		virtual void disconnect();
		virtual size_t availableData() const;
		virtual size_t waitForData(size_t len, uint32_t timeoutMs = 0) const;
		virtual size_t peek(char* ptr, size_t len) const;
		virtual size_t read(char* ptr, size_t len);
		virtual size_t write(const char* ptr, size_t len);

	private:
		SOCKET m_socket;
		SOCKADDR_STORAGE m_sockaddr;
		std::string m_address;
		uint16_t m_port;
		bool m_connected;
	};

	class ClientBackend : public IConnectionBackend
	{
	public:
		ClientBackend(SOCKET, SOCKADDR_STORAGE*);
		virtual ~ClientBackend();
		virtual bool connect(void* user_data = nullptr);
		virtual void disconnect();
		virtual size_t availableData() const;
		virtual size_t waitForData(size_t len, uint32_t timeoutMs = 0) const;
		virtual size_t peek(char* ptr, size_t len) const;
		virtual size_t read(char* ptr, size_t len);
		virtual size_t write(const char* ptr, size_t len);

	private:
		SOCKET m_socket;
		SOCKADDR_STORAGE m_sockaddr;
		bool m_connected;
	};

	class ServerBackend : public IServerBackend
	{
	public:
		ServerBackend(uint16_t port);
		virtual ~ServerBackend();
		virtual bool start(void* user_data = nullptr);
		virtual void stop();
		virtual std::unique_ptr<IConnectionBackend>&& getNextConnection(uint32_t timeoutMs = 0);

	private:
		SOCKET m_socket;
		SOCKADDR_STORAGE m_sockaddr;
		uint16_t m_port;
		bool m_started;
	};
};

#endif // _WIN32
