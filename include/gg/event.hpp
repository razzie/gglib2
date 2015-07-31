/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

/**
 * NOTE:
 * This is a helper file to avoid including gg/network.hpp
 */

#pragma once

#include <cstdint>
#include <memory>
#include "gg/storage.hpp"

namespace gg
{
	class IPacket;

	class ISerializable
	{
	public:
		virtual ~ISerializable() = default;
		virtual void serialize(IPacket&) = 0;
	};

	class IEvent : public ISerializable
	{
	public:
		typedef uint16_t Type;

		virtual ~IEvent() = default;
		virtual Type getType() const = 0;
		virtual const IStorage& getParams() const = 0;
		virtual void serialize(IPacket&) = 0;

		template<class T>
		const T& get(unsigned n) const
		{
			return getParams().get<T>(n);
		}
	};

	class IEventDefinitionBase
	{
	public:
		virtual ~IEventDefinitionBase() = default;
		virtual IEvent::Type getType() const = 0;
		virtual std::shared_ptr<IEvent> create() const = 0;
		virtual std::shared_ptr<IEvent> create(IPacket&) const = 0;
	};

	template<class... Params>
	class IEventDefinition : public IEventDefinitionBase
	{
	public:
		virtual ~IEventDefinition() = default;
		virtual IEvent::Type getType() const = 0;
		virtual std::shared_ptr<IEvent> create() const = 0;
		virtual std::shared_ptr<IEvent> create(IPacket&) const = 0;
		virtual std::shared_ptr<IEvent> create(Params... params) const = 0;
	};
};
