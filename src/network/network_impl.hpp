/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <mutex>
#include "gg/network.hpp"

namespace gg
{
	class Packet : public IPacket
	{
	public:
		static const size_t BUF_SIZE = 1024;

		Packet(Mode, Type);
		virtual ~Packet();
		virtual Mode mode() const;
		virtual Type type() const;
		char* data(); // for internal use
		virtual const char* data() const;
		virtual size_t length() const;

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
		virtual Packet& operator& (IBlob&);
		virtual Packet& operator& (ISerializable&);

	protected:
		Mode m_mode;
		Type m_type;
		char m_data[BUF_SIZE];
		size_t m_data_len;
		size_t m_data_pos;

		size_t write(const char* ptr, size_t len);
		size_t read(char* ptr, size_t len);
	};

	class PacketException : public IPacketException
	{
	public:
		PacketException(const char* what);
		virtual ~PacketException();
		virtual const char* what() const;

	private:
		const char* m_what;
	};


	class Connection : public IConnection
	{
	public:
		Connection(std::unique_ptr<IConnectionBackend>&&);
		virtual ~Connection();
		virtual bool connect(void* user_data = nullptr);
		virtual void disconnect();
		virtual std::shared_ptr<IPacket> getNextPacket(uint32_t timeoutMs = 0); // 0: non-blocking
		virtual std::shared_ptr<IPacket> createPacket(IPacket::Type) const;
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
		virtual std::shared_ptr<IConnection> getNextConnection(uint32_t timeoutMs = 0); // 0: non-blocking

	private:
		std::unique_ptr<IServerBackend> m_backend;
	};


	class NetworkManager : public INetworkManager
	{
	public:
		NetworkManager();
		virtual ~NetworkManager();
		virtual std::shared_ptr<IPacket> createPacket(IPacket::Type) const;
		virtual std::shared_ptr<IConnection> createConnection(const std::string& address, uint16_t port) const;
		virtual std::shared_ptr<IConnection> createConnection(std::unique_ptr<IConnectionBackend>&&) const;
		virtual std::shared_ptr<IServer> createServer(uint16_t port) const;
		virtual std::shared_ptr<IServer> createServer(std::unique_ptr<IServerBackend>&&) const;
	};
};
