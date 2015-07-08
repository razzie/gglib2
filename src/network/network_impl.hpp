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
		virtual Mode getMode() const;
		virtual Type getType() const;
		virtual const char* getData() const;
		virtual size_t getDataLen() const;

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

	protected:
		Mode m_mode;
		Type m_type;
		char m_data[BUF_SIZE];
		size_t m_data_len;
		size_t m_data_pos;

		size_t write(const char* ptr, size_t len);
		size_t read(char* ptr, size_t len);
	};


	class Connection : public IConnection
	{
	public:
		Connection(std::unique_ptr<IConnectionBackend>&&);
		virtual ~Connection();
		virtual bool connect(const std::string& args);
		virtual void disconnect();
		virtual const std::string& getDescription() const;
		virtual bool packetAvailable() const;
		virtual std::shared_ptr<IPacket> getNextPacket();
		virtual std::shared_ptr<IPacket> waitForNextPacket(uint32_t timeoutMs = 0);
		virtual std::shared_ptr<IPacket> createPacket(IPacket::Type) const;
		virtual bool send(std::shared_ptr<IPacket>);

	private:
		mutable std::recursive_mutex m_mutex;
		std::unique_ptr<IConnectionBackend> m_backend;
	};


	class NetworkManager : public INetworkManager
	{
	public:
		NetworkManager();
		virtual ~NetworkManager();
		virtual std::shared_ptr<IPacket> createPacket(IPacket::Type) const;
		virtual std::shared_ptr<IConnection> createConnection() const;
		virtual std::shared_ptr<IConnection> createConnection(std::unique_ptr<IConnectionBackend>&&) const;
	};
};
