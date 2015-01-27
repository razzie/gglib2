/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "renderer/font.hpp"
#include "renderer/opengl_renderer.hpp"


gg::OpenGLTextObject::OpenGLTextObject() :
	m_color(0xffffffff),
	m_font(gg::getNormalFont())
{

}

gg::OpenGLTextObject::~OpenGLTextObject()
{
}

bool gg::OpenGLTextObject::setText(const char* text)
{
	return false;
}

bool gg::OpenGLTextObject::setColor(gg::Color color)
{
	m_color = color;
	return true;
}

bool gg::OpenGLTextObject::setFont(const gg::Font* font)
{
	m_font = font;
	return true;
}

unsigned gg::OpenGLTextObject::getHeight() const
{
	return 0;
}

gg::IRenderer::Backend gg::OpenGLTextObject::getBackend() const
{
	return IRenderer::Backend::OPENGL;
}


gg::OpenGLRenderer::OpenGLRenderer(HWND hwnd) :
	m_hwnd(hwnd)
{

}

gg::OpenGLRenderer::~OpenGLRenderer()
{
}

gg::IRenderer::Backend gg::OpenGLRenderer::getBackend() const
{
	return Backend::OPENGL;
}

bool gg::OpenGLRenderer::getWindowDimensions(unsigned* width, unsigned* height) const
{
	RECT client_rect;
	GetClientRect(m_hwnd, &client_rect);
	*width = client_rect.right;
	*height = client_rect.bottom;
	return true;
}

gg::WindowHandle gg::OpenGLRenderer::getWindowHandle() const
{
	return (WindowHandle)m_hwnd;
}

gg::OpenGLTextObject* gg::OpenGLRenderer::createTextObject() const
{
	return nullptr;
}

void gg::OpenGLRenderer::render()
{
	// begin

	IRenderer::invokeRenderCallback();

	// end
}

bool gg::OpenGLRenderer::drawTextObject(gg::ITextObject* text, int x, int y)
{
	return false;
}

bool gg::OpenGLRenderer::drawRectangle(int x, int y, int width, int height, gg::Color color)
{
	return false;
}
