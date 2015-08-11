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
	class IPacket
	{
	public:
		typedef IEvent::Type Type;

		enum Mode
		{
			READ_PACKET,
			WRITE_PACKET
		};

		virtual ~IPacket() = default;
		virtual Mode getMode() const = 0;
		virtual Type getType() const = 0;
		virtual const char* getData() const = 0;
		virtual size_t getSize() const = 0;

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

		virtual size_t write(const char* ptr, size_t len) = 0;
		virtual size_t read(char* ptr, size_t len) = 0;

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


	template<class... Types>
	class SerializableStorage : public Storage<Types...>, public ISerializable
	{
	public:
		SerializableStorage() = default;
		SerializableStorage(Types... values) : Storage(std::forward<Types>(values)...) {}
		virtual ~SerializableStorage() = default;
		virtual void serialize(IPacket& packet) { serialize<0, Types...>(packet, *this); }

	private:
		template<size_t N>
		static void serialize(IPacket& packet, IStorage& storage)
		{
		}

		template<size_t N, class Type0, class... Types>
		static void serialize(IPacket& packet, IStorage& storage)
		{
			packet & storage.get<Type0>(N);
			serialize<N + 1, Types...>(packet, storage);
		}
	};

	template<IEvent::Type EventType, class... Params>
	class SerializableEvent : public IEvent
	{
	public:
		SerializableEvent() = default;

		SerializableEvent(Params... params) :
			m_params(std::forward<Params>(params)...)
		{
		}

		virtual ~SerializableEvent() = default;
		virtual Type getType() const { return EventType; }
		virtual const IStorage& getParams() const { return m_params; }
		virtual void serialize(IPacket& packet) { m_params.serialize(packet); }

	private:
		SerializableStorage<Params...> m_params;
	};

	template<IEvent::Type EventType, class... Params>
	class SerializableEventDefinition : public IEventDefinition<Params...>
	{
	public:
		typedef SerializableEvent<EventType, Params...> Event;

		virtual IEvent::Type getType() const
		{
			return EventType;
		}

		virtual EventPtr operator()() const
		{
			return EventPtr(new Event());
		}

		virtual EventPtr operator()(IPacket& packet) const
		{
			if (packet.getType() != EventType)
				return {};

			EventPtr event(new Event());
			event->serialize(packet);
			return event;
		}

		virtual EventPtr operator()(Params... params) const
		{
			return EventPtr(new Event(std::forward<Params>(params)...));
		}
	};


	inline void serialize(IPacket& packet, IEvent::Flag& flag)
	{
		if (packet.getMode() == IPacket::Mode::READ_PACKET)
		{
			uint8_t f;
			packet & f;
			f ? flag.set() : flag.unset();
		}
		else
		{
			uint8_t f = flag.isSet();
			packet & f;
		}
	}

	inline void serialize(IPacket& packet, Version& ver)
	{
		packet & ver.m_major & ver.m_minor & ver.m_revision;
	}
};
