/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_FONT_HPP_INCLUDED
#define GG_FONT_HPP_INCLUDED

namespace gg
{
	class Font
	{
	public:
		static Font* generate(const unsigned char* tex, unsigned w, unsigned h, bool D3D_align = false);

		Font();
		~Font();
		unsigned getCharWidth(unsigned n) const;
		unsigned getCharHeight() const;
		bool getUV(unsigned n, float* u0, float* v0, float* u1, float* v1) const;
		bool getTexture(const unsigned char** tex, unsigned* w, unsigned* h) const;

	private:
		unsigned char* m_texture;
		unsigned m_texture_width;
		unsigned m_texture_height;
		float m_char_u0[256];
		float m_char_v0[256];
		float m_char_u1[256];
		float m_char_v1[256];
		unsigned m_char_width[256];
		unsigned m_char_height;
		unsigned m_char_num;
	};

	const Font* getSmallFont();
	const Font* getNormalFont();
	const Font* getLargeFont();
};

#endif // GG_FONT_HPP_INCLUDED
