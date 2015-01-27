/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "renderer/opengl_renderer.hpp"

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

gg::ITextObject* gg::OpenGLRenderer::createTextObject() const
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
