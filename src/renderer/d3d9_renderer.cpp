/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "renderer/D3D9_renderer.hpp"

gg::D3D9Renderer::D3D9Renderer(HWND hwnd, IDirect3DDevice9* device) :
	m_hwnd(hwnd),
	m_device(device)
{

}

gg::D3D9Renderer::~D3D9Renderer()
{
}

gg::IRenderer::Backend gg::D3D9Renderer::getBackend() const
{
	return Backend::DIRECT3D9;
}

bool gg::D3D9Renderer::getWindowDimensions(unsigned* width, unsigned* height) const
{
	RECT client_rect;
	GetClientRect(m_hwnd, &client_rect);
	*width = client_rect.right;
	*height = client_rect.bottom;
	return true;
}

gg::WindowHandle gg::D3D9Renderer::getWindowHandle() const
{
	return (WindowHandle)m_hwnd;
}

gg::ITextObject* gg::D3D9Renderer::createTextObject() const
{
	return nullptr;
}

void gg::D3D9Renderer::render()
{
	// begin

	IRenderer::invokeRenderCallback();

	// end
}

bool gg::D3D9Renderer::drawTextObject(gg::ITextObject* text, int x, int y)
{
	return false;
}

bool gg::D3D9Renderer::drawRectangle(int x, int y, int width, int height, gg::Color color)
{
	return false;
}
