/**
 * Copyright (c) 2014-2016 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifdef _WIN32
#include <memory>
#include <windows.h>
#include "renderer/renderer.hpp"
#include "renderer/opengl_renderer.hpp"
#include "renderer/d3d9_renderer.hpp"
#include "renderer/d3d11_renderer.hpp"
#include "NtHookEngine/NtHookEngine.hpp"

static std::shared_ptr<gg::IRenderer> renderer;
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

		if (auto tmp_renderer = renderer)
			tmp_renderer->render();

		return ((SWAPBUFFERS)GetOriginalFunction((ULONG_PTR)SwapBuffers_hook))(hdc);
	}

	static HGLRC WINAPI wglCreateContext_hook(HDC hdc)
	{
		typedef HGLRC(WINAPI *WGLCREATECONTEXT)(HDC);

		HGLRC hglrc = ((WGLCREATECONTEXT)GetOriginalFunction((ULONG_PTR)wglCreateContext_hook))(hdc);

		if (hglrc != NULL)
			renderer.reset(new gg::OpenGLRenderer(WindowFromDC(hdc), hglrc));

		return hglrc;
	}

	static BOOL WINAPI wglDeleteContext_hook(HGLRC hglrc)
	{
		typedef BOOL(WINAPI *WGLDELETECONTEXT)(HGLRC);

		if (renderer && renderer->getBackendHandle() == (void*)hglrc)
			renderer.reset();

		return ((WGLDELETECONTEXT)GetOriginalFunction((ULONG_PTR)wglDeleteContext_hook))(hglrc);
	}

	static void hook()
	{
		HMODULE OGLLibrary = LoadLibrary(TEXT("opengl32.dll"));
		HookFunction((ULONG_PTR)GetProcAddress(OGLLibrary, "wglCreateContext"), (ULONG_PTR)wglCreateContext_hook);
		HookFunction((ULONG_PTR)GetProcAddress(OGLLibrary, "wglDeleteContext"), (ULONG_PTR)wglDeleteContext_hook);

		HMODULE GDILibrary = LoadLibrary(TEXT("gdi32.dll"));
		HookFunction((ULONG_PTR)GetProcAddress(GDILibrary, "SwapBuffers"), (ULONG_PTR)SwapBuffers_hook);
	}
};

namespace dx9
{
	typedef ULONG(STDMETHODCALLTYPE* RELEASE)(IDirect3DDevice9 FAR*);
	static RELEASE Release_orig;
	static ULONG STDMETHODCALLTYPE Release_hook(IDirect3DDevice9 FAR* This)
	{
		if (renderer && renderer->getBackendHandle() == (void*)This)
		{
			ULONG refcnt = This->AddRef() - 1;
			Release_orig(This);

			if (refcnt == 1)
				renderer.reset();
		}

		return Release_orig(This);
	}

	typedef HRESULT(STDMETHODCALLTYPE* RESET)(IDirect3DDevice9 FAR*, D3DPRESENT_PARAMETERS*);
	static RESET Reset_orig;
	static HRESULT STDMETHODCALLTYPE Reset_hook(IDirect3DDevice9 FAR* This, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		if (auto tmp_renderer = renderer)
			tmp_renderer->reset();

		return Reset_orig(This, pPresentationParameters);
	}

	typedef HRESULT(STDMETHODCALLTYPE* ENDSCENE)(IDirect3DDevice9 FAR*);
	static ENDSCENE EndScene_orig;
	static HRESULT STDMETHODCALLTYPE EndScene_hook(IDirect3DDevice9 FAR* This)
	{
		if (auto tmp_renderer = renderer)
			tmp_renderer->render();

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
		static bool first_use = true;

		HRESULT result = CreateDevice_orig(
			This, Adapter, DeviceType, hFocusWindow, BehaviorFlags,
			pPresentationParameters, ppReturnedDeviceInterface);

		if (SUCCEEDED(result))
		{
			renderer.reset(new gg::D3D9Renderer(hFocusWindow, *ppReturnedDeviceInterface));

			if (first_use)
			{
				first_use = false;
				hookVtableFunc(*ppReturnedDeviceInterface, 2, Release_hook, (void*&)Release_orig);
				hookVtableFunc(*ppReturnedDeviceInterface, 16, Reset_hook, (void*&)Reset_orig);
				hookVtableFunc(*ppReturnedDeviceInterface, 42, EndScene_hook, (void*&)EndScene_orig);
			}
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
