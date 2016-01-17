/**
 * Copyright (c) 2014-2016 Gábor Görzsöny (www.gorzsony.com)
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


gg::Packet::Packet(Mode mode, Type type) :
	Stream(mode),
	m_type(type),
	m_data_len(0),
	m_data_pos(0)
{
}

gg::Packet::~Packet()
{
}

gg::IPacket::Type gg::Packet::getType() const
{
	return m_type;
}

const char* gg::Packet::getData() const
{
	return m_data;
}

size_t gg::Packet::getSize() const
{
	return m_data_len;
}

char* gg::Packet::getDataPtr()
{
	return m_data;
}

void gg::Packet::setSize(size_t size)
{
	m_data_len = size;
}

size_t gg::Packet::write(const char* ptr, size_t len)
{
	if (getMode() != Mode::SERIALIZE)
		throw SerializationError();

	if (BUF_SIZE - m_data_len < len)
		len = BUF_SIZE - m_data_len;

	std::memcpy(&m_data[m_data_len], ptr, len);
	m_data_len += len;
	return len;
}

size_t gg::Packet::read(char* ptr, size_t len)
{
	if (getMode() != Mode::DESERIALIZE)
		throw SerializationError();

	if (m_data_len - m_data_pos < len)
		len = m_data_len - m_data_pos;

	std::memcpy(ptr, &m_data[m_data_pos], len);
	m_data_pos += len;
	return len;
}


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

	StreamHeader head;
	if (m_backend->peek(reinterpret_cast<char*>(&head), sizeof(StreamHeader)) < sizeof(StreamHeader))
		return {};

	if (head.packet_size > Stream::BUF_SIZE)
		throw NetworkException("Too large packet");

	size_t expected_size = sizeof(StreamHeader) + head.packet_size + sizeof(StreamTail);
	if (m_backend->waitForData(expected_size, timeoutMs) < expected_size)
		return {};

	// now that we have the full packet, let's skip the first heading bytes we already know
	m_backend->read(reinterpret_cast<char*>(&head), sizeof(StreamHeader));

	std::shared_ptr<Packet> packet(new Packet(IStream::Mode::DESERIALIZE, head.packet_type));
	m_backend->read(packet->getDataPtr(), head.packet_size);
	packet->setSize(head.packet_size);

	StreamTail tail;
	m_backend->read(reinterpret_cast<char*>(&tail), sizeof(StreamTail));
	if (!tail.ok())
		throw NetworkException("Corrupted packet");

	return packet;
}

gg::PacketPtr gg::Connection::createPacket(IPacket::Type type) const
{
	return PacketPtr( new Packet(IStream::Mode::SERIALIZE, type) );
}

gg::PacketPtr gg::Connection::createPacket(EventPtr event) const
{
	PacketPtr packet( new Packet(IStream::Mode::SERIALIZE, event->getType()) );
	event->serialize(*packet);
	return packet;
}

bool gg::Connection::send(PacketPtr packet)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	StreamHeader head;
	head.packet_size = static_cast<uint16_t>(packet->getSize());
	head.packet_type = packet->getType();

	StreamTail tail;

	size_t bytes_written = 0;
	bytes_written += m_backend->write(reinterpret_cast<const char*>(&head), sizeof(StreamHeader));
	bytes_written += m_backend->write(packet->getData(), head.packet_size);
	bytes_written += m_backend->write(reinterpret_cast<const char*>(&tail), sizeof(StreamTail));

	return (bytes_written == head.packet_size + sizeof(StreamHeader) + sizeof(StreamTail));
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
	return PacketPtr( new Packet(IStream::Mode::SERIALIZE, type) );
}

std::shared_ptr<gg::IPacket> gg::NetworkManager::createPacket(EventPtr event) const
{
	PacketPtr packet( new Packet(IStream::Mode::SERIALIZE, event->getType()) );
	event->serialize(*packet);
	return packet;
}
