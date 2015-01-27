/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_OPENGL_RENDERER_HPP_INCLUDED
#define GG_OPENGL_RENDERER_HPP_INCLUDED

#include <vector>
#include <windows.h>
#include <gl/GL.h>
#include "renderer/renderer.hpp"

namespace gg
{
	struct GLVec2
	{
		GLfloat x;
		GLfloat y;
	};

	class OpenGLTextObject : public ITextObject
	{
	public:
		OpenGLTextObject();
		virtual ~OpenGLTextObject();
		virtual bool setText(const std::string&);
		virtual bool setColor(Color);
		virtual bool setFont(const Font*);
		virtual unsigned getHeight() const;
		virtual IRenderer::Backend getBackend() const;

	private:
		friend class OpenGLRenderer;

		Color m_color;
		const Font* m_font;
		std::vector<GLVec2> m_verts;
		std::vector<GLVec2> m_uvs;
	};

	class OpenGLRenderer : public IRenderer
	{
	public:
		OpenGLRenderer(HWND hwnd);
		virtual ~OpenGLRenderer();
		virtual Backend getBackend() const;
		virtual bool getWindowDimensions(unsigned*, unsigned*) const;
		virtual WindowHandle getWindowHandle() const;
		virtual OpenGLTextObject* createTextObject() const;
		virtual void render();
		virtual bool drawTextObject(ITextObject*, int x, int y);
		virtual bool drawRectangle(int x, int y, int width, int height, Color color);

	private:
		struct FontTexturePair
		{
			const Font* font;
			GLuint texture_id;
		};

		GLuint getFontTextureID(const Font*);

		HWND m_hwnd;
		std::vector<FontTexturePair> m_font_textures;
		bool m_drawing;
	};
};

#endif // GG_OPENGL_RENDERER_HPP_INCLUDED
