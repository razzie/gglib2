/**
 * Copyright (c) 2014-2015 G�bor G�rzs�ny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "gg/any.hpp"
#include "gg/console.hpp"
#include "gg/fastmutex.hpp"
#include "gg/framework.hpp"
#include "gg/function.hpp"
#include "gg/logger.hpp"
#include "gg/network.hpp"
#include "gg/storage.hpp"
#include "gg/streamutil.hpp"
#include "gg/timer.hpp"
#include "gg/optional.hpp"
#include <thread>

struct Foo
{
	int a;
	int b;
	int c;
};

void serialize(gg::IPacket& packet, Foo& foo)
{
	packet & foo.a & foo.b & foo.c;
}

std::ostream& operator<< (std::ostream& o, const Foo& foo)
{
	o << foo.a << ", " << foo.b << ", " << foo.c;
	return o;
}

void serverThread()
{
	auto server = gg::net.createServer(12345);
	if (server->start())
	{
		std::cout << "server started" << std::endl;
	}
	else
	{
		std::cout << "couldn't start server :(" << std::endl;
		return;
	}

	std::vector<std::shared_ptr<gg::IConnection>> connections;

	try
	{
		bool quit = false;
		while (server->alive() && !quit)
		{
			auto newConn = server->getNextConnection(10);
			if (newConn)
			{
				std::cout << "connection: " << newConn->getAddress() << std::endl;
				connections.push_back(newConn);
			}

			for (auto& conn : connections)
			{
				auto packet = conn->getNextPacket(10);
				if (packet)
				{
					std::cout << "packet: length=" << packet->length() << ", type=" << packet->type() << std::endl;

					if (packet->type() == 1)
					{
						Foo foo;
						packet & foo;
						std::cout << foo << std::endl;

						if (foo.a == 1)
							quit = true;
					}
				}
			}
		}
	}
	catch (std::exception& e)
	{
		std::cout << "exception: " << e.what() << std::endl;
	}

	std::cout << "server thread exit" << std::endl;
}


int main()
{
	std::thread server(serverThread);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	auto connection = gg::net.createConnection("localhost", 12345);
	if (!connection->connect())
	{
		std::cout << "Can't connect" << std::endl;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	Foo foo = { 1, 2, 3 };
	auto packet = connection->createPacket(1);
	packet & foo;
	connection->send(packet);

	std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	server.join();

	return 0;
}
