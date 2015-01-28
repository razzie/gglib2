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

#ifdef _WIN32
#include "renderer/font.hpp"
#include "renderer/D3D9_renderer.hpp"


gg::D3D9TextObject::D3D9TextObject() :
	m_color(0xffffffff),
	m_font(gg::getNormalFont()),
	m_height(0)
{

}

gg::D3D9TextObject::~D3D9TextObject()
{
}

bool gg::D3D9TextObject::setText(const std::string& text, unsigned line_spacing, const gg::Font* font)
{
	m_font = (font != nullptr) ? font : gg::getNormalFont();
	return false;
}

bool gg::D3D9TextObject::setColor(gg::Color color)
{
	m_color = color;
	return true;
}

unsigned gg::D3D9TextObject::getHeight() const
{
	return m_height;
}

gg::IRenderer::Backend gg::D3D9TextObject::getBackend() const
{
	return IRenderer::Backend::DIRECT3D9;
}


gg::D3D9Renderer::D3D9Renderer(HWND hwnd, IDirect3DDevice9* device) :
	m_hwnd(hwnd),
	m_device(device),
	m_drawing(false)
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
	return new D3D9TextObject();
}

void gg::D3D9Renderer::render()
{
	// begin

	m_drawing = true;
	IRenderer::invokeRenderCallback();
	m_drawing = false;

	// end
}

bool gg::D3D9Renderer::drawTextObject(gg::ITextObject* itext, int x, int y, Color* color_ptr)
{
	if (!m_drawing)
		return false;

	if (itext->getBackend() != Backend::DIRECT3D9)
		return false;

	D3D9TextObject* text = static_cast<D3D9TextObject*>(itext);

	return true;
}

bool gg::D3D9Renderer::drawCaret(gg::ITextObject* itext, int x, int y, int pos, gg::Color color)
{
	if (!m_drawing)
		return false;

	if (itext->getBackend() != Backend::DIRECT3D9)
		return false;

	D3D9TextObject* text = static_cast<D3D9TextObject*>(itext);

	return true;
}

bool gg::D3D9Renderer::drawLine(int x1, int y1, int x2, int y2, gg::Color color)
{
	if (!m_drawing)
		return false;

	return true;
}

bool gg::D3D9Renderer::drawRectangle(int x, int y, int width, int height, gg::Color color)
{
	if (!m_drawing)
		return false;

	return true;
}

#endif // _WIN32
