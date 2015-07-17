/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <memory>
#include "gg/storage.hpp"
#include "gg/network.hpp"

#if defined GGFRAMEWORK_BUILD
#	define GG_API __declspec(dllexport)
#else
#	define GG_API __declspec(dllimport)
#endif

namespace gg
{
	class IEvent : public ISerializable
	{
	public:
		typedef IPacket::Type Type;

		virtual ~IEvent() = default;
		virtual Type type() const = 0;
		virtual const IStorage& args() const = 0;
		virtual void serialize(IPacket&) = 0;

		template<class T>
		const T& get(unsigned n) const
		{
			return args()->get<T>(n);
		}
	};

	template<class... Args>
	class Event final : public IEvent
	{
	public:
		Event(Type type) :
			m_type(type)
		{
		}

		Event(Type type, Args... args) :
			m_type(type), m_args(std::forward<Args>(args)...)
		{
		}

		virtual ~Event() = default;
		virtual Type type() const { return m_type; }
		virtual const IStorage& args() const { return m_args; }
		virtual void serialize(IPacket& packet) { packet & m_args; }

		static std::shared_ptr<IEvent> getNextFromConnection(std::shared_ptr<IConnection> conn, uint32_t timeoutMs = 0)
		{
			auto packet = conn->getNextPacket(timeoutMs);
			if (!packet)
				return {};

			std::shared_ptr<IEvent> event( new Event<Args...>(static_cast<Type>(packet->type())) );
			packet & event;
			return event;
		}

		bool sendToConnection(std::shared_ptr<IConnection> conn)
		{
			auto packet = conn->createPacket(static_cast<IPacket::Type>(m_type));
			packet & *this;
			return conn->send(packet);
		}

	private:
		Type m_type;
		Storage<Args...> m_args;
	};


	class ITask;

	class ITaskPool
	{
	public:
		class Setup
		{
		public:
			virtual ~Setup() = default;
			virtual void subscribeToEvent(IEvent::Type) = 0;
			virtual void addChild(std::unique_ptr<ITask>&&) = 0;
		};

		class Run
		{
		public:
			virtual ~Run() = default;
			virtual uint32_t getElapsedMs() const = 0;
			virtual void finish() = 0;
		};

		virtual ~ITaskPool() = default;
		virtual void addTask(std::unique_ptr<ITask>&&) = 0;
		virtual void clearTasks() = 0;
		virtual void startAsThread() = 0;
		virtual void startLocally() = 0;
		virtual void stop() = 0;
		virtual void join() = 0;
	};

	class ITask
	{
	public:
		virtual ~ITask() = default;

	protected:
		virtual void setup(ITaskPool::Setup&) = 0;
		virtual void run(ITaskPool::Run&) = 0;
	};


	class IFramework
	{
	public:
		virtual ~IFramework() = default;
		virtual std::shared_ptr<ITaskPool> createTaskPool() const = 0;
	};

	//extern GG_API IFramework& fw;
};
