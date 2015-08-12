/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <atomic>
#include <map>
#include <mutex>
#include <vector>
#include "gg/thread.hpp"
#include "gg/idgenerator.hpp"
#include "gg/timer.hpp"

namespace gg
{
	class TaskData : public ITaskOptions
	{
	public:
		TaskData(IThread*, TaskPtr, ITask::ID, IThread::State);
		TaskData(TaskData&&);
		virtual ~TaskData();
		TaskData& operator=(TaskData&&);
		bool isFinished() const;
		bool isSubscribed(IEvent::Type) const;
		void update(const std::vector<EventPtr>&);
		void stateChange(IThread::State old_state, IThread::State new_state);
		void error(std::exception&);

		// inherited from IThread::TaskOptions
		virtual IThread& getThread();
		virtual ITask::ID getTaskID() const;
		virtual IThread::State getState() const;
		virtual void setState(IThread::State);
		virtual void subscribe(IEvent::Type);
		virtual void subscribe(IEventDefinitionBase&);
		virtual void unsubscribe(IEvent::Type);
		virtual void unsubscribe(IEventDefinitionBase&);
		virtual uint32_t getElapsedMs() const;
		virtual void finish();

	private:
		IThread* m_thread;
		TaskPtr m_task;
		ITask::ID m_task_id;
		IThread::State m_task_state;
		std::vector<IEvent::Type> m_subscriptions;
		Timer m_timer;
		bool m_finished;
	};

	class Thread : public IThread
	{
	public:
		Thread(const std::string& name);
		virtual ~Thread();
		virtual State getState() const;
		virtual void setState(State);
		virtual void sendEvent(EventPtr);
		virtual void addTask(TaskPtr&&, State);
		virtual void finishTasks();
		virtual bool run(Mode = Mode::REMOTE);
		virtual bool isAlive() const;
		virtual void join();

	private:
		struct TaskWithState
		{
			TaskPtr task;
			State state;
		};

		std::string m_name;
		std::atomic<State> m_state;
		gg::IDGenerator<ITask::ID> m_task_id_generator;
		std::mutex m_tasks_mutex;
		std::vector<TaskData> m_tasks[2];
		std::vector<TaskWithState> m_pending_tasks;
		std::mutex m_events_mutex;
		std::vector<EventPtr> m_events[2];
		std::vector<EventPtr> m_pending_events;
		unsigned m_switch_active;
		std::thread m_thread;
		std::thread::id m_thread_id;
		std::atomic<bool> m_running;
		volatile bool m_finish_tasks;

		void thread();
	};

	class ThreadManager : public IThreadManager
	{
	public:
		ThreadManager();
		virtual ~ThreadManager();
		virtual ThreadPtr createThread(const std::string& name);
		virtual ThreadPtr getThread(const std::string& name) const;
		virtual void sendEvent(EventPtr);
		virtual void clean();

		ThreadPtr operator[](const std::string& name) const
		{
			return getThread(name);
		}

	private:
		mutable std::mutex m_mutex;
		std::map<std::string, std::weak_ptr<IThread>> m_threads;
	};
};
