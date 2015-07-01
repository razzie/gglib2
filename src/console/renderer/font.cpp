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

#include <cstring>
#include "renderer/font.hpp"

static int next_sq(int n)
{
	int r = 1;
	while (r < n)
		r *= 2;
	return r;
}

gg::Font* gg::Font::generate(const unsigned char* bitmap, unsigned width, unsigned height, bool D3D_align)
{
	const float scale = 1.f;

	// find height of the font
	int x, y;
	int h = 0, hh = 0;
	int r, rows = 0;
	for (y = 0; y < height; ++y)
	{
		if (bitmap[y*width] == 0)
		{
			if ((hh <= 0 && h <= 0) || (h != hh && h > 0 && hh > 0))
			{
				return nullptr; // bad font height
			}
			else if (h <= 0)
				h = hh;
			else if (hh <= 0)
				break;
			hh = 0;
			++rows;
		}
		else
			++hh;
	}

	// find width and position of each character
	int w = 0;
	int x0[224], y0[224], x1[224], y1[224];
	int ch = 32;
	int start;
	for (r = 0; r<rows; ++r)
	{
		start = 1;
		for (x = 1; x < width; ++x)
		{
			if (bitmap[(r*(h + 1) + h)*width + x] == 0 || x == width - 1)
			{
				if (x == start)
					break;  // next row
				if (ch < 256)
				{
					x0[ch - 32] = start;
					x1[ch - 32] = x;
					y0[ch - 32] = r*(h + 1);
					y1[ch - 32] = r*(h + 1) + h - 1;
					w += x - start + 1;
					start = x + 1;
				}
				++ch;
			}
		}
	}
	for (x = ch - 32; x<224; ++x)
	{
		x0[ch] = 0;
		x1[ch] = 0;
		y0[ch] = 0;
		y1[ch] = 0;
	}

	// Repack: build 14 rows of 16 characters.
	// - First, find the largest row
	int l, lmax = 1;
	for (r = 0; r<14; ++r)
	{
		l = 0;
		for (x = 0; x<16; ++x)
			l += x1[x + r * 16] - x0[x + r * 16] + 1;
		if (l>lmax)
			lmax = l;
	}
	// A little empty margin is added between chars to avoid artefact when antialiasing is on
	const int MARGIN_X = 2;
	const int MARGIN_Y = 2;
	lmax += 16 * MARGIN_X;
	// - Second, build the texture
	Font *font = new Font();
	font->m_char_num = ch - 32;
	font->m_char_height = (int)(scale*h + 0.5f);
	font->m_texture_width = next_sq(lmax);
	font->m_texture_height = next_sq(14 * (h + MARGIN_Y));
	font->m_texture = new unsigned char[font->m_texture_width*font->m_texture_height];
	std::memset(font->m_texture, 0, font->m_texture_width*font->m_texture_height);
	int xx;
	float du = 0.4f;
	float dv = 0.4f;
	if (!D3D_align)
	{
		du = 0;
		dv = 0;
	}
	else    // texel alignement for D3D
	{
		du = 0.5f;
		dv = 0.5f;
	}
	float alpha;
	for (r = 0; r < 14; ++r)
	{
		for (xx = 0, ch = r * 16; ch < (r + 1) * 16; ++ch)
		{
			if (y1[ch] - y0[ch] == h - 1)
			{
				for (y = 0; y < h; ++y)
				{
					for (x = x0[ch]; x <= x1[ch]; ++x)
					{
						alpha = ((float)(bitmap[x + (y0[ch] + y)*width])) / 256.0f;
						//alpha = alpha*sqrtf(alpha); // powf(alpha, 1.5f);   // some gamma correction
						font->m_texture[(xx + x - x0[ch]) + (r*(h + MARGIN_Y) + y)*font->m_texture_width] = (unsigned char)(alpha*256.0f);
					}
				}
				font->m_char_u0[ch + 32] = (float(xx) + du) / float(font->m_texture_width);
				xx += x1[ch] - x0[ch] + 1;
				font->m_char_u1[ch + 32] = (float(xx) + du) / float(font->m_texture_width);
				font->m_char_v0[ch + 32] = (float(r*(h + MARGIN_Y)) + dv) / float(font->m_texture_height);
				font->m_char_v1[ch + 32] = (float(r*(h + MARGIN_Y) + h) + dv) / float(font->m_texture_height);
				font->m_char_width[ch + 32] = (int)(scale*(x1[ch] - x0[ch] + 1) + 0.5f);
				xx += MARGIN_X;
			}
		}
	}

	const unsigned char Undef = 127; // default character used as for undifined ones (having ascii codes from 0 to 31)
	for (ch = 0; ch<32; ++ch)
	{
		font->m_char_u0[ch] = font->m_char_u0[Undef];
		font->m_char_u1[ch] = font->m_char_u1[Undef];
		font->m_char_v0[ch] = font->m_char_v0[Undef];
		font->m_char_v1[ch] = font->m_char_v1[Undef];
		font->m_char_width[ch] = font->m_char_width[Undef] / 2;
	}

	return font;
}

gg::Font::Font() :
	m_texture_width(0),
	m_texture_height(0),
	m_texture(nullptr),
	m_char_num(0),
	m_char_height(0)
{
	for (unsigned i = 0; i < 256; ++i)
	{
		m_char_u0[i] = 0;
		m_char_u1[i] = 0;
		m_char_v0[i] = 0;
		m_char_v1[i] = 0;
		m_char_width[i] = 0;
	}
}

gg::Font::~Font()
{
	if (m_texture != nullptr)
		delete[] m_texture;
	m_texture = nullptr;
	m_texture_width = 0;
	m_texture_height = 0;
	m_char_num = 0;
}

unsigned gg::Font::getCharWidth(unsigned n) const
{
	return m_char_width[n];
}

unsigned gg::Font::getCharHeight() const
{
	return m_char_height;
}

bool gg::Font::getUV(unsigned n, float* u0, float* v0, float* u1, float* v1) const
{
	*u0 = m_char_u0[n];
	*v0 = m_char_v0[n];
	*u1 = m_char_u1[n];
	*v1 = m_char_v1[n];
	return false;
}

bool gg::Font::getTexture(const unsigned char** texture, unsigned* width, unsigned* height) const
{
	*texture = m_texture;
	*width = m_texture_width;
	*height = m_texture_height;
	return true;
}
