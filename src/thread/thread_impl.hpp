/**
 * Copyright (c) 2014-2016 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <atomic>
#include <condition_variable>
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
		virtual const std::string& getName() const;
		virtual State getState() const;
		virtual void setState(State);
		virtual void sendEvent(EventPtr);
		virtual void addTask(TaskPtr&&, State);
		virtual void finish();
		virtual void finishTasks();
		virtual void finishTasksInState(State);
		virtual bool run(Mode = Mode::REMOTE);
		virtual bool isAlive() const;
		virtual void join();

	private:
		struct TaskWithState
		{
			TaskPtr task;
			State state;
		};

		struct Finish
		{
			bool thread = false;
			bool all_tasks = false;
			bool state_tasks = false;
			State state = 0;
		};

		std::string m_name;
		std::thread m_thread;
		std::thread::id m_thread_id;
		Mode m_mode;
		std::atomic<bool> m_running;
		std::atomic<bool> m_zombie;
		mutable std::mutex m_state_mutex;
		std::vector<State> m_state;
		mutable std::mutex m_finish_mutex;
		volatile Finish m_finish;
		mutable std::mutex m_awake_mutex;
		std::condition_variable m_awake;
		gg::IDGenerator<ITask::ID> m_task_id_generator;
		unsigned m_switch_active;
		mutable std::mutex m_tasks_mutex;
		std::vector<TaskData> m_tasks[2];
		std::vector<TaskWithState> m_pending_tasks;
		mutable std::mutex m_events_mutex;
		std::vector<EventPtr> m_events[2];
		std::vector<EventPtr> m_pending_events;

		void thread();
	};

	class ThreadPool : public IThreadPool
	{
	public:
		ThreadPool();
		virtual ~ThreadPool();
		virtual ThreadPtr createAndAddThread(const std::string& name);
		virtual ThreadPtr getThread(const std::string& name) const;
		virtual void addThread(ThreadPtr);
		virtual bool removeThread(const std::string& name);
		virtual void removeThreads();
		virtual void sendEvent(EventPtr);

	private:
		mutable std::mutex m_mutex;
		std::map<std::string, ThreadPtr> m_threads;
	};

	class ThreadManager : public IThreadManager
	{
	public:
		ThreadManager();
		virtual ~ThreadManager();
		virtual ThreadPtr createThread(const std::string& name) const;
		virtual ThreadPoolPtr createThreadPool() const;
		virtual ThreadPoolPtr getDefaultThreadPool();

	private:
		ThreadPoolPtr m_default_thread_pool;
	};
};
