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
		virtual const IStorage& params() const = 0;
		virtual void serialize(IPacket&) = 0;

		template<class T>
		const T& get(unsigned n) const
		{
			return params()->get<T>(n);
		}

		std::shared_ptr<IPacket> createPacket()
		{
			auto packet = net.createPacket(type());
			serialize(*packet);
			return packet;
		}
	};

	class IEventDefinition
	{
	public:
		virtual ~IEventDefinition() = default;
		virtual IEvent::Type type() const = 0;
		virtual std::shared_ptr<IEvent> create() = 0;
		virtual std::shared_ptr<IEvent> create(std::shared_ptr<IPacket>) = 0;

		bool operator< (const IEventDefinition& def)
		{
			return type() < def.type();
		}
	};

	template<IEvent::Type EventType, class... Params>
	class EventDefinition : public IEventDefinition
	{
	public:
		EventDefinition() = default;
		EventDefinition(const EventDefinition&) = default;
		virtual ~EventDefinition() = default;

		virtual IEvent::Type type() const
		{
			return EventType;
		}

		virtual std::shared_ptr<IEvent> create()
		{
			std::shared_ptr<IEvent> event(new Event());
			return event;
		}

		virtual std::shared_ptr<IEvent> create(std::shared_ptr<IPacket> packet)
		{
			if (packet->type() != EventType)
				return {};

			std::shared_ptr<IEvent> event(new Event());
			event->serialize(*packet);
			return event;
		}

		std::shared_ptr<IEvent> create(Params... params)
		{
			std::shared_ptr<IEvent> event(new Event(std::forward<Params>(params)...));
			return event;
		}

	private:
		class Event : public IEvent
		{
		public:
			Event() = default;

			Event(Params... params) :
				m_params(std::forward<Params>(params)...)
			{
			}

			virtual ~Event() = default;
			virtual Type type() const { return EventType; }
			virtual const IStorage& params() const { return m_params; }
			virtual void serialize(IPacket& packet) { m_params.serialize(packet); }

		private:
			SerializableStorage<Params...> m_params;
		};
	};


	class ITask;

	class IThread
	{
	public:
		class TaskOptions
		{
		public:
			virtual ~TaskOptions() = default;
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
		virtual void setup(IThread::TaskOptions&) {};
		virtual void run(IThread::TaskOptions&) = 0;
	};

	template<class F>
	std::unique_ptr<ITask> createTask(F func)
	{
		class FuncTask : public ITask
		{
		public:
			FuncTask(F func) : m_func(func) {}
			virtual ~FuncTask() = default;
			virtual void run(IThread::TaskOptions& o) { m_func(o); }

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
