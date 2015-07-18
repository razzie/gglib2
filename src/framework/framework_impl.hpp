/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include "gg/framework.hpp"

namespace gg
{
	class TaskData : public IThread::Options
	{
	public:
		TaskData();
		virtual ~TaskData();;
		virtual void subscribe(IEvent::Type);
		virtual void unsubscribe(IEvent::Type);
		virtual void addTask(std::unique_ptr<ITask>&&);
		virtual void addChild(std::unique_ptr<ITask>&&);
		virtual uint32_t getElapsedMs() const;
		virtual bool hasEvent() const;
		virtual std::shared_ptr<IEvent> getNextEvent();
		virtual void sendEvent(std::shared_ptr<IEvent>);
		virtual void finish();
	};

	class Thread : public IThread
	{
	public:
		Thread(const std::string& name);
		virtual ~Thread();
		virtual void sendEvent(std::shared_ptr<IEvent>);
		virtual void addTask(std::unique_ptr<ITask>&&);
		virtual void clearTasks();
		virtual void run(Mode = Mode::REMOTE);
		virtual bool alive() const;
		virtual void join();
	};

	class Framework : public IFramework
	{
	public:
		Framework();
		virtual ~Framework();
		virtual std::shared_ptr<IThread> createThread(const std::string& name) const;
		virtual std::shared_ptr<IThread> getThread(const std::string& name) const;
	};
};
