/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

/**
 * Events enable communication between processes or threads of a single
 * process. Once an event is created, its type and parameters cannot be
 * modified (except for special parameters like IEvent::Flag). This helps
 * avoiding race conditions.
 *
 * The preferred way of defining an event is having an extern IEventDefinition
 * reference in a public header file and putting the implementation to a source
 * file:
 *
 * myevents.hpp
 * ------------
 * extern gg::IEventDefinition<int, char>& foo_event;
 *
 * myevents.cpp
 * ------------
 * static gg::SerializableEventDefinition<TYPE, int, char> _foo_event;
 * gg::IEventDefinition<int, char>& foo_event = _foo_event;
 *
 * main.cpp
 * --------
 * auto event = foo_event(1, 'a');
 * thread->sendEvent(event);
 *
 * NOTE:
 * For implemented events and event definitions include one of the following
 * headers: gg/network.hpp or gg/thread.hpp
 */

#pragma once

#include <cstdint>
#include <memory>
#include "gg/serializable.hpp"
#include "gg/storage.hpp"

namespace gg
{
	class IEventDefinitionBase;

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

		// Flag can be used as a parameter to indicate event state
		// (eg: event is consumed)
		class Flag
		{
		public:
			Flag() : m_flag(false) {}
			Flag(bool flag) : m_flag(flag) {}
			Flag(const Flag& f) : m_flag(f.m_flag) {}
			~Flag() = default;
			void set() const { m_flag = true; }
			void unset() const { m_flag = false; }
			bool isSet() const { return m_flag; }
			operator bool() const { return m_flag; }

		private:
			mutable bool m_flag;
		};
	};

	typedef std::shared_ptr<IEvent> EventPtr;

	class IEventDefinitionBase
	{
	public:
		virtual ~IEventDefinitionBase() = default;
		virtual IEvent::Type getType() const = 0;
		virtual EventPtr operator()() const = 0;
		virtual EventPtr operator()(IPacket&) const = 0;
	};

	template<class... Params>
	class IEventDefinition : public IEventDefinitionBase
	{
	public:
		virtual ~IEventDefinition() = default;
		virtual IEvent::Type getType() const = 0;
		virtual EventPtr operator()() const = 0;
		virtual EventPtr operator()(IPacket&) const = 0;
		virtual EventPtr operator()(Params... params) const = 0;

		template<unsigned N, class R = Param<N, Params...>::Type>
		static R get(EventPtr event)
		{
			return event->get<R>(N);
		}

	private:
		template<int N, class... T>
		struct Param;

		template<class T0, class... T>
		struct Param<0, T0, T...>
		{
			typedef T0 Type;
		};

		template<int N, class T0, class... T>
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
