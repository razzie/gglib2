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
	class IThread;
	class ITask;
	class ITaskOptions;

	typedef std::shared_ptr<IThread> ThreadPtr;
	typedef std::unique_ptr<ITask> TaskPtr;

	class IThread
	{
	public:
		typedef uint16_t State;

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
	};

	class ITask
	{
	public:
		typedef uint16_t ID;

		virtual ~ITask() = default;
		// called once before the first update
		virtual void onStart(ITaskOptions&) {}
		// called for each event the task received right before onUpdate
		virtual void onEvent(ITaskOptions&, EventPtr) = 0;
		// called periodically until the task finishes
		virtual void onUpdate(ITaskOptions&) = 0;
		// called when thread changed state and task got activated/deactivated
		virtual void onStateChange(ITaskOptions&, IThread::State old_state, IThread::State new_state) {}
		// called each time an exception is encountered while processing events/update
		virtual void onError(ITaskOptions&, std::exception&) {}
		// called once after the task has finished
		virtual void onFinish(ITaskOptions&) {}
	};

	class ITaskOptions
	{
	public:
		virtual ~ITaskOptions() = default;
		virtual IThread& getThread() = 0;
		virtual ITask::ID getTaskID() const = 0;
		virtual IThread::State getState() const = 0;
		virtual void setState(IThread::State) = 0;
		virtual void subscribe(IEvent::Type) = 0;
		virtual void subscribe(IEventDefinitionBase&) = 0;
		virtual void unsubscribe(IEvent::Type) = 0;
		virtual void unsubscribe(IEventDefinitionBase&) = 0;
		virtual uint32_t getElapsedMs() const = 0;
		virtual void finish() = 0;
	};

	class IThreadManager
	{
	public:
		virtual ~IThreadManager() = default;
		virtual ThreadPtr createThread(const std::string& name) = 0;
		virtual ThreadPtr getThread(const std::string& name) const = 0;
		virtual void sendEvent(EventPtr) = 0;
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

		virtual EventPtr operator()() const
		{
			return EventPtr(new Event());
		}

		virtual EventPtr operator()(IPacket& packet) const
		{
			return {};
		}

		virtual EventPtr operator()(Params... params) const
		{
			return EventPtr(new Event(std::forward<Params>(params)...));
		}
	};
};
