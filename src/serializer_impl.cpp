/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <map>
#include <typeindex>
#include "gg/fastmutex.hpp"
#include "safebuffer.hpp"
#include "ieee754.hpp"
#include "serializer_impl.hpp"

static bool serializeFloat(const float& f, gg::Buffer& buf)
{
	uint64_t tmp = pack754_32(f);
	buf.write(reinterpret_cast<const char*>(&tmp), sizeof(tmp));
	return true;
}

static bool deserializeFloat(float& f, gg::Buffer& buf)
{
	uint64_t tmp;
	if (buf.read(reinterpret_cast<char*>(&tmp), sizeof(tmp)) < sizeof(tmp))
		return false;
	f = unpack754_32(tmp);
	return true;
}

static bool serializeDouble(const double& f, gg::Buffer& buf)
{
	uint64_t tmp = pack754_64(f);
	buf.write(reinterpret_cast<const char*>(&tmp), sizeof(tmp));
	return true;
}

static bool deserializeDouble(double& f, gg::Buffer& buf)
{
	uint64_t tmp;
	if (buf.read(reinterpret_cast<char*>(&tmp), sizeof(tmp)) < sizeof(tmp))
		return false;
	f = unpack754_64(tmp);
	return true;
}

static bool serializeString(const std::string& str, gg::Buffer& buf)
{
	const uint16_t len = str.length();
	buf.write(reinterpret_cast<const char*>(&len), sizeof(len));
	buf.write(str.c_str(), len);
	return true;
}

static bool deserializeString(std::string& str, gg::Buffer& buf)
{
	uint16_t len;
	if (buf.read(reinterpret_cast<char*>(&len), sizeof(len)) < sizeof(len))
		return false;

	str.resize(len + 1);
	if (buf.read(&str[0], len) < len)
		return false;

	return true;
}


struct SerializableType
{
	const std::type_info* type;
	const size_t size;
	gg::SerializerFunction save_func;
	gg::DeserializerFunction init_func;
};

struct GlobalsSRL
{
	gg::FastMutex types_mutex;
	std::map<std::type_index, SerializableType> types;

	GlobalsSRL()
	{
		gg::addSerializableTrivialType<int8_t>();
		gg::addSerializableTrivialType<uint8_t>();
		gg::addSerializableTrivialType<int16_t>();
		gg::addSerializableTrivialType<uint16_t>();
		gg::addSerializableTrivialType<int32_t>();
		gg::addSerializableTrivialType<uint32_t>();
		gg::addSerializableTrivialType<int64_t>();
		gg::addSerializableTrivialType<uint64_t>();
		gg::addSerializableType<float>(serializeFloat, deserializeFloat);
		gg::addSerializableType<double>(serializeDouble, deserializeDouble);
		gg::addSerializableType<std::string>(serializeString, deserializeString);
	}
};

static GlobalsSRL globals;


bool gg::addSerializableType(const std::type_info& type, size_t size, gg::SerializerFunction save_func, gg::DeserializerFunction init_func)
{
	std::lock_guard<gg::FastMutex> guard(globals.types_mutex);
	return globals.types.emplace(std::type_index(type), SerializableType{ &type, size, save_func, init_func }).second;
}

bool gg::serialize(const gg::Var& var, gg::Buffer& buf)
{
	return gg::serialize(var.getType(), var.getPtr(), buf);
}

bool gg::serialize(const gg::VarArray& va, gg::Buffer& buf)
{
	gg::Buffer tmp_buf;

	for (const gg::Var& v : va)
	{
		if (!gg::serialize(v, tmp_buf))
			return false;
	}

	buf.copyFrom(tmp_buf);
	return true;
}

bool gg::serialize(const gg::IStorage& st, gg::Buffer& buf)
{
	gg::Buffer tmp_buf;

	for (unsigned i = 0, n = st.size(); i < n; ++i)
	{
		if (!gg::serialize(st.getType(i), st.getPtr(i), tmp_buf))
			return false;
	}

	buf.copyFrom(tmp_buf);
	return true;
}

bool gg::serialize(const std::type_info& type, const void* ptr, gg::Buffer& buf)
{
	SerializableType* data = nullptr;

	{ // exception-safe mutex locking
		std::lock_guard<gg::FastMutex> guard(globals.types_mutex);
		auto it = globals.types.find(type);
		if (it == globals.types.end())
			return false;
		else
			data = &it->second;
	}

	gg::Buffer tmp_buf;
	bool result = data->save_func(ptr, tmp_buf);

	if (result) buf.copyFrom(tmp_buf);
	return result;
}

bool gg::deserialize(gg::Var& var, gg::Buffer& buf)
{
	return gg::deserialize(var.getType(), var.getPtr(), buf);
}

bool gg::deserialize(gg::VarArray& va, gg::Buffer& buf)
{
	gg::SafeBuffer tmp_buf(buf);

	for (gg::Var& v : va)
	{
		if (!gg::deserialize(v, tmp_buf))
			return false;
	}

	tmp_buf.finalize();
	return true;
}

bool gg::deserialize(gg::IStorage& st, gg::Buffer& buf)
{
	gg::SafeBuffer tmp_buf(buf);

	for (unsigned i = 0, n = st.size(); i < n; ++i)
	{
		if (!gg::deserialize(st.getType(i), st.getPtr(i), tmp_buf))
			return false;
	}

	tmp_buf.finalize();
	return true;
}

bool gg::deserialize(const std::type_info& type, void* ptr, gg::Buffer& buf)
{
	SerializableType* data = nullptr;

	{ // exception-safe mutex locking
		std::lock_guard<gg::FastMutex> guard(globals.types_mutex);
		auto it = globals.types.find(type);
		if (it == globals.types.end())
			return false;
		else
			data = &it->second;
	}

	gg::SafeBuffer tmp_buf(buf);
	bool result = data->init_func(ptr, tmp_buf);

	if (result) tmp_buf.finalize();
	return result;
}
