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
#include "renderer/opengl_renderer.hpp"

#pragma comment(lib, "opengl32.lib")

gg::OpenGLTextObject::OpenGLTextObject() :
	m_color(0xffffffff),
	m_font(gg::getNormalFont())
{

}

gg::OpenGLTextObject::~OpenGLTextObject()
{
}

static void getlines(const std::string& str, std::vector<std::string>& lines)
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

bool gg::OpenGLTextObject::setText(const std::string& text)
{
	const unsigned sep_height = 2;

	m_verts.clear();
	m_uvs.clear();

	std::vector<std::string> lines;
	getlines(text, lines);

	int x, x1, y, y1, i, Len;
	float u0, v0, u1, v1;
	unsigned char ch;
	const unsigned char *ctext;

	for (unsigned line = 0; line < lines.size(); ++line)
	{
		x = 0;
		y = line * (m_font->getCharHeight() + sep_height);
		y1 = y + m_font->getCharHeight();
		Len = (int)lines[line].length();
		ctext = (const unsigned char *)(lines[line].c_str());

		for (i = 0; i<Len; ++i)
		{
			ch = ctext[i];
			x1 = x + m_font->getCharWidth(ch);
			m_font->getUV(ch, &u0, &v0, &u1, &v1);

			m_verts.push_back(GLVec2{ (GLfloat)x, (GLfloat)y });
			m_verts.push_back(GLVec2{ (GLfloat)x1, (GLfloat)y });
			m_verts.push_back(GLVec2{ (GLfloat)x, (GLfloat)y1 });
			m_verts.push_back(GLVec2{ (GLfloat)x1, (GLfloat)y });
			m_verts.push_back(GLVec2{ (GLfloat)x1, (GLfloat)y1 });
			m_verts.push_back(GLVec2{ (GLfloat)x, (GLfloat)y1 });

			m_uvs.push_back(GLVec2{ u0, v0 });
			m_uvs.push_back(GLVec2{ u1, v0 });
			m_uvs.push_back(GLVec2{ u0, v1 });
			m_uvs.push_back(GLVec2{ u1, v0 });
			m_uvs.push_back(GLVec2{ u1, v1 });
			m_uvs.push_back(GLVec2{ u0, v1 });

			x = x1;
		}
	}

	return true;
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
	m_hwnd(hwnd),
	m_drawing(false)
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
	return new OpenGLTextObject();
}

void gg::OpenGLRenderer::render()
{
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	RECT client_rect;
	GetClientRect(m_hwnd, &client_rect);
	GLfloat width = (GLfloat)client_rect.right - 1.f;
	GLfloat height = (GLfloat)client_rect.bottom - 1.f;
	glOrtho(0.f, width, height, 0.f, -1.f, 1.f);

	//glUseProgram(0);

	m_drawing = true;
	IRenderer::invokeRenderCallback();
	m_drawing = false;
}

bool gg::OpenGLRenderer::drawTextObject(gg::ITextObject* itext, int x, int y)
{
	if (itext->getBackend() != Backend::OPENGL)
		return false;

	OpenGLTextObject* text = static_cast<OpenGLTextObject*>(itext);

	if (text->m_verts.size() < 4)
		return false;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef((GLfloat)x, (GLfloat)y, 0.f);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, getFontTextureID(text->m_font));
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, &(text->m_verts[0]));
	glTexCoordPointer(2, GL_FLOAT, 0, &(text->m_uvs[0]));
	glColor4ub(GLubyte(text->m_color >> 16), GLubyte(text->m_color >> 8), GLubyte(text->m_color), GLubyte(text->m_color >> 24));
	glDrawArrays(GL_TRIANGLES, 0, (int)text->m_verts.size());

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	return true;
}

bool gg::OpenGLRenderer::drawRectangle(int x, int y, int width, int height, gg::Color color)
{
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);
		glColor4ub(GLubyte(color >> 16), GLubyte(color >> 8), GLubyte(color), GLubyte(color >> 24));
		glVertex2f((GLfloat)y, (GLfloat)y);
		glColor4ub(GLubyte(color >> 16), GLubyte(color >> 8), GLubyte(color), GLubyte(color >> 24));
		glVertex2f((GLfloat)(x + width), (GLfloat)y);
		glColor4ub(GLubyte(color >> 16), GLubyte(color >> 8), GLubyte(color), GLubyte(color >> 24));
		glVertex2f((GLfloat)(x + width), (GLfloat)(y + height));
		glColor4ub(GLubyte(color >> 16), GLubyte(color >> 8), GLubyte(color), GLubyte(color >> 24));
		glVertex2f((GLfloat)x, (GLfloat)(y + height));
	glEnd();

	return false;
}

static GLuint createGLTexture(const unsigned char* texture, unsigned width, unsigned height)
{
	GLuint texture_id = 0;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelTransferf(GL_ALPHA_SCALE, 1);
	glPixelTransferf(GL_ALPHA_BIAS, 0);
	glPixelTransferf(GL_RED_BIAS, 1);
	glPixelTransferf(GL_GREEN_BIAS, 1);
	glPixelTransferf(GL_BLUE_BIAS, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPixelTransferf(GL_ALPHA_BIAS, 0);
	glPixelTransferf(GL_RED_BIAS, 0);
	glPixelTransferf(GL_GREEN_BIAS, 0);
	glPixelTransferf(GL_BLUE_BIAS, 0);

	return texture_id;
}

GLuint gg::OpenGLRenderer::getFontTextureID(const Font* font)
{
	for (auto& it : m_font_textures)
	{
		if (it.font == font)
			return it.texture_id;
	}

	const unsigned char* texture;
	unsigned width;
	unsigned height;
	font->getTexture(&texture, &width, &height);
	GLuint texture_id = createGLTexture(texture, width, height);

	if (texture_id)
		m_font_textures.push_back({ font, texture_id });

	return texture_id;
}
