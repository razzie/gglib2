/**
* Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
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
	namespace log
	{
		extern std::ostream& console; // gglib console
		extern std::ostream& file;
		extern std::ostream& out; // standard output
	};
};

#endif // GG_LOGGER_HPP_INCLUDED
