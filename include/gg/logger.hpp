/**
* Copyright (c) 2014-2015 G�bor G�rzs�ny (www.gorzsony.com)
*
* This source is a private work and can be used only with the
* written permission of the author. Do not redistribute it!
* All rights reserved.
*/

#ifndef GG_LOGGER_HPP_INCLUDED
#define GG_LOGGER_HPP_INCLUDED

#include <iosfwd>

namespace gg
{
	class ILogger : public virtual std::ostream
	{
	public:
		virtual ~ILogger() {}
	};

	extern ILogger& log;
};

#endif // GG_LOGGER_HPP_INCLUDED
