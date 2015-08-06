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
	class IEventDefinitionBase;

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

		bool is(const IEventDefinitionBase&) const;

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

		template<unsigned N, class R = Param<N, Params...>::Type>
		static R get(std::shared_ptr<IEvent> event)
		{
			return event->get<R>(N);
		}

	private:
		template <int N, typename... T>
		struct Param;

		template <typename T0, typename... T>
		struct Param<0, T0, T...>
		{
			typedef T0 Type;
		};
		template <int N, typename T0, typename... T>
		struct Param<N, T0, T...>
		{
			typedef typename Param<N - 1, T...>::Type Type;
		};
	};

	inline bool IEvent::is(const IEventDefinitionBase& def) const
	{
		return (getType() == def.getType());
	}
};
