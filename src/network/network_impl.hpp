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
#include "archive_impl.hpp"
#include "gg/network.hpp"

namespace gg
{
	class Packet : public IPacket, public Archive
	{
	public:
		Packet(Mode mode, Type type) :
			Archive(mode),
			m_type(type)
		{
		}

		virtual ~Packet() = default;

		virtual Type getType() const
		{
			return m_type;
		}

	private:
		Type m_type;
	};

	class Connection : public IConnection
	{
	public:
		Connection(std::unique_ptr<IConnectionBackend>&&);
		virtual ~Connection();
		virtual bool connect(void* user_data = nullptr);
		virtual void disconnect();
		virtual bool isAlive() const;
		virtual const std::string& getAddress() const;
		virtual std::shared_ptr<IPacket> getNextPacket(uint32_t timeoutMs = 0);
		virtual std::shared_ptr<IPacket> createPacket(IPacket::Type) const;
		virtual std::shared_ptr<IPacket> createPacket(EventPtr) const;
		virtual bool send(std::shared_ptr<IPacket>);

	private:
		struct ArchiveHeader
		{
			uint16_t packet_size;
			IPacket::Type packet_type;
		};

		struct ArchiveTail
		{
			uint32_t n = 0;
			bool ok() { return n == 0; }
		};

		mutable std::recursive_mutex m_mutex;
		std::unique_ptr<IConnectionBackend> m_backend;
	};

	class Server : public IServer
	{
	public:
		Server(std::unique_ptr<IServerBackend>&&);
		virtual ~Server();
		virtual bool start(void* user_data = nullptr);
		virtual void stop();
		virtual bool isAlive() const;
		virtual std::shared_ptr<IConnection> getNextConnection(uint32_t timeoutMs = 0);
		virtual void closeConnections();

	private:
		mutable std::recursive_mutex m_mutex;
		std::unique_ptr<IServerBackend> m_backend;
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
		virtual std::shared_ptr<IConnection> createConnection(const std::string& host, uint16_t port) const;
		virtual std::shared_ptr<IConnection> createConnection(std::unique_ptr<IConnectionBackend>&&) const;
		virtual std::shared_ptr<IServer> createServer(uint16_t port) const;
		virtual std::shared_ptr<IServer> createServer(std::unique_ptr<IServerBackend>&&) const;
		virtual std::shared_ptr<IPacket> createPacket(IPacket::Type) const;
		virtual std::shared_ptr<IPacket> createPacket(EventPtr) const;
	};
};
