/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "renderer/renderer.hpp"
#include "console_impl.hpp"

#ifdef _WIN32
//#pragma warning (disable : 4005)

#include <windows.h>
/*#include "AntTweakBar/AntTweakBar.h" // public header
#include "AntTweakBar/TwPrecomp.h"
#include "AntTweakBar/TwMgr.h"
#include "AntTweakBar/TwColors.h"
#include "AntTweakBar/TwFonts.h"

#define FONT g_DefaultNormalFont*/

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

void gg::Console::render(gg::IRenderer* renderer)
{
	if (!m_render) return;

	if (!console_hwnd) wnd::hookWnd((HWND)renderer->getWindowHandle());

	std::lock_guard<std::recursive_mutex> guard(m_mutex);

	/*RECT client_rect;
	GetClientRect(console_hwnd, &client_rect);
	g_TwMgr->m_Graph->BeginDraw(client_rect.right, client_rect.bottom);

	std::vector<std::string> lines;
	std::vector<color32> colors;*/
	unsigned curr_height = 0;

	for (auto& output : m_output)
	{
		if (output.render_data == nullptr)
		{
			/*output.render_data = g_TwMgr->m_Graph->NewTextObj();*/
			output.render_data = renderer->createTextObject();
			output.dirty = true;
		}

		ITextObject* text = static_cast<ITextObject*>(output.render_data);

		if (output.dirty)
		{
			/*lines.clear();
			getLines(output.text, lines);

			/*color32*/Color color = 0xffffffff;

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

			/*colors.clear();
			colors.insert(colors.end(), lines.size(), color);

			output.lines = lines.size();
			g_TwMgr->m_Graph->BuildText(output.render_data, lines.data(), colors.data(), NULL, output.lines, FONT, 2, 0);*/
			text->setText(output.text);
			text->setColor(color);
		}

		/*g_TwMgr->m_Graph->DrawText(output.render_data, 10, 10 + curr_height, 0, 0);
		curr_height += output.lines * (FONT->m_CharHeight + 2);*/
		curr_height += text->getHeight();
	}

	// for debugging
	/*g_TwMgr->m_Graph->DrawRect(10, 10, 20, 20, 0xffff0000);

	g_TwMgr->m_Graph->EndDraw();*/
	renderer->drawRectangle(10, 10, 20, 20, 0xffff0000);
}

#else

bool gg::Console::init()
{
	return false;
}

void gg::Console::render(gg::IRenderer*)
{

}

#endif
