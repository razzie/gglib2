/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "gg/any.hpp"
#include "gg/console.hpp"
#include "gg/database.hpp"
#include "gg/function.hpp"
#include "gg/idgenerator.hpp"
#include "gg/logger.hpp"
#include "gg/network.hpp"
#include "gg/resource.hpp"
#include "gg/rwmutex.hpp"
#include "gg/serializable.hpp"
#include "gg/storage.hpp"
#include "gg/streamutil.hpp"
#include "gg/thread.hpp"
#include "gg/timer.hpp"
#include "gg/optional.hpp"
#include "gg/version.hpp"
#include <fstream>

gg::IDGenerator<> gen;

gg::SerializableEventDefinition<1, int, float> foo_event;

class ConnectionTask : public gg::ITask
{
public:
	ConnectionTask(gg::ConnectionPtr connection) :
		m_connection(connection)
	{
	}

	virtual ~ConnectionTask() = default;

	virtual void onStart(gg::ITaskOptions& options)
	{
		options.subscribe(foo_event);
	}

	virtual void onEvent(gg::ITaskOptions& options, gg::EventPtr event)
	{
		if (event->is(foo_event))
		{
			gg::log << "foo_event: " << foo_event.get<0>(event) << ", " << foo_event.get<1>(event) << std::endl;
		}
	}

	virtual void onUpdate(gg::ITaskOptions& options)
	{
		try
		{
			if (m_connection->isAlive())
			{
				auto packet = m_connection->getNextPacket(10);
				if (packet)
				{
					gg::log << "packet: length=" << packet->getSize() << ", type=" << packet->getType() << std::endl;

					auto event = foo_event(*packet);
					options.getThread().sendEvent(event); // accepts empty pointer too
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
	gg::ConnectionPtr m_connection;
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

	virtual void onEvent(gg::ITaskOptions& options, gg::EventPtr event)
	{
	}

	virtual void onUpdate(gg::ITaskOptions& options)
	{
		try
		{
			if (m_server->isAlive())
			{
				auto connection = m_server->getNextConnection(10);
				if (connection)
				{
					gg::log << "connection: " << connection->getAddress() << std::endl;
					options.getThread().addTask<ConnectionTask>(connection);
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
	gg::ServerPtr m_server;
};


int main()
{
	if (auto db = gg::db.open("test/database.db"))
	{
		auto table = db->getTable("fruit");
		if (!table)
		{
			table = db->createAndGetTable("fruit", { "apple", "pear", "grapes" });
			auto row = table->getNextRow(0);
			if (!row)
			{
				row = table->createAndGetRow();
				row->cell("apple")->set(1);
				row->cell("pear")->set(2);
				row->cell("grapes")->set(3);
			}
		}
		db->save();
	}

	if (auto db = gg::db.open("test/database.db"))
	{
		auto table = db->getTable("fruit");
		auto row = table->getNextRow(0);
		gg::log << row->cell("apple")->getString() << std::endl;
	}


	auto pool = gg::res.createResourcePool();
	pool->includeResource("test/resfolder.res");

	auto file = pool->openFile("resfolder.res/coffee_is_hot.jpg");
	if (file)
	{
		std::ofstream pic("test/results/coffee_is_hot.jpg", std::ios::out | std::ios::binary);
		pic.write(file->getData(), file->getSize());
		pic.close();
	}

	file = gg::res.getFileSerializer()->openFile("test/results/serialized.dat", gg::IFileSerializer::OpenMode::REWRITE);
	if (file)
	{
		int a = 1, b = 2, c = 3;
		gg::IFile& f = *file;
		f & a & b & c;

		file = gg::res.getFileSerializer()->openFile("test/results/serialized.dat", gg::IFileSerializer::OpenMode::READ);
		if (file)
		{
			int b;
			file & b;
			gg::log << b << std::endl;
		}
	}


	for (int i = 0; i < 8; ++i)
		gg::log << gen.next() << ", ";
	gg::log << std::endl;

	auto server = gg::threadmgr.createThread("server thread");
	server->addTask<ServerTask>();
	server->run();

	auto connection = gg::net.createConnection("localhost", 12345);
	if (!connection->connect())
	{
		gg::log << "Can't connect :(" << std::endl;
		server->finishTasks();
		server->join();
		return 1;
	}

	for (int i = 0; i < 5; ++i)
	{
		auto event = foo_event(1, 2.34f);
		auto packet = gg::net.createPacket(event);
		connection->send(packet);

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	server->finishTasks();
	server->join();

	return 0;
}
