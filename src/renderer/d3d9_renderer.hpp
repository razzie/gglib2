/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_D3D9_RENDERER_HPP_INCLUDED
#define GG_D3D9_RENDERER_HPP_INCLUDED
#ifdef _WIN32

#include <vector>
#include <windows.h>
#include <d3d9.h>
#include "renderer/renderer.hpp"

namespace gg
{
	class D3D9TextObject : public ITextObject
	{
	public:
		D3D9TextObject();
		virtual ~D3D9TextObject();
		virtual bool setText(const std::string&, unsigned line_spacing = 2, const Font* = nullptr);
		virtual bool setColor(Color);
		virtual unsigned getHeight() const;
		virtual IRenderer::Backend getBackend() const;

	private:
		struct Vertex
		{
			float pos[3];
			float uv[2];
		};

		friend class D3D9Renderer;

		Color m_color;
		const Font* m_font;
		unsigned m_height;
		std::vector<Vertex> m_vertices;
	};

	class D3D9Renderer : public IRenderer
	{
	public:
		D3D9Renderer(HWND hwnd, IDirect3DDevice9*);
		virtual ~D3D9Renderer();
		virtual Backend getBackend() const;
		virtual void* getBackendHandle() const;
		virtual bool getWindowDimensions(unsigned*, unsigned*) const;
		virtual WindowHandle getWindowHandle() const;
		virtual D3D9TextObject* createTextObject() const;
		virtual void render();
		virtual void reset();
		virtual bool drawTextObject(const ITextObject*, int x, int y, Color* = nullptr);
		virtual bool drawCaret(const ITextObject*, int x, int y, int pos, Color);
		virtual bool drawLine(int x1, int y1, int x2, int y2, Color color);
		virtual bool drawRectangle(int x, int y, int width, int height, Color color);

	private:
		struct FontTexturePair
		{
			const Font* font;
			IDirect3DTexture9* texture;
		};

		IDirect3DTexture9* getFontTexture(const Font*);

		HWND m_hwnd;
		IDirect3DDevice9* m_device;
		D3DCAPS9 m_caps;
		IDirect3DStateBlock9* m_stateblock;
		std::vector<FontTexturePair> m_font_textures;
		bool m_puredevice;
		bool m_drawing;
	};
};

#endif // _WIN32
#endif // GG_D3D9_RENDERER_HPP_INCLUDED
