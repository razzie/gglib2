/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <cstring>
#include <stdexcept>
#include "network_impl.hpp"
#include "backend_impl.hpp"

static gg::NetworkManager s_netmgr;
gg::INetworkManager& gg::net = s_netmgr;


gg::Connection::Connection(ConnectionBackendPtr&& backend) :
	m_backend(std::move(backend))
{
}

gg::Connection::~Connection()
{
	disconnect();
}

bool gg::Connection::connect(void* user_data)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	return m_backend->connect(user_data);
}

void gg::Connection::disconnect()
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	m_backend->disconnect();
}

bool gg::Connection::isAlive() const
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	return m_backend->isAlive();
}

const std::string& gg::Connection::getAddress() const
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	return m_backend->getAddress();
}

gg::PacketPtr gg::Connection::getNextPacket(uint32_t timeoutMs)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	ArchiveHeader head;
	if (m_backend->peek(reinterpret_cast<char*>(&head), sizeof(ArchiveHeader)) < sizeof(ArchiveHeader))
		return {};

	if (head.packet_size > Archive::BUF_SIZE)
		throw NetworkException("Too large packet");

	size_t expected_size = sizeof(ArchiveHeader) + head.packet_size + sizeof(ArchiveTail);
	if (m_backend->waitForData(expected_size, timeoutMs) < expected_size)
		return {};

	// now that we have the full packet, let's skip the first heading bytes we already know
	m_backend->read(reinterpret_cast<char*>(&head), sizeof(ArchiveHeader));

	std::shared_ptr<Packet> packet(new Packet(IArchive::Mode::SERIALIZE, head.packet_type));
	m_backend->read(packet->getDataPtr(), head.packet_size);
	packet->setSize(head.packet_size);

	ArchiveTail tail;
	m_backend->read(reinterpret_cast<char*>(&tail), sizeof(ArchiveTail));
	if (!tail.ok())
		throw NetworkException("Corrupted packet");

	return packet;
}

gg::PacketPtr gg::Connection::createPacket(IPacket::Type type) const
{
	return PacketPtr( new Packet(IArchive::Mode::DESERIALIZE, type) );
}

gg::PacketPtr gg::Connection::createPacket(EventPtr event) const
{
	PacketPtr packet( new Packet(IArchive::Mode::DESERIALIZE, event->getType()) );
	event->serialize(*packet);
	return packet;
}

bool gg::Connection::send(PacketPtr packet)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	ArchiveHeader head;
	head.packet_size = static_cast<uint16_t>(packet->getSize());
	head.packet_type = packet->getType();

	ArchiveTail tail;

	size_t bytes_written = 0;
	bytes_written += m_backend->write(reinterpret_cast<const char*>(&head), sizeof(ArchiveHeader));
	bytes_written += m_backend->write(packet->getData(), head.packet_size);
	bytes_written += m_backend->write(reinterpret_cast<const char*>(&tail), sizeof(ArchiveTail));

	return (bytes_written == head.packet_size + sizeof(ArchiveHeader) + sizeof(ArchiveTail));
}


gg::Server::Server(ServerBackendPtr&& backend) :
	m_backend(std::move(backend))
{
}

gg::Server::~Server()
{
	stop();
}

bool gg::Server::start(void* user_data)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	return m_backend->start(user_data);
}

void gg::Server::stop()
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	try
	{
		m_backend->stop();
	}
	catch (INetworkException&)
	{
		closeConnections();
		throw;
	}

	closeConnections();
}

bool gg::Server::isAlive() const
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	return m_backend->isAlive();
}

gg::ConnectionPtr gg::Server::getNextConnection(uint32_t timeoutMs)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	try
	{
		auto client_backend = m_backend->getNextConnection(timeoutMs);
		if (client_backend)
		{
			std::shared_ptr<Connection> client(new Connection(std::move(client_backend)));
			m_clients.push_back(client);
			return client;
		}
		else
		{
			return {};
		}
	}
	catch (INetworkException&)
	{
		stop();
		throw;
	}
}

void gg::Server::closeConnections()
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	for (auto& client : m_clients)
	{
		auto client_ptr = client.lock();
		if (client_ptr)
		{
			try
			{
				client_ptr->disconnect();
			}
			catch (INetworkException&)
			{
			}
		}
	}

	m_clients.clear();
}


gg::NetworkException::NetworkException(const char* what) :
	m_what(what)
{
}

gg::NetworkException::~NetworkException()
{
}

const char* gg::NetworkException::what() const
{
	return m_what;
}


gg::NetworkManager::NetworkManager()
{
#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

gg::NetworkManager::~NetworkManager()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

gg::ConnectionPtr gg::NetworkManager::createConnection(const std::string& host, uint16_t port) const
{
	ConnectionBackendPtr backend(new ConnectionBackend(host, port));
	return ConnectionPtr( new Connection(std::move(backend)) );
}

gg::ConnectionPtr gg::NetworkManager::createConnection(ConnectionBackendPtr&& backend) const
{
	return ConnectionPtr( new Connection(std::move(backend)) );
}

gg::ServerPtr gg::NetworkManager::createServer(uint16_t port) const
{
	ServerBackendPtr backend(new ServerBackend(port));
	return ServerPtr( new Server(std::move(backend)) );
}

gg::ServerPtr gg::NetworkManager::createServer(ServerBackendPtr&& backend) const
{
	return ServerPtr( new Server(std::move(backend)) );
}

std::shared_ptr<gg::IPacket> gg::NetworkManager::createPacket(IPacket::Type type) const
{
	return PacketPtr( new Packet(IArchive::Mode::DESERIALIZE, type) );
}

std::shared_ptr<gg::IPacket> gg::NetworkManager::createPacket(EventPtr event) const
{
	PacketPtr packet( new Packet(IArchive::Mode::DESERIALIZE, event->getType()) );
	event->serialize(*packet);
	return packet;
}
