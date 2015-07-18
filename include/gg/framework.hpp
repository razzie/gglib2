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

#ifdef GG_API
#	undef GG_API
#endif

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

		template<class... Args>
		static std::shared_ptr<IEvent> getFromPacket(std::shared_ptr<IPacket> packet)
		{
			std::shared_ptr<IEvent> event(new Event<Args...>(packet->type()));
			packet & event;
			return event;
		}

		std::shared_ptr<IPacket> createPacket()
		{
			auto packet = net.createPacket(type());
			serialize(*packet);
			return packet;
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

		static std::shared_ptr<IEvent> getFromPacket(std::shared_ptr<IPacket> packet)
		{
			return IEvent::getFromPacket<Args...>(packet);
		}

	private:
		Type m_type;
		SerializableStorage<Args...> m_args;
	};


	class ITask;

	class IThread
	{
	public:
		class Options
		{
		public:
			virtual ~Options() = default;
			virtual void subscribe(IEvent::Type) = 0;
			virtual void unsubscribe(IEvent::Type) = 0;
			virtual void addTask(std::unique_ptr<ITask>&&) = 0;
			virtual void addChild(std::unique_ptr<ITask>&&) = 0;
			virtual uint32_t getElapsedMs() const = 0;
			virtual bool hasEvent() const = 0;
			virtual std::shared_ptr<IEvent> getNextEvent() = 0;
			virtual void sendEvent(std::shared_ptr<IEvent>) = 0;
			virtual void finish() = 0;
		};

		enum Mode
		{
			LOCAL, // run in current thread (blocks)
			REMOTE // creates remote thread
		};

		virtual ~IThread() = default;
		virtual void sendEvent(std::shared_ptr<IEvent>) = 0;
		virtual void addTask(std::unique_ptr<ITask>&&) = 0;
		virtual void clearTasks() = 0;
		virtual void run(Mode = Mode::REMOTE) = 0;
		virtual bool alive() const = 0;
		virtual void join() = 0;
	};

	class ITask
	{
	public:
		virtual ~ITask() = default;
		virtual void setup(IThread::Options&) {};
		virtual void run(IThread::Options&) = 0;
	};

	template<class F>
	std::unique_ptr<ITask> createTask(F func)
	{
		class FuncTask : public ITask
		{
		public:
			FuncTask(F func) : m_func(func) {}
			virtual ~FuncTask() = default;
			virtual void run(IThread::Options& o) { m_func(o); }

		private:
			F m_func;
		};

		std::unique_ptr<ITask> task(new FuncTask(func));
		return std::move(task);
	}


	class IFramework
	{
	public:
		virtual ~IFramework() = default;
		virtual std::shared_ptr<IThread> createThread(const std::string& name) const = 0;
		virtual std::shared_ptr<IThread> getThread(const std::string& name) const = 0;
	};

	extern GG_API IFramework& fw;
};
