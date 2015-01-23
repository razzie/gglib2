/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "console_impl.hpp"

#ifdef _WIN32
#pragma warning (disable : 4005)

#include <windows.h>
//#define CINTERFACE
#include <d3d9.h>
#include <d3d10_1.h>
#include <d3d11.h>
#include "NtHookEngine/NtHookEngine.hpp"
#include "AntTweakBar/AntTweakBar.h" // public header
#include "AntTweakBar/TwPrecomp.h"
#include "AntTweakBar/TwMgr.h"
#include "AntTweakBar/TwColors.h"
#include "AntTweakBar/TwFonts.h"

#define FONT g_DefaultNormalFont

static gg::Console* console = nullptr;
static HWND console_hwnd = 0;


static BOOL HookVtableFunction(void* obj, ULONG_PTR hook_func, unsigned index)
{
	UINT_PTR* vtable = (UINT_PTR*)(*((UINT_PTR*)obj));
	UINT_PTR orig_func = vtable[index];
	return HookFunction(orig_func, hook_func);
}

namespace wnd
{
	static WNDPROC orig_wnd_proc;

	static LRESULT CALLBACK consoleWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return CallWindowProc(orig_wnd_proc, hwnd, uMsg, wParam, lParam);
	}

	static void hookWnd(HWND hwnd)
	{
		static bool hooked = false;

		if (!hooked)
		{
			orig_wnd_proc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)consoleWndProc);
			console_hwnd = hwnd;
		}
	}
};

namespace ogl
{
	static HGLRC WINAPI wglCreateContext_hook(HDC hdc)
	{
		typedef HGLRC(WINAPI *WGLCREATECONTEXT)(HDC);

		HGLRC hglrc = ((WGLCREATECONTEXT)GetOriginalFunction((ULONG_PTR)wglCreateContext_hook))(hdc);

		wnd::hookWnd(WindowFromDC(hdc));
		UnhookFunction((ULONG_PTR)wglCreateContext_hook);
		TwInit(TW_OPENGL, NULL);

		return hglrc;
	}

	static BOOL WINAPI SwapBuffers_hook(HDC hdc)
	{
		typedef BOOL(WINAPI *SWAPBUFFERS)(HDC);

		console->render();
		return ((SWAPBUFFERS)GetOriginalFunction((ULONG_PTR)SwapBuffers_hook))(hdc);
	}

	static void setupHooks()
	{
		HMODULE OGLLibrary = LoadLibrary(TEXT("opengl32.dll"));
		HookFunction((ULONG_PTR)GetProcAddress(OGLLibrary, "wglCreateContext"), (ULONG_PTR)wglCreateContext_hook);

		HMODULE GDILibrary = LoadLibrary(TEXT("gdi32.dll"));
		HookFunction((ULONG_PTR)GetProcAddress(GDILibrary, "SwapBuffers"), (ULONG_PTR)SwapBuffers_hook);
	}
};

namespace dx9
{
	/*static HRESULT STDMETHODCALLTYPE Present_hook(
		IDirect3DDevice9 FAR* This, CONST RECT* pSourceRect,
		CONST RECT* pDestRect, HWND hDestWindowOverride,
		CONST RGNDATA* pDirtyRegion)
	{
		typedef HRESULT(STDMETHODCALLTYPE* PRESENT)(IDirect3DDevice9 FAR*, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*);

		console->render();

		return ((PRESENT)GetOriginalFunction((ULONG_PTR)Present_hook))
			(This, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	}*/

	/*static HRESULT STDMETHODCALLTYPE Present_hook(
		IDirect3DSwapChain9 FAR* This, CONST RECT* pSourceRect,
		CONST RECT* pDestRect, HWND hDestWindowOverride,
		CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
	{
		typedef HRESULT(STDMETHODCALLTYPE* PRESENT)(IDirect3DSwapChain9 FAR*, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*, DWORD);

		console->render();

		return ((PRESENT)GetOriginalFunction((ULONG_PTR)Present_hook))
			(This, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
	}*/

	static HRESULT STDMETHODCALLTYPE EndScene_hook(IDirect3DDevice9 FAR* This)
	{
		typedef HRESULT(STDMETHODCALLTYPE* ENDSCENE)(IDirect3DDevice9 FAR*);

		console->render();
		return ((ENDSCENE)GetOriginalFunction((ULONG_PTR)EndScene_hook))(This);
	}

	static HRESULT STDMETHODCALLTYPE CreateDevice_hook(
		IDirect3D9 FAR* This,
		UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
		DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters,
		IDirect3DDevice9** ppReturnedDeviceInterface)
	{
		typedef HRESULT(STDMETHODCALLTYPE* CREATEDEVICE)(IDirect3D9 FAR*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);

		HRESULT result = ((CREATEDEVICE)GetOriginalFunction((ULONG_PTR)CreateDevice_hook))
			(This, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

		wnd::hookWnd(hFocusWindow);

		/*HookVtableFunction(*ppReturnedDeviceInterface, (ULONG_PTR)Present_hook, 17);*/
		/*IDirect3DSwapChain9* pSwapChain;
		(*ppReturnedDeviceInterface)->GetSwapChain(0, &pSwapChain);
		HookVtableFunction(pSwapChain, (ULONG_PTR)Present_hook, 3);*/
		HookVtableFunction(*ppReturnedDeviceInterface, (ULONG_PTR)EndScene_hook, 42);
		UnhookFunction((ULONG_PTR)CreateDevice_hook);
		TwInit(TW_DIRECT3D9, *ppReturnedDeviceInterface);

		return result;
	}

	static IDirect3D9* WINAPI Direct3DCreate9_hook(UINT SDKVersion)
	{
		typedef IDirect3D9*(WINAPI *DIRECT3DCREATE9)(UINT);

		IDirect3D9* pD3D = ((DIRECT3DCREATE9)GetOriginalFunction((ULONG_PTR)Direct3DCreate9_hook))(SDKVersion);

		HookVtableFunction(pD3D, (ULONG_PTR)CreateDevice_hook, 16);
		UnhookFunction((ULONG_PTR)Direct3DCreate9_hook);

		return pD3D;
	}

	static void setupHooks()
	{
		HMODULE DX9Library = LoadLibrary(TEXT("d3d9.dll"));
		HookFunction((ULONG_PTR)GetProcAddress(DX9Library, "Direct3DCreate9"), (ULONG_PTR)Direct3DCreate9_hook);
	}
};

namespace dx10
{
	static void setupHooks()
	{
	}
}

namespace dx11
{
	static void setupHooks()
	{
	}
}

bool gg::Console::init()
{
	static bool initialized = false;

	if (initialized) return false;
	initialized = true;

	::console = this;

	initNtHookEngine();
	ogl::setupHooks();
	dx9::setupHooks();
	dx10::setupHooks();
	dx11::setupHooks();

	return true;
}

static void getLines(const std::string& str, std::vector<std::string>& lines)
{
	auto it1 = str.begin(), it2 = str.begin(), end = str.end();
	for (; it2 != end; ++it2)
	{
		if (*it2 == '\n')
		{
			if (it1 != it2) lines.emplace_back(it1, it2);
			it1 = it2 + 1;
		}
	}
	lines.emplace_back(it2, end);
}

void gg::Console::render()
{
	if (!m_render) return;

	std::lock_guard<std::recursive_mutex> guard(m_mutex);

	RECT client_rect;
	GetClientRect(console_hwnd, &client_rect);
	g_TwMgr->m_Graph->BeginDraw(client_rect.right, client_rect.bottom);

	std::vector<std::string> lines;
	std::vector<color32> colors;
	unsigned curr_height = 0;

	for (auto& output : m_output)
	{
		if (output.render_data == nullptr)
		{
			output.render_data = g_TwMgr->m_Graph->NewTextObj();
			output.dirty = true;
		}

		if (output.dirty)
		{
			lines.clear();
			getLines(output.text, lines);

			color32 color = 0xffffffff;

			switch (output.type)
			{
			case OutputData::Type::NORMAL:
				color = 0xffaaaaaa;
				break;
			case OutputData::Type::FUNCTION_CALL:
				color = 0xffaaaaff;
				break;
			case OutputData::Type::FUNCTION_OUTPUT:
				color = 0xffffffff;
				break;
			}

			colors.clear();
			colors.insert(colors.end(), lines.size(), color);

			output.lines = lines.size();
			g_TwMgr->m_Graph->BuildText(output.render_data, lines.data(), colors.data(), NULL, output.lines, FONT, 2, 0);
		}

		g_TwMgr->m_Graph->DrawText(output.render_data, 10, 10 + curr_height, 0, 0);
		curr_height += output.lines * (FONT->m_CharHeight + 2);
	}

	// for debugging
	g_TwMgr->m_Graph->DrawRect(10, 10, 20, 20, 0xffff0000);

	g_TwMgr->m_Graph->EndDraw();
}

#else

bool gg::Console::init()
{
	return false;
}

void gg::Console::render()
{

}

#endif
