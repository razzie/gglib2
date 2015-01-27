/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

/**
 * Original code borrowed from AntTweakBar library under zlib/libpng license.
 */

#include "renderer/font.hpp"
#include "renderer/D3D9_renderer.hpp"


gg::D3D9TextObject::D3D9TextObject() :
	m_color(0xffffffff),
	m_font(gg::getNormalFont())
{

}

gg::D3D9TextObject::~D3D9TextObject()
{
}

bool gg::D3D9TextObject::setText(const std::string& text)
{
	return false;
}

bool gg::D3D9TextObject::setColor(gg::Color color)
{
	m_color = color;
	return true;
}

bool gg::D3D9TextObject::setFont(const gg::Font* font)
{
	m_font = font;
	return true;
}

unsigned gg::D3D9TextObject::getHeight() const
{
	return 0;
}

gg::IRenderer::Backend gg::D3D9TextObject::getBackend() const
{
	return IRenderer::Backend::DIRECT3D9;
}


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

gg::D3D9TextObject* gg::D3D9Renderer::createTextObject() const
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
