#include "gg/streamutil.hpp"

using namespace gg;

OstreamManipulator<const char*> format(const char* f)
{
	OstreamManipulator<const char*>::Manipulator m =
		[](std::ostream& os, const char* fmt) -> std::ostream&
	{
		std::locale& loc = os.getloc();
		int i = 0;
		while (fmt[i] != 0)
		{
			if (fmt[i] != '%')
			{
				os << fmt[i];
				i++;
			}
			else
			{
				i++;
				if (fmt[i] == '%')
				{
					os << fmt[i];
					i++;
				}
				else
				{
					bool ok = true;
					int istart = i;
					bool more = true;
					int width = 0;
					int precision = 6;
					std::ios_base::fmtflags flags;
					char fill = ' ';
					bool alternate = false;
					while (more)
					{
						switch (fmt[i])
						{
						case '+':
							flags |= std::ios::showpos;
							break;
						case '-':
							flags |= std::ios::left;
							break;
						case '0':
							flags |= std::ios::internal;
							fill = '0';
							break;
						case '#':
							alternate = true;
							break;
						case ' ':
							break;
						default:
							more = false;
							break;
						}
						if (more) i++;
					}
					if (std::isdigit(fmt[i], loc))
					{
						width = std::atoi(fmt + i);
						do i++;
						while (std::isdigit(fmt[i], loc));
					}
					if (fmt[i] == '.')
					{
						i++;
						precision = std::atoi(fmt + i);
						while (std::isdigit(fmt[i], loc)) i++;
					}
					switch (fmt[i])
					{
					case 'd':
						flags |= std::ios::dec;
						break;
					case 'x':
						flags |= std::ios::hex;
						if (alternate) flags |= std::ios::showbase;
						break;
					case 'X':
						flags |= std::ios::hex | std::ios::uppercase;
						if (alternate) flags |= std::ios::showbase;
						break;
					case 'o':
						flags |= std::ios::hex;
						if (alternate) flags |= std::ios::showbase;
						break;
					case 'f':
						flags |= std::ios::fixed;
						if (alternate) flags |= std::ios::showpoint;
						break;
					case 'e':
						flags |= std::ios::scientific;
						if (alternate) flags |= std::ios::showpoint;
						break;
					case 'E':
						flags |= std::ios::scientific | std::ios::uppercase;
						if (alternate) flags |= std::ios::showpoint;
						break;
					case 'g':
						if (alternate) flags |= std::ios::showpoint;
						break;
					case 'G':
						flags |= std::ios::uppercase;
						if (alternate) flags |= std::ios::showpoint;
						break;
					default:
						ok = false;
						break;
					}
					i++;
					if (fmt[i] != 0) ok = false;
					if (ok)
					{
						os.unsetf(std::ios::adjustfield | std::ios::basefield |
							std::ios::floatfield);
						os.setf(flags);
						os.width(width);
						os.precision(precision);
						os.fill(fill);
					}
					else i = istart;
				}
			}
		}
		return os;
	};

	return OstreamManipulator<const char*>(m, f);
}
