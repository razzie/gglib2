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


gg::TaskData::TaskData()
{
}

gg::TaskData::~TaskData()
{
}

void gg::TaskData::subscribe(IEvent::Type type)
{
}

void gg::TaskData::unsubscribe(IEvent::Type type)
{
}

void gg::TaskData::addTask(std::unique_ptr<ITask>&& task)
{
}

void gg::TaskData::addChild(std::unique_ptr<ITask>&& task)
{
}

uint32_t gg::TaskData::getElapsedMs() const
{
	return uint32_t();
}

bool gg::TaskData::hasEvent() const
{
	return false;
}

std::shared_ptr<gg::IEvent> gg::TaskData::getNextEvent()
{
	return std::shared_ptr<IEvent>();
}

void gg::TaskData::sendEvent(std::shared_ptr<gg::IEvent> event)
{
}

void gg::TaskData::finish()
{
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

std::shared_ptr<gg::IThread> gg::Framework::createThread(const std::string& name) const
{
	std::shared_ptr<IThread> thread(new Thread(name));

	return thread;
}

std::shared_ptr<gg::IThread> gg::Framework::getThread(const std::string& name) const
{
	return std::shared_ptr<IThread>();
}
