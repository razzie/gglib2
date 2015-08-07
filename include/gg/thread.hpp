/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

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

	class IThread
	{
	public:
		class TaskOptions
		{
		public:
			virtual ~TaskOptions() = default;
			virtual IThread& getThread() = 0;
			virtual void subscribe(IEvent::Type) = 0;
			virtual void subscribe(IEventDefinitionBase&) = 0;
			virtual void unsubscribe(IEvent::Type) = 0;
			virtual void unsubscribe(IEventDefinitionBase&) = 0;
			virtual bool hasEvent() const = 0;
			virtual std::shared_ptr<IEvent> getNextEvent() = 0;
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
		virtual void sendEvent(std::shared_ptr<IEvent>) = 0;
		virtual void addTask(std::unique_ptr<ITask>&&) = 0;
		virtual void clearTasks() = 0; // thread stops if there is no task to run
		virtual bool run(Mode = Mode::REMOTE) = 0;
		virtual bool isAlive() const = 0;
		virtual void join() = 0;

		template<class Task, class... Params>
		void addTask(Params... params)
		{
			std::unique_ptr<ITask> task(new Task(std::forward<Params>(params)...));
			addTask(std::move(task));
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
		virtual void onStart(IThread::TaskOptions&) {}; // called once before the first update
		virtual void onUpdate(IThread::TaskOptions&) = 0; // called periodically until the task finishes
		virtual void onFinish(IThread::TaskOptions&) {}; // called once after the task finished
	};

	template<class F>
	std::unique_ptr<ITask> createTask(F func)
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

		std::unique_ptr<ITask> task(new FuncTask(func));
		return std::move(task);
	}


	class IThreadManager
	{
	public:
		virtual ~IThreadManager() = default;
		virtual std::shared_ptr<IThread> createThread(const std::string& name) = 0;
		virtual std::shared_ptr<IThread> getThread(const std::string& name) const = 0;
	};

	extern GG_API IThreadManager& thread;


	// Warning: it is NOT network compatible
	template<IEvent::Type EventType, class... Params>
	class SimpleEventDefinition : public IEventDefinition<Params...>
	{
	public:
		SimpleEventDefinition(Params... params) :
			m_params(std::forward<Params>(params)...)
		{
		}

		virtual ~SimpleEventDefinition() = default;
		virtual IEvent::Type getType() const { return EventType; }
		virtual const IStorage& getParams() const { return m_params; }
		virtual void serialize(IPacket&) {}

	private:
		Storage<Params...> m_params;
	};
};
