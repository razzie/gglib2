#include <iostream>
#include "gg/fastmutex.hpp"
#include "gg/streamutil.hpp"
#include "gg/var.hpp"
#include "gg/function.hpp"
#include "gg/timer.hpp"
#include "gg/storage.hpp"
#include "gg/serializer.hpp"
#include "buffer.hpp"
#include "gg/filesystem.hpp"
#include "gg/logger.hpp"
#include "gg/message.hpp"
#include "expression.hpp"
#include "gg/console.hpp"

#define D3D_DEBUG_INFO
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

int main()
{
	gg::console.init();

	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	return 0;
}
