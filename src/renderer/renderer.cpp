/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifdef _WIN32
#include <windows.h>
#include "renderer/renderer.hpp"
#include "renderer/opengl_renderer.hpp"
#include "renderer/d3d9_renderer.hpp"
#include "NtHookEngine/NtHookEngine.hpp"

static gg::IRenderer* renderer = nullptr;
static gg::RenderCallback render_cb;


static void hookVtableFunc(void* obj, unsigned index, void* hook_func, void*& orig_func)
{
	void** vtable = (void**)(*((void**)obj));
	DWORD old_protect;

	// store pointer to original function
	orig_func = vtable[index];

	// making vtable point to hook function
	VirtualProtect(&vtable[index], sizeof(void*), PAGE_READWRITE, &old_protect);
	vtable[index] = hook_func;
	VirtualProtect(&vtable[index], sizeof(void*), old_protect, &old_protect);
}

namespace ogl
{
	static BOOL WINAPI SwapBuffers_hook(HDC hdc)
	{
		typedef BOOL(WINAPI *SWAPBUFFERS)(HDC);

		if (renderer != nullptr)
			renderer->render();

		return ((SWAPBUFFERS)GetOriginalFunction((ULONG_PTR)SwapBuffers_hook))(hdc);
	}

	static HGLRC WINAPI wglCreateContext_hook(HDC hdc)
	{
		typedef HGLRC(WINAPI *WGLCREATECONTEXT)(HDC);

		HGLRC hglrc = ((WGLCREATECONTEXT)GetOriginalFunction((ULONG_PTR)wglCreateContext_hook))(hdc);

		//UnhookFunction((ULONG_PTR)wglCreateContext_hook);

		if (renderer == nullptr)
		{
			delete renderer;
			renderer = nullptr;
		}

		renderer = new gg::OpenGLRenderer(WindowFromDC(hdc));

		HMODULE GDILibrary = LoadLibrary(TEXT("gdi32.dll"));
		HookFunction((ULONG_PTR)GetProcAddress(GDILibrary, "SwapBuffers"), (ULONG_PTR)SwapBuffers_hook);

		return hglrc;
	}

	static void hook()
	{
		HMODULE OGLLibrary = LoadLibrary(TEXT("opengl32.dll"));
		HookFunction((ULONG_PTR)GetProcAddress(OGLLibrary, "wglCreateContext"), (ULONG_PTR)wglCreateContext_hook);
	}
};

namespace dx9
{
	typedef HRESULT(STDMETHODCALLTYPE* ENDSCENE)(IDirect3DDevice9 FAR*);
	static ENDSCENE EndScene_orig;
	static HRESULT STDMETHODCALLTYPE EndScene_hook(IDirect3DDevice9 FAR* This)
	{
		if (renderer != nullptr)
			renderer->render();

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

		if (renderer == nullptr)
		{
			delete renderer;
			renderer = nullptr;
		}

		renderer = new gg::D3D9Renderer(hFocusWindow, *ppReturnedDeviceInterface);

		static bool first_use = true;
		if (SUCCEEDED(result) && first_use)
		{
			hookVtableFunc(*ppReturnedDeviceInterface, 42, EndScene_hook, (void*&)EndScene_orig);
			first_use = false;
		}

		return result;
	}

	static void hook()
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
	static void hook()
	{
	}
}

namespace dx11
{
	static void hook()
	{
	}
}

bool gg::IRenderer::injectHooks()
{
	static bool initialized = false;

	if (initialized) return false;
	initialized = true;

	initNtHookEngine();
	ogl::hook();
	dx9::hook();
	dx10::hook();
	dx11::hook();

	return true;
}

void gg::IRenderer::setRenderCallback(gg::RenderCallback cb)
{
	render_cb = cb;
}

bool gg::IRenderer::invokeRenderCallback()
{
	if (render_cb)
	{
		render_cb(this);
		return true;
	}
	else
	{
		return false;
	}
}

#endif // _WIN32
