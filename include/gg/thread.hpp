/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <cstdint>
#include <memory>
#include "gg/event.hpp"

#if defined GGTHREAD_BUILD
#	define GG_API __declspec(dllexport)
#else
#	define GG_API __declspec(dllimport)
#endif

namespace gg
{
	class ITask;
	class IThread;

	typedef std::unique_ptr<ITask> TaskPtr;
	typedef std::shared_ptr<IThread> ThreadPtr;

	class IThread
	{
	public:
		typedef uint16_t State;

		class TaskOptions
		{
		public:
			virtual ~TaskOptions() = default;
			virtual IThread& getThread() = 0;
			virtual State getState() const = 0;
			virtual void setState(State) = 0;
			virtual void subscribe(IEvent::Type) = 0;
			virtual void subscribe(IEventDefinitionBase&) = 0;
			virtual void unsubscribe(IEvent::Type) = 0;
			virtual void unsubscribe(IEventDefinitionBase&) = 0;
			virtual bool hasEvent() const = 0;
			virtual EventPtr getNextEvent() = 0;
			virtual uint32_t getElapsedMs() const = 0;
			virtual const std::string& getLastError() const = 0;
			virtual void finish() = 0;
		};

		enum Mode
		{
			LOCAL, // run in current thread (blocks)
			REMOTE // creates remote thread
		};

		virtual ~IThread() = default;
		virtual State getState() const = 0;
		virtual void setState(State) = 0;
		virtual void sendEvent(EventPtr) = 0;
		virtual void addTask(TaskPtr&&, State = 0) = 0;
		virtual void finishTasks() = 0; // thread stops if there is no task to run
		virtual bool run(Mode = Mode::REMOTE) = 0;
		virtual bool isAlive() const = 0;
		virtual void join() = 0;

		template<class Task, State state = 0, class... Params>
		void addTask(Params... params)
		{
			TaskPtr task(new Task(std::forward<Params>(params)...));
			addTask(std::move(task), state);
		}

		template<IEventDefinitionBase& Def, class... Params>
		void sendEvent(Params... params)
		{
			auto event = Def.create(std::forward<Params>(params)...);
			sendEvent(event);
		}
	};

	class ITask
	{
	public:
		virtual ~ITask() = default;
		// called once before the first update
		virtual void onStart(IThread::TaskOptions&) {};
		// called periodically until the task finishes
		virtual void onUpdate(IThread::TaskOptions&) = 0;
		// called when thread changed state and task got activate/deactivated
		virtual void onStateChange(IThread::State old_state, IThread::State new_state) {};
		// called once after the task has finished
		virtual void onFinish(IThread::TaskOptions&) {};
	};

	template<class F>
	TaskPtr createTask(F func)
	{
		class FuncTask : public ITask
		{
		public:
			FuncTask(F func) : m_func(func) {}
			virtual ~FuncTask() = default;
			virtual void onUpdate(IThread::TaskOptions& o) { m_func(o); }

		private:
			F m_func;
		};

		TaskPtr task(new FuncTask(func));
		return std::move(task);
	}


	class IThreadManager
	{
	public:
		virtual ~IThreadManager() = default;
		virtual ThreadPtr createThread(const std::string& name) = 0;
		virtual ThreadPtr getThread(const std::string& name) const = 0;
		virtual void sendEvent(EventPtr) = 0;

		template<IEventDefinitionBase& Def, class... Params>
		void sendEvent(Params... params)
		{
			auto event = Def.create(std::forward<Params>(params)...);
			sendEvent(event);
		}
	};

	extern GG_API IThreadManager& threadmgr;


	// Warning: the following event definitions are NOT network compatible
	template<IEvent::Type EventType, class... Params>
	class LocalEvent : public IEvent
	{
	public:
		LocalEvent() = default;

		LocalEvent(Params... params) :
			m_params(std::forward<Params>(params)...)
		{
		}

		virtual ~LocalEvent() = default;
		virtual IEvent::Type getType() const { return EventType; }
		virtual const IStorage& getParams() const { return m_params; }
		virtual void serialize(IPacket&) {}

	private:
		Storage<Params...> m_params;
	};

	template<IEvent::Type EventType, class... Params>
	class LocalEventDefinition : public IEventDefinition<Params...>
	{
	public:
		typedef LocalEvent<EventType, Params...> Event;

		virtual IEvent::Type getType() const
		{
			return EventType;
		}

		virtual EventPtr create() const
		{
			return EventPtr(new Event());
		}

		virtual EventPtr create(IPacket& packet) const
		{
			return {};
		}

		virtual EventPtr create(Params... params) const
		{
			return EventPtr(new Event(std::forward<Params>(params)...));
		}
	};
};
