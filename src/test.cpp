/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "gg/any.hpp"
#include "gg/console.hpp"
#include "gg/framework.hpp"
#include "gg/function.hpp"
#include "gg/logger.hpp"
#include "gg/network.hpp"
#include "gg/rwmutex.hpp"
#include "gg/storage.hpp"
#include "gg/streamutil.hpp"
#include "gg/timer.hpp"
#include "gg/optional.hpp"


gg::EventDefinition<1, int, float> foo_event;

class ConnectionTask : public gg::ITask
{
public:
	ConnectionTask(std::shared_ptr<gg::IConnection> connection) :
		m_connection(connection)
	{
	}

	virtual ~ConnectionTask() = default;

	virtual void run(gg::IThread& thread, gg::IThread::TaskOptions& options)
	{
		try
		{
			if (m_connection->alive())
			{
				auto packet = m_connection->getNextPacket(10);
				if (packet)
				{
					gg::log << "packet: length=" << packet->length() << ", type=" << packet->type() << std::endl;

					auto event = foo_event.create(*packet);
					if (event)
					{
						gg::log << "foo_event: " << event->params().get<int>(0) << ", " << event->params().get<float>(1) << std::endl;
					}
				}
			}
			else
			{
				options.finish();
			}
		}
		catch (std::exception& e)
		{
			gg::log << "exception: " << e.what() << std::endl;
		}
	}

private:
	std::shared_ptr<gg::IConnection> m_connection;
};

class ServerTask : public gg::ITask
{
public:
	ServerTask()
	{
		m_server = gg::net.createServer(12345);

		if (m_server->start())
			gg::log << "server started" << std::endl;
		else
			gg::log << "couldn't start server :(" << std::endl;
	}

	virtual ~ServerTask() = default;

	virtual void run(gg::IThread& thread, gg::IThread::TaskOptions& options)
	{
		try
		{
			if (m_server->alive())
			{
				auto connection = m_server->getNextConnection(10);
				if (connection)
				{
					gg::log << "connection: " << connection->getAddress() << std::endl;
					thread.addTask<ConnectionTask>(connection);
				}
			}
			else
			{
				options.finish();
			}
		}
		catch (std::exception& e)
		{
			gg::log << "exception: " << e.what() << std::endl;
		}
	}

private:
	std::shared_ptr<gg::IServer> m_server;
};


int main()
{
	auto server = gg::fw.createThread("server thread");
	server->addTask<ServerTask>();
	server->run();

	auto connection = gg::net.createConnection("localhost", 12345);
	if (!connection->connect())
	{
		gg::log << "Can't connect :(" << std::endl;
		server->clearTasks();
		server->join();
		return 1;
	}

	for (int i = 0; i < 5; ++i)
	{
		auto event = foo_event.create(1, 2.34f);
		auto packet = gg::net.createPacket(event);
		connection->send(packet);

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	server->clearTasks();
	server->join();
	return 0;
}
