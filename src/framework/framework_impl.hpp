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
#include <queue>
#include <vector>
#include "gg/framework.hpp"
#include "gg/timer.hpp"

namespace gg
{
	class TaskData : public IThread::TaskOptions
	{
	public:
		TaskData(IThread&, std::unique_ptr<ITask>);
		TaskData(TaskData&&);
		virtual ~TaskData();
		TaskData& operator=(TaskData&&);
		void pushEvents(const std::vector<std::shared_ptr<IEvent>>&);
		std::vector<std::unique_ptr<ITask>>& children();
		bool finished() const;
		void run();

		// inherited from IThread::TaskOptions
		virtual void subscribe(IEvent::Type);
		virtual void unsubscribe(IEvent::Type);
		virtual void addChild(std::unique_ptr<ITask>&&);
		virtual uint32_t getElapsedMs() const;
		virtual bool hasEvent() const;
		virtual std::shared_ptr<IEvent> getNextEvent();
		virtual void finish();

	private:
		IThread& m_thread;
		std::unique_ptr<ITask> m_task;
		std::vector<IEvent::Type> m_subscriptions;
		std::vector<std::unique_ptr<ITask>> m_children;
		Timer m_timer;
		std::queue<std::shared_ptr<IEvent>> m_events;
		bool m_finished;
	};

	class Thread : public IThread
	{
	public:
		Thread(const std::string& name);
		virtual ~Thread();
		virtual void sendEvent(std::shared_ptr<IEvent>);
		virtual void addTask(std::unique_ptr<ITask>&&);
		virtual void clearTasks();
		virtual bool run(Mode = Mode::REMOTE);
		virtual bool alive() const;
		virtual void join();

	private:
		std::string m_name;
		std::mutex m_tasks_mutex;
		std::vector<TaskData> m_tasks;
		std::vector<TaskData> m_pending_tasks;
		std::mutex m_events_mutex;
		std::vector<std::shared_ptr<IEvent>> m_events;
		std::vector<std::shared_ptr<IEvent>> m_pending_events;
		std::thread m_thread;
		std::thread::id m_thread_id;
		std::atomic<bool> m_running;
		volatile bool m_clear_tasks;

		void thread();
	};

	class Framework : public IFramework
	{
	public:
		Framework();
		virtual ~Framework();
		virtual std::shared_ptr<IThread> createThread(const std::string& name);
		virtual std::shared_ptr<IThread> getThread(const std::string& name) const;

	private:
		mutable std::mutex m_mutex;
		std::map<std::string, std::weak_ptr<IThread>> m_threads;
	};
};
