/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <mutex>
#include <vector>
#include "gg/network.hpp"

namespace gg
{
	class Packet : public IPacket
	{
	public:
		static const size_t BUF_SIZE = 2048;

		Packet(Mode, Type);
		virtual ~Packet();
		virtual Mode getMode() const;
		virtual Type getType() const;
		virtual const char* getData() const;
		virtual size_t getSize() const;

		virtual Packet& operator& (int8_t&);
		virtual Packet& operator& (int16_t&);
		virtual Packet& operator& (int32_t&);
		virtual Packet& operator& (int64_t&);
		virtual Packet& operator& (uint8_t&);
		virtual Packet& operator& (uint16_t&);
		virtual Packet& operator& (uint32_t&);
		virtual Packet& operator& (uint64_t&);
		virtual Packet& operator& (float&);
		virtual Packet& operator& (double&);
		virtual Packet& operator& (std::string&);
		virtual Packet& operator& (ISerializable&);

		virtual size_t write(const char* ptr, size_t len);
		virtual size_t read(char* ptr, size_t len);

		// for internal use
		char* getDataPtr();
		void setSize(size_t);

	protected:
		Mode m_mode;
		Type m_type;
		char m_data[BUF_SIZE];
		size_t m_data_len;
		size_t m_data_pos;
	};

	class SerializationError : public ISerializationError
	{
	public:
		SerializationError() = default;
		virtual ~SerializationError() = default;
		virtual const char* what() const;
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
		virtual std::shared_ptr<IPacket> createPacket(std::shared_ptr<IEvent>) const;
		virtual bool send(std::shared_ptr<IPacket>);

	private:
		struct PacketHeader
		{
			uint16_t packet_size;
			IPacket::Type packet_type;
		};

		struct PacketTail
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
		virtual std::shared_ptr<IPacket> createPacket(std::shared_ptr<IEvent>) const;
	};
};
