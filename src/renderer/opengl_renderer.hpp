/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_OPENGL_RENDERER_HPP_INCLUDED
#define GG_OPENGL_RENDERER_HPP_INCLUDED

#include <windows.h>
#include "renderer/renderer.hpp"

namespace gg
{
	class OpenGLRenderer : public IRenderer
	{
	public:
		OpenGLRenderer(HWND hwnd);
		virtual ~OpenGLRenderer();
		virtual Backend getBackend() const;
		virtual bool getWindowDimensions(unsigned*, unsigned*) const;
		virtual WindowHandle getWindowHandle() const;
		virtual ITextObject* createTextObject() const;
		virtual void render();
		virtual bool drawTextObject(ITextObject*, int x, int y);
		virtual bool drawRectangle(int x, int y, int width, int height, Color color);

	private:
		HWND m_hwnd;
	};
};

#endif // GG_OPENGL_RENDERER_HPP_INCLUDED
