/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "thread_impl.hpp"

static gg::ThreadManager s_thread;
gg::IThreadManager& gg::thread = s_thread;


gg::TaskData::TaskData(gg::IThread* thread, std::unique_ptr<ITask> task) :
	m_thread(thread),
	m_task(std::move(task)),
	m_finished(false)
{
	try
	{
		m_task->onStart(*this);
	}
	catch (...)
	{
		finish();
	}
}

gg::TaskData::TaskData(gg::TaskData&& taskdata) :
	m_thread(taskdata.m_thread),
	m_task(std::move(taskdata.m_task)),
	m_subscriptions(std::move(taskdata.m_subscriptions)),
	m_events(std::move(taskdata.m_events)),
	m_last_error(std::move(taskdata.m_last_error)),
	m_finished(false)
{
}

gg::TaskData::~TaskData()
{
}

gg::TaskData& gg::TaskData::operator=(TaskData&& taskdata)
{
	m_thread = taskdata.m_thread;
	m_task = std::move(taskdata.m_task);
	m_subscriptions = std::move(taskdata.m_subscriptions);
	m_events = std::move(taskdata.m_events);
	m_last_error = std::move(taskdata.m_last_error);
	m_finished = false;
	return *this;
}

gg::IThread& gg::TaskData::getThread()
{
	return *m_thread;
}

void gg::TaskData::subscribe(IEvent::Type type)
{
	for (IEvent::Type subscription : m_subscriptions)
	{
		if (type == subscription)
			return;
	}

	m_subscriptions.push_back(type);
}

void gg::TaskData::subscribe(IEventDefinitionBase& def)
{
	subscribe(def.getType());
}

void gg::TaskData::unsubscribe(IEvent::Type type)
{
	for (auto it = m_subscriptions.begin(), end = m_subscriptions.end(); it != end; ++it)
	{
		if (*it == type)
		{
			m_subscriptions.erase(it);
			return;
		}
	}
}

void gg::TaskData::unsubscribe(IEventDefinitionBase& def)
{
	unsubscribe(def.getType());
}

bool gg::TaskData::hasEvent() const
{
	return !m_events.empty();
}

std::shared_ptr<gg::IEvent> gg::TaskData::getNextEvent()
{
	if (m_events.empty())
	{
		return {};
	}
	else
	{
		std::shared_ptr<gg::IEvent> event = m_events.front();
		m_events.pop();
		return event;
	}
}

uint32_t gg::TaskData::getElapsedMs() const
{
	return static_cast<uint32_t>(m_timer.peekElapsed());
}

const std::string& gg::TaskData::getLastError() const
{
	return m_last_error;
}

void gg::TaskData::finish()
{
	m_finished = true;

	try
	{
		m_task->onFinish(*this);
	}
	catch (...)
	{
	}
}

void gg::TaskData::pushEvents(const std::vector<std::shared_ptr<gg::IEvent>>& events)
{
	for (auto& event : events)
	{
		for (IEvent::Type subscription : m_subscriptions)
		{
			if (event->getType() == subscription)
				m_events.push(event);
		}
	}
}

bool gg::TaskData::isFinished() const
{
	return m_finished;
}

void gg::TaskData::update()
{
	try
	{
		m_task->onUpdate(*this);
	}
	catch (std::exception& e)
	{
		m_last_error = e.what();
		finish();
	}
	catch (...)
	{
		m_last_error = "Unknown error";
		finish();
	}

	m_timer.reset();
}


gg::Thread::Thread(const std::string& name) :
	m_name(name),
	m_thread_id(std::this_thread::get_id()),
	m_running(false),
	m_clear_tasks(false)
{
	m_tasks.reserve(10);
	m_pending_tasks.reserve(10);
	m_internal_pending_tasks.reserve(10);
	m_events.reserve(10);
	m_pending_events.reserve(10);
}

gg::Thread::~Thread()
{
	if (m_running && m_thread.joinable())
	{
		clearTasks();
		m_thread.join();
	}
}

void gg::Thread::sendEvent(std::shared_ptr<gg::IEvent> event)
{
}

void gg::Thread::addTask(std::unique_ptr<gg::ITask>&& task)
{
	if (m_thread_id == std::this_thread::get_id())
	{
		m_internal_pending_tasks.push_back(std::move(task));
	}
	else
	{
		std::lock_guard<decltype(m_tasks_mutex)> guard(m_tasks_mutex);
		m_pending_tasks.push_back(std::move(task));
	}
}

void gg::Thread::clearTasks()
{
	m_clear_tasks = true;
}

bool gg::Thread::run(Mode mode)
{
	if (m_running.exchange(true))
		return false;

	switch (mode)
	{
	case Mode::LOCAL:
		m_thread_id = std::this_thread::get_id();
		thread();
		return true;

	case Mode::REMOTE:
		m_thread = std::move(std::thread(&Thread::thread, this));
		m_thread_id = m_thread.get_id();
		return true;

	default:
		return false;
	}
}

void gg::Thread::thread()
{
	do
	{
		// add pending tasks to task list
		if (m_tasks_mutex.try_lock())
		{
			for (auto&& task : m_pending_tasks)
				m_tasks.emplace_back(this, std::move(task));
			m_pending_tasks.clear();
			m_tasks_mutex.unlock();
		}

		// add pending internal tasks to task list
		for (auto&& task : m_internal_pending_tasks)
			m_tasks.emplace_back(this, std::move(task));
		m_internal_pending_tasks.clear();

		// add pending events to event list
		if (m_events_mutex.try_lock())
		{
			m_events.insert(m_events.end(),
				std::make_move_iterator(m_pending_events.begin()),
				std::make_move_iterator(m_pending_events.end()));
			m_pending_tasks.clear();
			m_events_mutex.unlock();
		}

		// assign events to subscribed tasks
		for (auto& task : m_tasks)
			task.pushEvents(m_events);
		m_events.clear();

		// run tasks
		for (auto it = m_tasks.begin(); it != m_tasks.end(); )
		{
			it->update(); // exceptions are already catched here

			// if task is finished, add its children to task list and remove it..
			if (it->isFinished())
			{
				it = m_tasks.erase(it);
				continue;
			}
			// ..otherwise go to next task
			else
			{
				++it;
			}
		}

		// clear tasks if it was requested
		if (m_clear_tasks)
		{
			m_clear_tasks = false;

			m_tasks.clear();
			m_internal_pending_tasks.clear();

			m_tasks_mutex.lock();
			m_pending_tasks.clear();
			m_tasks_mutex.unlock();
		}
	} while (!m_tasks.empty());

	m_running.store(false);
}

bool gg::Thread::isAlive() const
{
	return m_running;
}

void gg::Thread::join()
{
	if (m_thread.joinable())
		m_thread.join();
}


gg::ThreadManager::ThreadManager()
{
}

gg::ThreadManager::~ThreadManager()
{
}

std::shared_ptr<gg::IThread> gg::ThreadManager::createThread(const std::string& name)
{
	std::shared_ptr<IThread> thread(new Thread(name));

	{
		std::lock_guard<decltype(m_mutex)> guard(m_mutex);
		m_threads[name] = thread;
	}

	return thread;
}

std::shared_ptr<gg::IThread> gg::ThreadManager::getThread(const std::string& name) const
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	auto it = m_threads.find(name);
	if (it != m_threads.end())
	{
		return it->second.lock();
	}
	else
	{
		return {};
	}
}
