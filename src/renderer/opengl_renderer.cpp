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
#include "stringutil.hpp"
#include "renderer/font.hpp"
#include "renderer/opengl_renderer.hpp"

#pragma comment(lib, "opengl32.lib")

gg::OpenGLTextObject::OpenGLTextObject() :
	m_color(0xff000000),
	m_font(gg::getNormalFont()),
	m_height(0)
{
}

gg::OpenGLTextObject::~OpenGLTextObject()
{
}

bool gg::OpenGLTextObject::setText(const std::string& text, unsigned line_spacing, const gg::Font* font)
{
	m_font = (font != nullptr) ? font : gg::getNormalFont();
	m_height = 0;
	m_vertices.clear();
	m_uv_coords.clear();

	std::vector<std::string> lines;
	gg::separate(text, lines, '\n');

	int x, x1, y, y1, i, len;
	float u0, v0, u1, v1;
	unsigned char ch;
	const unsigned char *ctext;

	for (unsigned line = 0; line < lines.size(); ++line)
	{
		x = 0;
		y = line * (m_font->getCharHeight() + line_spacing);
		y1 = y + m_font->getCharHeight();
		len = (int)lines[line].length();
		ctext = (const unsigned char *)(lines[line].c_str());

		for (i = 0; i < len; ++i)
		{
			ch = ctext[i];
			x1 = x + m_font->getCharWidth(ch);
			m_font->getUV(ch, &u0, &v0, &u1, &v1);

			m_vertices.push_back(GLVec2{ (GLfloat)x, (GLfloat)y });
			m_vertices.push_back(GLVec2{ (GLfloat)x1, (GLfloat)y });
			m_vertices.push_back(GLVec2{ (GLfloat)x, (GLfloat)y1 });
			m_vertices.push_back(GLVec2{ (GLfloat)x1, (GLfloat)y });
			m_vertices.push_back(GLVec2{ (GLfloat)x1, (GLfloat)y1 });
			m_vertices.push_back(GLVec2{ (GLfloat)x, (GLfloat)y1 });

			m_uv_coords.push_back(GLVec2{ u0, v0 });
			m_uv_coords.push_back(GLVec2{ u1, v0 });
			m_uv_coords.push_back(GLVec2{ u0, v1 });
			m_uv_coords.push_back(GLVec2{ u1, v0 });
			m_uv_coords.push_back(GLVec2{ u1, v1 });
			m_uv_coords.push_back(GLVec2{ u0, v1 });

			x = x1;
		}
	}

	m_height = y1;

	return true;
}

bool gg::OpenGLTextObject::setColor(gg::Color color)
{
	m_color = color;
	return true;
}

unsigned gg::OpenGLTextObject::getHeight() const
{
	return m_height;
}

gg::IRenderer::Backend gg::OpenGLTextObject::getBackend() const
{
	return IRenderer::Backend::OPENGL;
}


gg::OpenGLRenderer::OpenGLRenderer(HWND hwnd) :
	m_hwnd(hwnd),
	m_drawing(false)
{
	m_glUseProgram = (GLUSEPROGRAM)wglGetProcAddress("glUseProgram");
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
	//glPushAttrib(GL_ENABLE_BIT);

	if (m_glUseProgram) m_glUseProgram(0);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glDisable(GL_ALPHA_TEST);
	glDisable(GL_FOG);
	//glDisable(GL_LOGIC_OP);
	//glDisable(GL_SCISSOR_TEST);

	//glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_NORMAL_ARRAY);
	//glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	//glDisableClientState(GL_INDEX_ARRAY);
	//glDisableClientState(GL_COLOR_ARRAY);
	//glDisableClientState(GL_EDGE_FLAG_ARRAY);

	RECT client_rect;
	GetClientRect(m_hwnd, &client_rect);
	int width = client_rect.right;
	int height = client_rect.bottom;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, width, height);
	glOrtho(0.f, (GLfloat)width, (GLfloat)height, 0.f, -1.f, 1.f);

	/*glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glOrtho(viewport[0], viewport[0] + viewport[2], viewport[1] + viewport[3], viewport[1], -1.f, 1.f);*/

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	m_drawing = true;
	IRenderer::invokeRenderCallback();
	m_drawing = false;

	glFlush();

	//glPopAttrib();
}

bool gg::OpenGLRenderer::drawTextObject(const gg::ITextObject* itext, int x, int y, Color* color_ptr)
{
	if (!m_drawing)
		return false;

	if (itext->getBackend() != Backend::OPENGL)
		return false;

	const OpenGLTextObject* text = static_cast<const OpenGLTextObject*>(itext);

	if (text->m_vertices.size() < 4)
		return false;

	Color color = (color_ptr == nullptr) ? text->m_color : *color_ptr;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef((GLfloat)x, (GLfloat)y, 0.f);

	//glPushAttrib(GL_ENABLE_BIT);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, getFontTextureID(text->m_font));

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, &(text->m_vertices[0]));

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, &(text->m_uv_coords[0]));

	glDisableClientState(GL_COLOR_ARRAY);
	glColor4ub(GLubyte(color >> 16), GLubyte(color >> 8), GLubyte(color), GLubyte(color >> 24));

	glDrawArrays(GL_TRIANGLES, 0, (int)text->m_vertices.size());

	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	//glPopAttrib();

	return true;
}

bool gg::OpenGLRenderer::drawCaret(const gg::ITextObject* itext, int x, int y, int pos, gg::Color color)
{
	if (!m_drawing)
		return false;

	if (itext->getBackend() != Backend::OPENGL)
		return false;

	const OpenGLTextObject* text = static_cast<const OpenGLTextObject*>(itext);

	if (pos == 0)
		return drawRectangle(x, y, 3, text->m_font->getCharHeight(), color);

	int text_len = text->m_vertices.size() / 6; // 1 char has 6 vertices
	if (pos > text_len)
		return false;
	if (pos < 0)
		pos = text_len;

	GLVec2 v = text->m_vertices[((pos - 1) * 6) + 1];
	return drawRectangle(x + (int)v.x, y + (int)v.y, 3, text->m_font->getCharHeight(), color);
}

bool gg::OpenGLRenderer::drawLine(int x1, int y1, int x2, int y2, gg::Color color)
{
	if (!m_drawing)
		return false;

	//glPushAttrib(GL_ENABLE_BIT);

	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_LINES);
		glColor4ub(GLubyte(color >> 16), GLubyte(color >> 8), GLubyte(color), GLubyte(color >> 24));
		glVertex2f((GLfloat)x1, (GLfloat)y1);
		glVertex2f((GLfloat)x2, (GLfloat)y2);
	glEnd();

	//glPopAttrib();

	return true;
}

bool gg::OpenGLRenderer::drawRectangle(int x, int y, int width, int height, gg::Color color)
{
	if (!m_drawing)
		return false;

	//glPushAttrib(GL_ENABLE_BIT);

	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);
		glColor4ub(GLubyte(color >> 16), GLubyte(color >> 8), GLubyte(color), GLubyte(color >> 24));
		glVertex2f((GLfloat)x, (GLfloat)y);
		glVertex2f((GLfloat)(x + width), (GLfloat)y);
		glVertex2f((GLfloat)(x + width), (GLfloat)(y + height));
		glVertex2f((GLfloat)x, (GLfloat)(y + height));
	glEnd();

	//glPopAttrib();

	return true;
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

#endif // _WIN32
