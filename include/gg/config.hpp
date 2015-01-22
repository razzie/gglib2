/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_CONFIG_IMPL_HPP_INCLUDED
#define GG_CONFIG_IMPL_HPP_INCLUDED

#if defined GG_STATIC
#	define GG_API
#elif defined GG_BUILD_DLL
#	define GG_API __declspec(dllexport)
#else
#	define GG_API __declspec(dllimport)
#endif

#endif // GG_CONFIG_IMPL_HPP_INCLUDED
