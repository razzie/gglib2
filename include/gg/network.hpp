/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

/**
 * HOW TO USE:
 * -----------
 *
 */

#pragma once

#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <typeinfo>
#include "gg/storage.hpp"

#if defined GGNETWORK_BUILD
#	define GG_API __declspec(dllexport)
#else
#	define GG_API __declspec(dllimport)
#endif

namespace gg
{
	class IBlob;
	class ISerializable;

	class IPacket
	{
	public:
		typedef uint16_t Type;

		enum Mode
		{
			READ,
			WRITE
		};

		virtual ~IPacket() = default;
		virtual Mode mode() const = 0;
		virtual Type type() const = 0;
		virtual const char* data() const = 0;
		virtual size_t length() const = 0;

		virtual IPacket& operator& (int8_t&) = 0;
		virtual IPacket& operator& (int16_t&) = 0;
		virtual IPacket& operator& (int32_t&) = 0;
		virtual IPacket& operator& (int64_t&) = 0;
		virtual IPacket& operator& (uint8_t&) = 0;
		virtual IPacket& operator& (uint16_t&) = 0;
		virtual IPacket& operator& (uint32_t&) = 0;
		virtual IPacket& operator& (uint64_t&) = 0;
		virtual IPacket& operator& (float&) = 0;
		virtual IPacket& operator& (double&) = 0;
		virtual IPacket& operator& (std::string&) = 0;
		virtual IPacket& operator& (IBlob&) = 0;
		virtual IPacket& operator& (ISerializable&) = 0;

		template<class T>
		IPacket& operator& (T& t)
		{
			serialize(*this, t);
			return *this;
		}
	};

	template<class T>
	IPacket& operator& (std::shared_ptr<IPacket> packet_ptr, T& t)
	{
		IPacket& packet = *packet_ptr;
		packet & t;
		return packet;
	}

	class ISerializationError : public std::exception
	{
	public:
		virtual ~ISerializationError() = default;
		virtual const char* what() const = 0;
	};


	class IConnectionBackend // adaption to external APIs like Steam
	{
	public:
		virtual ~IConnectionBackend() = default;
		virtual bool connect(void* user_data = nullptr) = 0;
		virtual void disconnect() = 0;
		virtual bool alive() const = 0;
		virtual const std::string& getAddress() const = 0;
		virtual size_t availableData() = 0;
		virtual size_t waitForData(size_t len, uint32_t timeoutMs = 0) = 0; // 0: non-blocking
		virtual size_t peek(char* ptr, size_t len) = 0;
		virtual size_t read(char* ptr, size_t len) = 0;
		virtual size_t write(const char* ptr, size_t len) = 0;
	};

	class IConnection
	{
	public:
		virtual ~IConnection() = default;
		virtual bool connect(void* user_data = nullptr) = 0;
		virtual void disconnect() = 0;
		virtual bool alive() const = 0;
		virtual const std::string& getAddress() const = 0;
		virtual std::shared_ptr<IPacket> getNextPacket(uint32_t timeoutMs = 0) = 0; // 0: non-blocking
		virtual std::shared_ptr<IPacket> createPacket(IPacket::Type) const = 0;
		virtual bool send(std::shared_ptr<IPacket>) = 0;
	};

	class IServerBackend // adaption to external APIs like Steam
	{
	public:
		virtual ~IServerBackend() = default;
		virtual bool start(void* user_data = nullptr) = 0;
		virtual void stop() = 0;
		virtual bool alive() const = 0;
		virtual std::unique_ptr<IConnectionBackend> getNextConnection(uint32_t timeoutMs = 0) = 0; // 0: non-blocking
	};

	class IServer
	{
	public:
		virtual ~IServer() = default;
		virtual bool start(void* user_data = nullptr) = 0;
		virtual void stop() = 0;
		virtual bool alive() const = 0;
		virtual std::shared_ptr<IConnection> getNextConnection(uint32_t timeoutMs = 0) = 0; // 0: non-blocking
		virtual void closeConnections() = 0;
	};

	class INetworkException : public std::exception
	{
	public:
		virtual ~INetworkException() = default;
		virtual const char* what() const = 0;
	};

	class INetworkManager
	{
	public:
		virtual ~INetworkManager() = default;
		virtual std::shared_ptr<IPacket> createPacket(IPacket::Type) const = 0;
		virtual std::shared_ptr<IConnection> createConnection(const std::string& host, uint16_t port) const = 0;
		virtual std::shared_ptr<IConnection> createConnection(std::unique_ptr<IConnectionBackend>&&) const = 0;
		virtual std::shared_ptr<IServer> createServer(uint16_t port) const = 0;
		virtual std::shared_ptr<IServer> createServer(std::unique_ptr<IServerBackend>&&) const = 0;
	};

	extern GG_API INetworkManager& net;


	class ISerializable
	{
	public:
		virtual ~ISerializable() = default;
		virtual void serialize(IPacket&) = 0;
	};

	class IBlob
	{
	public:
		virtual ~IBlob() = default;
		virtual char* data() = 0;
		virtual const char* data() const = 0;
		virtual size_t length() const = 0;
	};

	template<size_t SIZE>
	class Blob : public IBlob
	{
	public:
		Blob() = default;
		virtual ~Blob() = default;
		virtual char* data() { return m_data; }
		virtual const char* data() const { return m_data; }
		virtual size_t length() const { return SIZE; }

	private:
		char m_data[SIZE];
	};


	namespace __SerializeStorage
	{
		template<size_t N>
		void serialize(IPacket& packet, IStorage& storage)
		{
		}

		template<size_t N, class Type0, class... Types>
		void serialize(IPacket& packet, IStorage& storage)
		{
			packet & storage.get<Type0>(N);
			serialize<N + 1, Types...>(packet, storage);
		}
	}

	template<class... Types>
	void serialize(IPacket& packet, Storage<Types...>& storage)
	{
		__SerializeStorage::serialize<0, Types...>(packet, storage);
	}
};
