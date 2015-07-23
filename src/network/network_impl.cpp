/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <cassert>
#include <cstring>
#include <stdexcept>
#include "network_impl.hpp"
#include "backend_impl.hpp"
#include "ieee754.hpp"

static gg::NetworkManager s_netmgr;
gg::INetworkManager& gg::net = s_netmgr;


/*static bool isBigEndian()
{
	union
	{
		uint32_t i;
		char c[4];
	} chk = { 0x01020304 };

	return (chk.c[0] == 1);
}*/


gg::Packet::Packet(gg::IPacket::Mode mode, gg::IPacket::Type type) :
	m_mode(mode),
	m_type(type),
	m_data_len(0),
	m_data_pos(0)
{
}

gg::Packet::~Packet()
{
}

gg::IPacket::Mode gg::Packet::mode() const
{
	return m_mode;
}

gg::IPacket::Type gg::Packet::type() const
{
	return m_type;
}

char* gg::Packet::data()
{
	return m_data;
}

const char* gg::Packet::data() const
{
	return m_data;
}

size_t gg::Packet::length() const
{
	return m_data_len;
}

size_t& gg::Packet::length()
{
	return m_data_len;
}

gg::Packet& gg::Packet::operator&(int8_t& i)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&i), sizeof(int8_t)) < sizeof(int8_t))
			throw SerializationError();
	}
	else
	{
		int8_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(int8_t)) < sizeof(int8_t))
			throw SerializationError();
		i = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(int16_t& i)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&i), sizeof(int16_t)) < sizeof(int16_t))
			throw SerializationError();
	}
	else
	{
		int16_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(int16_t)) < sizeof(int16_t))
			throw SerializationError();
		i = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(int32_t& i)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&i), sizeof(int32_t)) < sizeof(int32_t))
			throw SerializationError();
	}
	else
	{
		int32_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(int32_t)) < sizeof(int32_t))
			throw SerializationError();
		i = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(int64_t& i)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&i), sizeof(int64_t)) < sizeof(int64_t))
			throw SerializationError();
	}
	else
	{
		int64_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(int64_t)) < sizeof(int64_t))
			throw SerializationError();
		i = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(uint8_t& u)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&u), sizeof(uint8_t)) < sizeof(uint8_t))
			throw SerializationError();
	}
	else
	{
		int8_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(uint8_t)) < sizeof(uint8_t))
			throw SerializationError();
		u = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(uint16_t& u)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&u), sizeof(uint16_t)) < sizeof(uint16_t))
			throw SerializationError();
	}
	else
	{
		int16_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(uint16_t)) < sizeof(uint16_t))
			throw SerializationError();
		u = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(uint32_t& u)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&u), sizeof(uint32_t)) < sizeof(uint32_t))
			throw SerializationError();
	}
	else
	{
		int32_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(uint32_t)) < sizeof(uint32_t))
			throw SerializationError();
		u = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(uint64_t& u)
{
	if (m_mode == Mode::WRITE)
	{
		if (write(reinterpret_cast<const char*>(&u), sizeof(uint64_t)) < sizeof(uint64_t))
			throw SerializationError();
	}
	else
	{
		int64_t tmp;
		if (read(reinterpret_cast<char*>(&tmp), sizeof(uint64_t)) < sizeof(uint64_t))
			throw SerializationError();
		u = tmp;
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(float& f)
{
	if (m_mode == Mode::WRITE)
	{
		uint32_t tmp = static_cast<uint32_t>(pack754_32(f));
		*this & tmp;
	}
	else
	{
		uint32_t tmp;
		*this & tmp;
		f = static_cast<float>(unpack754_32(tmp));
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(double& f)
{
	if (m_mode == Mode::WRITE)
	{
		uint64_t tmp = pack754_64(f);
		*this & tmp;
	}
	else
	{
		uint64_t tmp;
		*this & tmp;
		f = static_cast<float>(unpack754_64(tmp));
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(std::string& str)
{
	if (m_mode == Mode::WRITE)
	{
		uint16_t len = static_cast<uint16_t>(str.length());
		*this & len;
		if (write(str.c_str(), len) < len)
			throw SerializationError();
	}
	else
	{
		uint16_t len;
		*this & len;
		str.resize(len + 1);
		if (read(&str[0], len) < len)
			throw SerializationError();
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(gg::IBlob& blob)
{
	if (m_mode == Mode::WRITE)
	{
		uint16_t len = static_cast<uint16_t>(blob.length());
		*this & len;
		if (write(blob.data(), len) < len)
			throw SerializationError();
	}
	else
	{
		uint16_t len;
		*this & len;
		if (blob.length() != len || read(blob.data(), len) < len)
			throw SerializationError();
	}

	return *this;
}

gg::Packet& gg::Packet::operator&(ISerializable& serializable)
{
	serializable.serialize(*this);
	return *this;
}

size_t gg::Packet::write(const char* ptr, size_t len)
{
	assert(m_mode == Mode::WRITE);

	if (BUF_SIZE - m_data_len < len)
		len = BUF_SIZE - m_data_len;

	std::memcpy(&m_data[m_data_len], ptr, len);
	m_data_len += len;
	return len;
}

size_t gg::Packet::read(char* ptr, size_t len)
{
	assert(m_mode == Mode::READ);

	if (m_data_len - m_data_pos < len)
		len = m_data_len - m_data_pos;

	std::memcpy(ptr, &m_data[m_data_pos], len);
	m_data_pos += len;
	return len;
}


const char* gg::SerializationError::what() const
{
	return "Serialization error";
}


gg::Connection::Connection(std::unique_ptr<gg::IConnectionBackend>&& backend) :
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

bool gg::Connection::alive() const
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	return m_backend->alive();
}

const std::string& gg::Connection::getAddress() const
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	return m_backend->getAddress();
}

std::shared_ptr<gg::IPacket> gg::Connection::getNextPacket(uint32_t timeoutMs)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	PacketHeader head;
	if (m_backend->peek(reinterpret_cast<char*>(&head), sizeof(PacketHeader)) < sizeof(PacketHeader))
		return {};

	if (head.packet_size > Packet::BUF_SIZE)
		throw NetworkException("Too large packet");

	size_t expected_size = sizeof(PacketHeader) + head.packet_size + sizeof(PacketTail);
	if (m_backend->waitForData(expected_size, timeoutMs) < expected_size)
		return {};

	// now that we have the full packet, let's skip the first heading bytes we already know
	m_backend->read(reinterpret_cast<char*>(&head), sizeof(PacketHeader));

	std::shared_ptr<Packet> packet(new Packet(IPacket::Mode::READ, head.packet_type));
	m_backend->read(packet->data(), head.packet_size);
	packet->length() = head.packet_size;

	PacketTail tail;
	m_backend->read(reinterpret_cast<char*>(&tail), sizeof(PacketTail));
	if (!tail.ok())
		throw NetworkException("Corrupted packet");

	return packet;
}

std::shared_ptr<gg::IPacket> gg::Connection::createPacket(gg::IPacket::Type type) const
{
	return std::shared_ptr<IPacket>( new Packet(IPacket::Mode::WRITE, type) );
}

std::shared_ptr<gg::IPacket> gg::Connection::createPacket(std::shared_ptr<gg::IEvent> event) const
{
	std::shared_ptr<IPacket> packet( new Packet(IPacket::Mode::WRITE, event->type()) );
	event->serialize(*packet);
	return packet;
}

bool gg::Connection::send(std::shared_ptr<gg::IPacket> packet)
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);

	PacketHeader head;
	head.packet_size = static_cast<uint16_t>(packet->length());
	head.packet_type = packet->type();

	PacketTail tail;

	size_t bytes_written = 0;
	bytes_written += m_backend->write(reinterpret_cast<const char*>(&head), sizeof(PacketHeader));
	bytes_written += m_backend->write(packet->data(), head.packet_size);
	bytes_written += m_backend->write(reinterpret_cast<const char*>(&tail), sizeof(PacketTail));

	return (bytes_written == head.packet_size + sizeof(PacketHeader) + sizeof(PacketTail));
}


gg::Server::Server(std::unique_ptr<gg::IServerBackend>&& backend) :
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

bool gg::Server::alive() const
{
	std::lock_guard<decltype(m_mutex)> guard(m_mutex);
	return m_backend->alive();
}

std::shared_ptr<gg::IConnection> gg::Server::getNextConnection(uint32_t timeoutMs)
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

std::shared_ptr<gg::IConnection> gg::NetworkManager::createConnection(const std::string& host, uint16_t port) const
{
	std::unique_ptr<gg::IConnectionBackend> backend(new ConnectionBackend(host, port));
	return std::shared_ptr<IConnection>( new Connection(std::move(backend)) );
}

std::shared_ptr<gg::IConnection> gg::NetworkManager::createConnection(std::unique_ptr<gg::IConnectionBackend>&& backend) const
{
	return std::shared_ptr<IConnection>( new Connection(std::move(backend)) );
}

std::shared_ptr<gg::IServer> gg::NetworkManager::createServer(uint16_t port) const
{
	std::unique_ptr<gg::IServerBackend> backend(new ServerBackend(port));
	return std::shared_ptr<IServer>( new Server(std::move(backend)) );
}

std::shared_ptr<gg::IServer> gg::NetworkManager::createServer(std::unique_ptr<gg::IServerBackend>&& backend) const
{
	return std::shared_ptr<IServer>( new Server(std::move(backend)) );
}

std::shared_ptr<gg::IPacket> gg::NetworkManager::createPacket(gg::IPacket::Type type) const
{
	return std::shared_ptr<IPacket>( new Packet(IPacket::Mode::WRITE, type) );
}

std::shared_ptr<gg::IPacket> gg::NetworkManager::createPacket(std::shared_ptr<gg::IEvent> event) const
{
	std::shared_ptr<IPacket> packet( new Packet(IPacket::Mode::WRITE, event->type()) );
	event->serialize(*packet);
	return packet;
}
