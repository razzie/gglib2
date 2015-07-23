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

#if defined GGFRAMEWORK_BUILD
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
			virtual void subscribe(IEvent::Type) = 0;
			virtual void unsubscribe(IEvent::Type) = 0;
			virtual void addChild(std::unique_ptr<ITask>&&) = 0;
			virtual uint32_t getElapsedMs() const = 0;
			virtual bool hasEvent() const = 0;
			virtual std::shared_ptr<IEvent> getNextEvent() = 0;
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
		virtual void run(IThread&, IThread::TaskOptions&) = 0;
	};

	template<class F>
	std::unique_ptr<ITask> createTask(F func)
	{
		class FuncTask : public ITask
		{
		public:
			FuncTask(F func) : m_func(func) {}
			virtual ~FuncTask() = default;
			virtual void run(IThread& t, IThread::TaskOptions& o) { m_func(t, o); }

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
		virtual std::shared_ptr<IThread> createThread(const std::string& name) = 0;
		virtual std::shared_ptr<IThread> getThread(const std::string& name) const = 0;
	};

	extern GG_API IFramework& fw;
};
