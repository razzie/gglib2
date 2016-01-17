/**
 * Copyright (c) 2014-2016 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

/**
 * Original code borrowed from AntTweakBar library under zlib/libpng license.
 */

#ifdef _WIN32
#include <cstring>
#include "stringutil.hpp"
#include "renderer/font.hpp"
#include "renderer/D3D9_renderer.hpp"

static const float identity_matrix[4][4] { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };


gg::D3D9Renderer::D3D9Renderer(HWND hwnd, IDirect3DDevice9* device) :
	m_hwnd(hwnd),
	m_device(device),
	m_stateblock(nullptr),
	m_drawing(false)
{
	std::memset(&m_caps, 0, sizeof(D3DCAPS9));
	m_device->GetDeviceCaps(&m_caps);

	D3DDEVICE_CREATION_PARAMETERS cp;
	m_device->GetCreationParameters(&cp);
	m_puredevice = (cp.BehaviorFlags & D3DCREATE_PUREDEVICE) ? true : false;
}

gg::D3D9Renderer::~D3D9Renderer()
{
	reset();
}

gg::IRenderer::Backend gg::D3D9Renderer::getBackend() const
{
	return Backend::DIRECT3D9;
}

void* gg::D3D9Renderer::getBackendHandle() const
{
	return (void*)m_device;
}

bool gg::D3D9Renderer::getWindowDimensions(unsigned* width, unsigned* height) const
{
	RECT client_rect;
	GetClientRect(m_hwnd, &client_rect);
	*width = client_rect.right;
	*height = client_rect.bottom;
	return true;
}

gg::WindowHandle gg::D3D9Renderer::getWindowHandle() const
{
	return (WindowHandle)m_hwnd;
}

gg::D3D9TextObject* gg::D3D9Renderer::createTextObject() const
{
	return new D3D9TextObject();
}

void gg::D3D9Renderer::render()
{
	if (!m_puredevice)
	{
		if (m_stateblock == nullptr)
			m_device->CreateStateBlock(D3DSBT_ALL, &m_stateblock);
		else
			m_stateblock->Capture();
	}

	unsigned width, height;
	getWindowDimensions(&width, &height);

	D3DVIEWPORT9 vp;
	vp.X = 0;
	vp.Y = 0;
	vp.Width = width;
	vp.Height = height;
	vp.MinZ = 0.f;
	vp.MaxZ = 1.f;
	m_device->SetViewport(&vp);

	D3DMATRIX view;
	std::memcpy(&view, &identity_matrix, sizeof(D3DMATRIX));
	view.m[3][0] = -(float)(width / 2);
	view.m[3][1] = -(float)(height / 2);
	m_device->SetTransform(D3DTS_VIEW, &view);

	D3DMATRIX proj;
	std::memcpy(&proj, &identity_matrix, sizeof(D3DMATRIX));
	proj.m[0][0] = 2.f / (float)width;
	proj.m[1][1] = -2.f / (float)height;
	m_device->SetTransform(D3DTS_PROJECTION, &proj);

	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
	m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	m_device->SetRenderState(D3DRS_LASTPIXEL, FALSE);
	m_device->SetRenderState(D3DRS_FOGENABLE, FALSE);
	m_device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	m_device->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0000000F);
	//m_device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);
	m_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	if (m_caps.PrimitiveMiscCaps & D3DPMISCCAPS_SEPARATEALPHABLEND)
		m_device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
	//if( m_State->m_Caps.LineCaps & D3DLINECAPS_ANTIALIAS )
	m_device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);

	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CONSTANT);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU);
	m_device->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	m_device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	m_device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	m_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	m_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	m_device->SetVertexShader(NULL);
	m_device->SetPixelShader(NULL);

	m_drawing = true;
	IRenderer::invokeRenderCallback();
	m_drawing = false;

	if (!m_puredevice)
	{
		m_stateblock->Apply();
	}
}

void gg::D3D9Renderer::reset()
{
	if (m_stateblock != nullptr)
	{
		m_stateblock->Release();
		m_stateblock = nullptr;
	}

	for (auto& it : m_font_textures)
		it.texture->Release();
	m_font_textures.clear();
}

bool gg::D3D9Renderer::drawTextObject(const gg::ITextObject* itext, int x, int y, Color* color_ptr)
{
	if (!m_drawing)
		return false;

	if (itext->getBackend() != Backend::DIRECT3D9)
		return false;

	const D3D9TextObject* text = static_cast<const D3D9TextObject*>(itext);

	if (text->m_vertices.size() < 4)
		return false;

	D3DMATRIX model;
	std::memcpy(&model, identity_matrix, sizeof(D3DMATRIX));
	model.m[3][0] = (float)x;
	model.m[3][1] = (float)y;
	m_device->SetTransform(D3DTS_WORLD, &model);

	DWORD color = (DWORD)((color_ptr == nullptr) ? text->m_color : *color_ptr);
	m_device->SetTexture(0, getFontTexture(text->m_font));
	m_device->SetTextureStageState(0, D3DTSS_CONSTANT, color);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	m_device->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0));
	m_device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, text->m_vertices.size() / 3, &(text->m_vertices[0]), sizeof(D3D9TextObject::Vertex));

	return true;
}

bool gg::D3D9Renderer::drawCaret(const gg::ITextObject* itext, int x, int y, int pos, gg::Color color)
{
	if (!m_drawing)
		return false;

	if (itext->getBackend() != Backend::DIRECT3D9)
		return false;

	const D3D9TextObject* text = static_cast<const D3D9TextObject*>(itext);

	if (pos == 0)
		return drawRectangle(x, y, 3, text->m_font->getCharHeight(), color);

	int text_len = text->m_vertices.size() / 6; // 1 char has 6 vertices
	if (pos > text_len)
		return false;
	if (pos < 0)
		pos = text_len;

	auto v = text->m_vertices[((pos - 1) * 6) + 1];
	return drawRectangle(x + (int)v.pos[0], y + (int)v.pos[1], 3, text->m_font->getCharHeight(), color);
}

bool gg::D3D9Renderer::drawLine(int x1, int y1, int x2, int y2, gg::Color color)
{
	struct Vertex
	{
		float pos[4];
	};

	if (!m_drawing)
		return false;

	Vertex p[2];

	p[0].pos[0] = (float)x1;
	p[0].pos[1] = (float)y1;
	p[0].pos[2] = 0.f;
	p[0].pos[3] = 1.f;

	p[1].pos[0] = (float)x2;
	p[1].pos[1] = (float)y2;
	p[1].pos[2] = 0.f;
	p[1].pos[3] = 1.f;

	m_device->SetTextureStageState(0, D3DTSS_CONSTANT, color);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	m_device->SetFVF(D3DFVF_XYZRHW);
	m_device->DrawPrimitiveUP(D3DPT_LINELIST, 1, p, sizeof(Vertex));

	return true;
}

bool gg::D3D9Renderer::drawRectangle(int x, int y, int width, int height, gg::Color color)
{
	struct Vertex
	{
		float pos[4];
	};

	if (!m_drawing)
		return false;

	Vertex p[4];

	p[0].pos[0] = (float)(x + width);
	p[0].pos[1] = (float)(y);
	p[0].pos[2] = 0.f;
	p[0].pos[3] = 1.f;

	p[1].pos[0] = (float)(x);
	p[1].pos[1] = (float)(y);
	p[1].pos[2] = 0.f;
	p[1].pos[3] = 1.f;

	p[2].pos[0] = (float)(x + width);
	p[2].pos[1] = (float)(y + height);
	p[2].pos[2] = 0.f;
	p[2].pos[3] = 1.f;

	p[3].pos[0] = (float)(x);
	p[3].pos[1] = (float)(y + height);
	p[3].pos[2] = 0.f;
	p[3].pos[3] = 1.f;

	m_device->SetTextureStageState(0, D3DTSS_CONSTANT, color);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	m_device->SetFVF(D3DFVF_XYZRHW);
	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, p, sizeof(Vertex));

	return true;
}

static IDirect3DTexture9* createD3D9Texture(IDirect3DDevice9* device, const unsigned char* bitmap, unsigned width, unsigned height)
{
	IDirect3DTexture9* texture = nullptr;
	IDirect3DDevice9Ex* device_ex = nullptr;
	HRESULT hr;

	hr = device->QueryInterface(__uuidof(IDirect3DDevice9Ex), (void **)&device_ex);
	if (SUCCEEDED(hr) && device_ex)
	{
		hr = device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL);
		device_ex->Release();
	}
	else
	{
		hr = device->CreateTexture(width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture, NULL);
	}

	if (FAILED(hr))
		return nullptr;

	D3DLOCKED_RECT r;
	hr = texture->LockRect(0, &r, NULL, 0);
	if (SUCCEEDED(hr))
	{
		gg::Color *p = static_cast<gg::Color*>(r.pBits);
		for (unsigned i = 0; i < (width * height); ++i, ++p)
			*p = 0x00ffffff | (((gg::Color)(bitmap[i])) << 24);
		texture->UnlockRect(0);
	}

	return texture;
}

IDirect3DTexture9* gg::D3D9Renderer::getFontTexture(const Font* font)
{
	for (auto& it : m_font_textures)
	{
		if (it.font == font)
			return it.texture;
	}

	const unsigned char* bitmap;
	unsigned width;
	unsigned height;
	font->getTexture(&bitmap, &width, &height);
	IDirect3DTexture9* texture = createD3D9Texture(m_device, bitmap, width, height);

	if (texture)
		m_font_textures.push_back({ font, texture });

	return texture;
}


gg::D3D9TextObject::D3D9TextObject() :
m_color(0xffffffff),
m_font(gg::getNormalFont()),
m_height(0)
{
}

gg::D3D9TextObject::~D3D9TextObject()
{
}

bool gg::D3D9TextObject::setText(const std::string& text, unsigned line_spacing, const gg::Font* font)
{
	m_font = (font != nullptr) ? font : gg::getNormalFont();
	m_height = 0;
	m_vertices.clear();

	std::vector<std::string> lines;
	gg::separate(text, lines, '\n');

	int x, x1, y, y1, i, len;
	float u0, v0, u1, v1;
	unsigned char ch;
	const unsigned char *ctext;

	D3D9TextObject::Vertex vtx;
	vtx.pos[2] = 0;

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

			vtx.pos[0] = (float)x + 0.5f;
			vtx.pos[1] = (float)y + 0.5f;
			vtx.uv[0] = u0;
			vtx.uv[1] = v0;
			m_vertices.push_back(vtx);

			vtx.pos[0] = (float)x1 + 0.5f;
			vtx.pos[1] = (float)y + 0.5f;
			vtx.uv[0] = u1;
			vtx.uv[1] = v0;
			m_vertices.push_back(vtx);

			vtx.pos[0] = (float)x + 0.5f;
			vtx.pos[1] = (float)y1 + 0.5f;
			vtx.uv[0] = u0;
			vtx.uv[1] = v1;
			m_vertices.push_back(vtx);

			vtx.pos[0] = (float)x1 + 0.5f;
			vtx.pos[1] = (float)y + 0.5f;
			vtx.uv[0] = u1;
			vtx.uv[1] = v0;
			m_vertices.push_back(vtx);

			vtx.pos[0] = (float)x1 + 0.5f;
			vtx.pos[1] = (float)y1 + 0.5f;
			vtx.uv[0] = u1;
			vtx.uv[1] = v1;
			m_vertices.push_back(vtx);

			vtx.pos[0] = (float)x + 0.5f;
			vtx.pos[1] = (float)y1 + 0.5f;
			vtx.uv[0] = u0;
			vtx.uv[1] = v1;
			m_vertices.push_back(vtx);

			x = x1;
		}
	}

	m_height = y1;

	return true;
}

bool gg::D3D9TextObject::setColor(gg::Color color)
{
	m_color = color;
	return true;
}

unsigned gg::D3D9TextObject::getHeight() const
{
	return m_height;
}

gg::IRenderer::Backend gg::D3D9TextObject::getBackend() const
{
	return IRenderer::Backend::DIRECT3D9;
}

#endif // _WIN32
