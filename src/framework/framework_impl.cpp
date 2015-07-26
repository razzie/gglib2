/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "framework_impl.hpp"

static gg::Framework s_fw;
gg::IFramework& gg::fw = s_fw;


gg::TaskData::TaskData(gg::IThread& thread, std::unique_ptr<ITask> task) :
	m_thread(thread),
	m_task(std::move(task)),
	m_finished(false)
{
	try
	{
		m_task->setup(*this);
	}
	catch (...)
	{
		m_finished = true;
	}
}

gg::TaskData::TaskData(gg::TaskData&& taskdata) :
	m_thread(taskdata.m_thread),
	m_task(std::move(taskdata.m_task)),
	m_subscriptions(std::move(taskdata.m_subscriptions)),
	m_children(std::move(taskdata.m_children)),
	m_events(std::move(taskdata.m_events)),
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
	m_children = std::move(taskdata.m_children);
	m_events = std::move(taskdata.m_events);
	m_finished = false;
	return *this;
}

void gg::TaskData::pushEvents(const std::vector<std::shared_ptr<gg::IEvent>>& events)
{
	for (auto& event : events)
	{
		for (IEvent::Type subscription : m_subscriptions)
		{
			if (event->type() == subscription)
				m_events.push(event);
		}
	}
}

std::vector<std::unique_ptr<gg::ITask>>& gg::TaskData::children()
{
	return m_children;
}

bool gg::TaskData::finished() const
{
	return m_finished;
}

void gg::TaskData::run()
{
	try
	{
		m_task->run(m_thread, *this);
	}
	catch (...)
	{
		m_finished = true;
	}

	m_timer.reset();
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

void gg::TaskData::addChild(std::unique_ptr<ITask>&& task)
{
	m_children.push_back(std::move(task));
}

uint32_t gg::TaskData::getElapsedMs() const
{
	return static_cast<uint32_t>(m_timer.peekElapsed());
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

void gg::TaskData::finish()
{
	m_finished = true;
}


gg::Thread::Thread(const std::string& name) :
	m_name(name),
	m_thread_id(std::this_thread::get_id()),
	m_running(false),
	m_clear_tasks(false)
{
	m_tasks.reserve(10);
	m_pending_tasks.reserve(10);
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
	if (m_thread_id == std::this_thread::get_id()
		&& m_tasks.capacity() > m_tasks.size())
	{
		m_tasks.emplace_back(*this, std::move(task));
	}
	else
	{
		std::lock_guard<decltype(m_tasks_mutex)> guard(m_tasks_mutex);
		m_pending_tasks.emplace_back(*this, std::move(task));
	}
}

void gg::Thread::addTasks(std::vector<std::unique_ptr<ITask>>& tasks)
{
	if (m_thread_id == std::this_thread::get_id()
		&& (m_tasks.capacity() - m_tasks.size()) > tasks.size())
	{
		for (auto& task : tasks)
			m_tasks.emplace_back(*this, std::move(task));
	}
	else
	{
		std::lock_guard<decltype(m_tasks_mutex)> guard(m_tasks_mutex);
		for (auto& task : tasks)
			m_pending_tasks.emplace_back(*this, std::move(task));
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
			m_tasks.insert(m_tasks.end(),
				std::make_move_iterator(m_pending_tasks.begin()),
				std::make_move_iterator(m_pending_tasks.end()));
			m_pending_tasks.clear();
			m_tasks_mutex.unlock();
		}

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
			it->run(); // exceptions are already catched here

			// if task is finished, add its children to task list and remove it..
			if (it->finished())
			{
				addTasks(it->children());
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
		}
	} while (!m_tasks.empty());

	m_running.store(false);
}

bool gg::Thread::alive() const
{
	return m_running;
}

void gg::Thread::join()
{
	if (m_thread.joinable())
		m_thread.join();
}


gg::Framework::Framework()
{
}

gg::Framework::~Framework()
{
}

std::shared_ptr<gg::IThread> gg::Framework::createThread(const std::string& name)
{
	std::shared_ptr<IThread> thread(new Thread(name));

	{
		std::lock_guard<decltype(m_mutex)> guard(m_mutex);
		m_threads[name] = thread;
	}

	return thread;
}

std::shared_ptr<gg::IThread> gg::Framework::getThread(const std::string& name) const
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
