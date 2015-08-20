/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "thread_impl.hpp"

static gg::ThreadManager s_thread;
gg::IThreadManager& gg::threadmgr = s_thread;


gg::TaskData::TaskData(gg::IThread* thread, TaskPtr task, ITask::ID task_id, IThread::State state) :
	m_thread(thread),
	m_task(std::move(task)),
	m_task_id(task_id),
	m_task_state(state),
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
	m_task_id(taskdata.m_task_id),
	m_task_state(taskdata.m_task_state),
	m_subscriptions(std::move(taskdata.m_subscriptions)),
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
	m_task_id = taskdata.m_task_id;
	m_task_state = taskdata.m_task_state;
	m_subscriptions = std::move(taskdata.m_subscriptions);
	m_finished = false;
	return *this;
}

gg::IThread& gg::TaskData::getThread()
{
	return *m_thread;
}

gg::ITask::ID gg::TaskData::getTaskID() const
{
	return m_task_id;
}

gg::IThread::State gg::TaskData::getState() const
{
	return m_task_state;
}

void gg::TaskData::setState(IThread::State state)
{
	m_task_state = state;
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

uint32_t gg::TaskData::getElapsedMs() const
{
	return static_cast<uint32_t>(m_timer.peekElapsed());
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

bool gg::TaskData::isFinished() const
{
	return m_finished;
}

bool gg::TaskData::isSubscribed(IEvent::Type event_type) const
{
	for (IEvent::Type subscription : m_subscriptions)
	{
		if (event_type == subscription)
			return true;
	}

	return false;
}

void gg::TaskData::update(const std::vector<EventPtr>& events)
{
	if (!events.empty() && !m_subscriptions.empty())
	{
		for (auto& event : events)
		{
			if (isSubscribed(event->getType()))
			{
				try
				{
					m_task->onEvent(*this, std::move(event));
				}
				catch (std::exception& e)
				{
					error(e);
				}
				catch (...)
				{
					error(std::runtime_error("unknown"));
				}
			}
		}
	}

	try
	{
		m_task->onUpdate(*this);
	}
	catch (std::exception& e)
	{
		error(e);
	}
	catch (...)
	{
		error(std::runtime_error("unknown"));
	}

	m_timer.reset();
}

void gg::TaskData::stateChange(IThread::State old_state, IThread::State new_state)
{
	m_task->onStateChange(*this, old_state, new_state);
}

void gg::TaskData::error(std::exception& e)
{
	try
	{
		m_task->onError(*this, e);
	}
	catch (...)
	{
		finish();
	}
}


gg::Thread::Thread(const std::string& name) :
	m_name(name),
	m_thread_id(std::this_thread::get_id()),
	m_running(false),
	m_zombie(false),
	m_switch_active(1)
{
	m_state.push_back(0);

	m_tasks[0].reserve(10);
	m_tasks[1].reserve(10);
	m_pending_tasks.reserve(10);

	m_events[0].reserve(10);
	m_events[1].reserve(10);
	m_pending_events.reserve(10);
}

gg::Thread::~Thread()
{
	if (m_running && m_thread.joinable())
	{
		finish();
		m_thread.join();
	}
}

gg::IThread::State gg::Thread::getState() const
{
	std::lock_guard<decltype(m_state_mutex)> guard(m_state_mutex);
	return m_state.front();
}

void gg::Thread::setState(State state)
{
	std::lock_guard<decltype(m_state_mutex)> guard(m_state_mutex);

	if (m_state.back() != state)
		m_state.push_back(state);

	if (m_zombie)
		m_awake.notify_all();
}

void gg::Thread::sendEvent(EventPtr event)
{
	if (!event)
		return;

	if (m_thread_id == std::this_thread::get_id())
	{
		m_events[(m_switch_active + 1) % 2].push_back(event);
	}
	else
	{
		std::lock_guard<decltype(m_events_mutex)> guard(m_events_mutex);
		m_pending_events.push_back(event);
	}
}

void gg::Thread::addTask(TaskPtr&& task, State state)
{
	if (m_thread_id == std::this_thread::get_id())
	{
		m_tasks[(m_switch_active + 1) % 2].emplace_back(
			this, std::move(task), m_task_id_generator.next(), state);
	}
	else
	{
		std::lock_guard<decltype(m_tasks_mutex)> guard(m_tasks_mutex);
		m_pending_tasks.push_back(TaskWithState{ std::move(task), state });

		if (m_zombie)
			m_awake.notify_all();
	}
}

void gg::Thread::finish()
{
	std::lock_guard<decltype(m_finish_mutex)> guard(m_finish_mutex);
	m_finish.thread = true;
}

void gg::Thread::finishTasks()
{
	std::lock_guard<decltype(m_finish_mutex)> guard(m_finish_mutex);
	m_finish.all_tasks = true;
}

void gg::Thread::finishTasksInState(State state)
{
	std::lock_guard<decltype(m_finish_mutex)> guard(m_finish_mutex);
	m_finish.state_tasks = true;
	m_finish.state = state;
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
	State state = getState();
	State prev_state;
	bool state_will_change;
	unsigned task_run_count;

restart_thread:
	m_zombie.store(false);

	do
	{
		// update current state
		{
			std::lock_guard<decltype(m_state_mutex)> guard(m_state_mutex);

			prev_state = state;
			state = m_state.front();
			state_will_change = false;

			// if there are states waiting, move the first one to the front
			if (m_state.size() > 1)
			{
				m_state.erase(m_state.begin());

				// keep the thread running if there is still at least one state in the queue
				if (m_state.size() > 1)
					state_will_change = true;
			}
		}

		// switch between tasks/next_tasks and events/next_events
		m_switch_active = (m_switch_active + 1) % 2;
		std::vector<TaskData>& tasks = m_tasks[m_switch_active];
		std::vector<TaskData>& next_tasks = m_tasks[(m_switch_active + 1) % 2];
		std::vector<EventPtr>& events = m_events[m_switch_active];
		std::vector<EventPtr>& next_events = m_events[(m_switch_active + 1) % 2];

		// add pending tasks to task list
		{
			std::lock_guard<decltype(m_tasks_mutex)> guard(m_tasks_mutex);

			for (auto& task : m_pending_tasks)
			{
				tasks.emplace_back(
					this, std::move(task.task), m_task_id_generator.next(), task.state);
			}
			m_pending_tasks.clear();
		}

		// check for finish conditions
		{
			std::lock_guard<decltype(m_finish_mutex)> guard(m_finish_mutex);

			if (m_finish.all_tasks)
			{
				m_finish.all_tasks = false;

				tasks.clear();
				next_tasks.clear();

				break;
			}

			if (m_finish.state_tasks)
			{
				m_finish.state_tasks = false;

				for (auto it = tasks.begin(); it != tasks.end(); )
				{
					if (it->getState() == m_finish.state)
						tasks.erase(it);
					else
						++it;
				}

				for (auto it = next_tasks.begin(); it != next_tasks.end(); )
				{
					if (it->getState() == m_finish.state)
						next_tasks.erase(it);
					else
						++it;
				}
			}

			if (m_finish.thread)
			{
				m_finish.thread = false;

				m_running.store(false);
				return;
			}
		}

		// add pending events to event list
		{
			std::lock_guard<decltype(m_events_mutex)> guard(m_events_mutex);

			events.insert(events.end(),
				std::make_move_iterator(m_pending_events.begin()),
				std::make_move_iterator(m_pending_events.end()));
			m_pending_tasks.clear();
		}

		task_run_count = 0;
		if (!tasks.empty())
		{
			for (auto& task : tasks)
			{
				// thread just changed state
				if (state != prev_state)
				{
					State task_state = task.getState();

					// check if task got activated or deactivated and fire callback
					if (task_state == state || task_state == prev_state)
					{
						task.stateChange(prev_state, state);
					}
				}

				// check again if task state matches thread state and..
				if (task.getState() != state)
				{
					// ..skip it now but try running it next time
					next_tasks.push_back(std::move(task));
					continue;
				}

				// update task, exceptions are handled internally
				task.update(events);

				++task_run_count;

				// run the task next time too if it's not finished
				if (!task.isFinished())
					next_tasks.push_back(std::move(task));
			}
			tasks.clear(); // clear after the run, we'll switch to next_tasks soon
		}

		events.clear();
		std::this_thread::yield();

	} while (task_run_count || state_will_change);

	// enter zombie (waiting) state if no task to run on REMOTE mode
	if (m_mode == Mode::REMOTE)
	{
		m_zombie.store(true);

		std::unique_lock<decltype(m_awake_mutex)> l(m_awake_mutex);
		m_awake.wait(l);

		goto restart_thread;
	}
	// in LOCAL mode, just exit the function
	else
	{
		m_running.store(false);
		return;
	}
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

gg::ThreadPtr gg::ThreadManager::createThread(const std::string& name)
{
	ThreadPtr thread(new Thread(name));

	{
		std::lock_guard<decltype(m_mutex)> guard(m_mutex);
		m_threads[name] = thread;
	}

	return thread;
}

gg::ThreadPtr gg::ThreadManager::getThread(const std::string& name) const
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

void gg::ThreadManager::sendEvent(EventPtr event)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	for (auto& it : m_threads)
	{
		ThreadPtr thread = it.second.lock();
		if (thread)
		{
			thread->sendEvent(event);
		}
	}
}

void gg::ThreadManager::clean()
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	for (auto it = m_threads.begin(); it != m_threads.end(); )
	{
		if (it->second.expired())
			it = m_threads.erase(it);
		else
			++it;
	}
}
