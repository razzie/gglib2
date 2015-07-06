/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#if defined GG_BUILD
#	define GG_API __declspec(dllexport)
#else
#	define GG_API __declspec(dllimport)
#endif

#include <cstdint>
#include <memory>
#include <string>
#include <typeinfo>
#include "gg/storage.hpp"

namespace gg
{
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
		virtual Mode getMode() const = 0;

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
		virtual IPacket& operator& (ISerializable&) = 0;

		template<class T>
		IPacket& operator& (T& t)
		{
			serialize(*this, t);
		}
	};

	class IReadModePacket : public IPacket
	{
	public:
		virtual ~IReadModePacket() = default;
		virtual IPacket::Type getType() const = 0;
	};

	class IWriteModePacket : public IPacket
	{
	public:
		virtual ~IWriteModePacket() = default;
		virtual void sendAs(IPacket::Type) const = 0;
	};

	class ISerializable
	{
	public:
		virtual ~ISerializable() = default;
		virtual void serialize(IPacket&) = 0;
	};


	class IConnectionBackend // adaption to external APIs like Steam
	{
	public:
		virtual ~IConnectionBackend() = default;
		virtual bool connect(const std::string& address, uint16_t port) = 0;
		virtual void disconnect() = 0;
		virtual size_t available() const = 0;
		virtual size_t waitForAvailable() const = 0;
		virtual size_t peek(char* ptr, size_t len) const = 0;
		virtual size_t read(char* ptr, size_t len) = 0;
		virtual void write(const char* ptr, size_t len) = 0;
	};

	class IConnection
	{
	public:
		virtual ~IConnection() = default;
		virtual bool connect(const std::string& address, uint16_t port) = 0;
		virtual void disconnect() = 0;
		virtual bool packetAvailable() const = 0;
		virtual std::shared_ptr<IReadModePacket> getNextPacket() = 0;
		virtual std::shared_ptr<IReadModePacket> waitForNextPacket() = 0;
		virtual std::shared_ptr<IWriteModePacket> createPacket() = 0;
	};

	class INetworkManager
	{
	public:
		virtual ~INetworkManager() = default;
		virtual std::shared_ptr<IConnection> createConnection() = 0;
		virtual std::shared_ptr<IConnection> createConnection(std::shared_ptr<IConnectionBackend>) = 0;
	};

	//extern GG_API INetworkManager& net;


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
