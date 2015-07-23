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


gg::TaskData::TaskData(gg::IThread& thread, std::shared_ptr<ITask> task) :
	m_thread(thread),
	m_task(task),
	m_finished(false)
{
	m_task->setup(*this);
}

gg::TaskData::TaskData(gg::TaskData&& taskdata) :
	m_thread(taskdata.m_thread),
	m_task(taskdata.m_task),
	m_subscriptions(std::move(taskdata.m_subscriptions)),
	m_children(std::move(taskdata.m_children)),
	m_events(std::move(taskdata.m_events)),
	m_finished(false)
{
}

gg::TaskData::~TaskData()
{
}

void gg::TaskData::getEvents(const std::vector<std::shared_ptr<gg::IEvent>>& events)
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

bool gg::TaskData::finished() const
{
	return m_finished;
}

void gg::TaskData::run()
{
	m_task->run(m_thread, *this);
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
	return m_timer.peekElapsed();
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


gg::Thread::Thread(const std::string& name)
{
}

gg::Thread::~Thread()
{
}

void gg::Thread::sendEvent(std::shared_ptr<gg::IEvent> event)
{
}

void gg::Thread::addTask(std::unique_ptr<gg::ITask>&& task)
{
}

void gg::Thread::clearTasks()
{
}

void gg::Thread::run(Mode mode)
{
}

bool gg::Thread::alive() const
{
	return false;
}

void gg::Thread::join()
{
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
