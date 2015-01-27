/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "console_impl.hpp"

#ifdef _WIN32
#include "renderer/renderer.hpp"
#include <windows.h>

static HWND console_hwnd = 0;


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

bool gg::Console::init()
{
	IRenderer::injectHooks();
	IRenderer::setRenderCallback(std::bind(&Console::render, this, std::placeholders::_1));
	return true;
}

void gg::Console::render(gg::IRenderer* renderer)
{
	if (!m_render) return;

	if (!console_hwnd) wnd::hookWnd((HWND)renderer->getWindowHandle());

	//renderer->drawRectangle(10, 10, 10, 10, 0xffff0000);

	std::lock_guard<std::recursive_mutex> guard(m_mutex);

	unsigned curr_height = 5;

	for (auto& output : m_output)
	{
		if (output.textobj == nullptr)
		{
			output.textobj = renderer->createTextObject();
			output.dirty = true;
		}

		if (output.dirty)
		{
			Color color;

			switch (output.type)
			{
			case OutputData::Type::NORMAL:
				color = 0xffdddddd;
				break;
			case OutputData::Type::FUNCTION_CALL:
				color = 0xffddffff;
				break;
			case OutputData::Type::FUNCTION_OUTPUT:
				color = 0xffddddff;
				break;
			}

			if (output.output_num % 2)
				color -= 0x00111111;

			output.textobj->setText(output.text);
			output.textobj->setColor(color);
		}

		Color shadow_color = 0xff000000;
		renderer->drawTextObject(output.textobj, 6, curr_height+1, &shadow_color);

		renderer->drawTextObject(output.textobj, 5, curr_height);
		curr_height += output.textobj->getHeight() + 2;
	}
}

#else // _WIN32

bool gg::Console::init()
{
	return false;
}

void gg::Console::render(gg::IRenderer*)
{

}

#endif // _WIN32
