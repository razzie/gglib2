/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_OPENGL_RENDERER_HPP_INCLUDED
#define GG_OPENGL_RENDERER_HPP_INCLUDED
#ifdef _WIN32

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
		virtual bool setText(const std::string&, unsigned line_spacing = 2, const Font* = nullptr);
		virtual bool setColor(Color);
		virtual unsigned getHeight() const;
		virtual IRenderer::Backend getBackend() const;

	private:
		friend class OpenGLRenderer;

		Color m_color;
		const Font* m_font;
		unsigned m_height;
		std::vector<GLVec2> m_vertices;
		std::vector<GLVec2> m_uv_coords;
	};

	class OpenGLRenderer : public IRenderer
	{
	public:
		OpenGLRenderer(HWND hwnd, HGLRC hglrc);
		virtual ~OpenGLRenderer();
		virtual Backend getBackend() const;
		virtual void* getBackendHandle() const;
		virtual bool getWindowDimensions(unsigned*, unsigned*) const;
		virtual WindowHandle getWindowHandle() const;
		virtual OpenGLTextObject* createTextObject() const;
		virtual void render();
		virtual bool drawTextObject(const ITextObject*, int x, int y, Color* = nullptr);
		virtual bool drawCaret(const ITextObject*, int x, int y, int pos, Color);
		virtual bool drawLine(int x1, int y1, int x2, int y2, Color color);
		virtual bool drawRectangle(int x, int y, int width, int height, Color color);

	private:
		struct FontTexturePair
		{
			const Font* font;
			GLuint texture_id;
		};

		// methods
		GLuint getFontTextureID(const Font*);

		// member variables
		HWND m_hwnd;
		HGLRC m_hglrc;
		std::vector<FontTexturePair> m_font_textures;
		bool m_drawing;

		// GL functions
		typedef void(*GLUSEPROGRAM)(GLuint);
		GLUSEPROGRAM m_glUseProgram;
	};
};

#endif // _WIN32
#endif // GG_OPENGL_RENDERER_HPP_INCLUDED
