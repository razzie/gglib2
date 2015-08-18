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
#include "gg/serializable.hpp"
#include "gg/event.hpp"
#include "gg/storage.hpp"
#include "gg/version.hpp"

#if defined GGNETWORK_BUILD
#	define GG_API __declspec(dllexport)
#else
#	define GG_API __declspec(dllimport)
#endif

namespace gg
{
	class IPacket : public virtual IArchive
	{
	public:
		typedef IEvent::Type Type;

		virtual ~IPacket() = default;
		virtual Type getType() const = 0;

		/* inherits all IArchive functions */
	};

	template<class T>
	IPacket& operator& (std::shared_ptr<IPacket> p, T& t)
	{
		IPacket& packet = *p;
		packet & t;
		return packet;
	}

	class IConnectionBackend // adaption to external APIs like Steam
	{
	public:
		virtual ~IConnectionBackend() = default;
		virtual bool connect(void* user_data = nullptr) = 0;
		virtual void disconnect() = 0;
		virtual bool isAlive() const = 0;
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
		virtual bool isAlive() const = 0;
		virtual const std::string& getAddress() const = 0;
		virtual std::shared_ptr<IPacket> getNextPacket(uint32_t timeoutMs = 0) = 0; // 0: non-blocking
		virtual std::shared_ptr<IPacket> createPacket(IPacket::Type) const = 0;
		virtual std::shared_ptr<IPacket> createPacket(EventPtr) const = 0;
		virtual bool send(std::shared_ptr<IPacket>) = 0;
	};

	class IServerBackend // adaption to external APIs like Steam
	{
	public:
		virtual ~IServerBackend() = default;
		virtual bool start(void* user_data = nullptr) = 0;
		virtual void stop() = 0;
		virtual bool isAlive() const = 0;
		virtual std::unique_ptr<IConnectionBackend> getNextConnection(uint32_t timeoutMs = 0) = 0; // 0: non-blocking
	};

	class IServer
	{
	public:
		virtual ~IServer() = default;
		virtual bool start(void* user_data = nullptr) = 0;
		virtual void stop() = 0;
		virtual bool isAlive() const = 0;
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
		virtual std::shared_ptr<IConnection> createConnection(const std::string& host, uint16_t port) const = 0;
		virtual std::shared_ptr<IConnection> createConnection(std::unique_ptr<IConnectionBackend>&&) const = 0;
		virtual std::shared_ptr<IServer> createServer(uint16_t port) const = 0;
		virtual std::shared_ptr<IServer> createServer(std::unique_ptr<IServerBackend>&&) const = 0;
		virtual std::shared_ptr<IPacket> createPacket(IPacket::Type) const = 0;
		virtual std::shared_ptr<IPacket> createPacket(EventPtr) const = 0;
	};

	extern GG_API INetworkManager& net;
};
