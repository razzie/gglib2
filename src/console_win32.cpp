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


static void hookVtableFunc(void* obj, unsigned index, void* hook_func, void*& orig_func)
{
	void** vtable = (void**)(*((void**)obj));
	DWORD old_protect;

	// storage pointer to original function
	orig_func = vtable[index];

	// replace pointer to hook function
	VirtualProtect(&vtable[index], sizeof(void*), PAGE_READWRITE, &old_protect);
	vtable[index] = hook_func;
	VirtualProtect(&vtable[index], sizeof(void*), old_protect, &old_protect);
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
	typedef HRESULT(STDMETHODCALLTYPE* ENDSCENE)(IDirect3DDevice9 FAR*);
	static ENDSCENE EndScene_orig;
	static HRESULT STDMETHODCALLTYPE EndScene_hook(IDirect3DDevice9 FAR* This)
	{
		console->render();
		return EndScene_orig(This);
	}

	typedef HRESULT(STDMETHODCALLTYPE* CREATEDEVICE)(IDirect3D9 FAR*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);
	static CREATEDEVICE CreateDevice_orig;
	static HRESULT STDMETHODCALLTYPE CreateDevice_hook(
		IDirect3D9 FAR* This,
		UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
		DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters,
		IDirect3DDevice9** ppReturnedDeviceInterface)
	{
		HRESULT result = CreateDevice_orig(
			This, Adapter, DeviceType, hFocusWindow, BehaviorFlags,
			pPresentationParameters, ppReturnedDeviceInterface);

		wnd::hookWnd(hFocusWindow);

		hookVtableFunc(*ppReturnedDeviceInterface, 42, EndScene_hook, (void*&)EndScene_orig);
		UnhookFunction((ULONG_PTR)CreateDevice_hook);
		TwInit(TW_DIRECT3D9, *ppReturnedDeviceInterface);

		return result;
	}

	static void setupHooks()
	{
		typedef IDirect3D9*(WINAPI *DIRECT3DCREATE9)(UINT);

		HMODULE DX9Library = LoadLibrary(TEXT("d3d9.dll"));
		DIRECT3DCREATE9 Direct3DCreate9_orig = (DIRECT3DCREATE9)GetProcAddress(DX9Library, "Direct3DCreate9");

		IDirect3D9* pD3D = Direct3DCreate9_orig(D3D_SDK_VERSION);
		hookVtableFunc(pD3D, 16, CreateDevice_hook, (void*&)CreateDevice_orig);
		pD3D->Release();
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
