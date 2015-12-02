/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once
#pragma warning (disable : 4250)

#include <mutex>
#include <vector>
#include "stream_impl.hpp"
#include "gg/network.hpp"

namespace gg
{
	class Packet : public Stream, public IPacket
	{
	public:
		Packet(Mode mode, Type type);
		virtual ~Packet();
		virtual Type getType() const;
		virtual const char* getData() const;
		virtual size_t getSize() const;
		virtual size_t write(const char* ptr, size_t len);
		virtual size_t read(char* ptr, size_t len);

		// for internal use
		char* getDataPtr();
		void setSize(size_t);

	protected:
		char m_data[BUF_SIZE];
		size_t m_data_len;
		size_t m_data_pos;

	private:
		Type m_type;
	};

	class Connection : public IConnection
	{
	public:
		Connection(ConnectionBackendPtr&&);
		virtual ~Connection();
		virtual bool connect(void* user_data = nullptr);
		virtual void disconnect();
		virtual bool isAlive() const;
		virtual const std::string& getAddress() const;
		virtual PacketPtr getNextPacket(uint32_t timeoutMs = 0);
		virtual PacketPtr createPacket(IPacket::Type) const;
		virtual PacketPtr createPacket(EventPtr) const;
		virtual bool send(PacketPtr);

	private:
		struct StreamHeader
		{
			uint16_t packet_size;
			IPacket::Type packet_type;
		};

		struct StreamTail
		{
			uint32_t n = 0;
			bool ok() { return n == 0; }
		};

		mutable std::recursive_mutex m_mutex;
		ConnectionBackendPtr m_backend;
	};

	class Server : public IServer
	{
	public:
		Server(ServerBackendPtr&&);
		virtual ~Server();
		virtual bool start(void* user_data = nullptr);
		virtual void stop();
		virtual bool isAlive() const;
		virtual ConnectionPtr getNextConnection(uint32_t timeoutMs = 0);
		virtual void closeConnections();

	private:
		mutable std::recursive_mutex m_mutex;
		ServerBackendPtr m_backend;
		std::vector<std::weak_ptr<IConnection>> m_clients;
	};

	class NetworkException : public INetworkException
	{
	public:
		NetworkException(const char* what);
		virtual ~NetworkException();
		virtual const char* what() const;

	private:
		const char* m_what;
	};

	class NetworkManager : public INetworkManager
	{
	public:
		NetworkManager();
		virtual ~NetworkManager();
		virtual ConnectionPtr createConnection(const std::string& host, uint16_t port) const;
		virtual ConnectionPtr createConnection(ConnectionBackendPtr&&) const;
		virtual ServerPtr createServer(uint16_t port) const;
		virtual ServerPtr createServer(ServerBackendPtr&&) const;
		virtual PacketPtr createPacket(IPacket::Type) const;
		virtual PacketPtr createPacket(EventPtr) const;
	};
};
