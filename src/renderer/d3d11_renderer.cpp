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
#include <cstring>
#include "stringutil.hpp"
#include "renderer/font.hpp"
#include "renderer/D3D11_renderer.hpp"

static const float identity_matrix[4][4] { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };

namespace gg
{
	class D3D11StateBlock
	{
	public:
		ID3D11ComputeShader *   m_CSShader;
		ID3D11ClassInstance **  m_CSClassInstances;
		UINT                    m_CSNumClassInstances;
		ID3D11DomainShader *    m_DSShader;
		ID3D11ClassInstance **  m_DSClassInstances;
		UINT                    m_DSNumClassInstances;
		ID3D11GeometryShader *  m_GSShader;
		ID3D11ClassInstance **  m_GSClassInstances;
		UINT                    m_GSNumClassInstances;
		ID3D11HullShader *      m_HSShader;
		ID3D11ClassInstance **  m_HSClassInstances;
		UINT                    m_HSNumClassInstances;
		ID3D11PixelShader *     m_PSShader;
		ID3D11ClassInstance **  m_PSClassInstances;
		UINT                    m_PSNumClassInstances;
		ID3D11Buffer *          m_PSConstantBuffer; // backup the first constant buffer only
		ID3D11SamplerState *    m_PSSampler; // backup the first sampler only
		ID3D11ShaderResourceView*m_PSShaderResourceView; // backup the first shader resource only
		ID3D11VertexShader *    m_VSShader;
		ID3D11ClassInstance **  m_VSClassInstances;
		UINT                    m_VSNumClassInstances;
		ID3D11Buffer *          m_VSConstantBuffer; // backup the first constant buffer only

		ID3D11Buffer *          m_IAIndexBuffer;
		DXGI_FORMAT             m_IAIndexBufferFormat;
		UINT                    m_IAIndexBufferOffset;
		ID3D11InputLayout *     m_IAInputLayout;
		D3D11_PRIMITIVE_TOPOLOGY m_IATopology;
		ID3D11Buffer *          m_IAVertexBuffer; // backup the first buffer only
		UINT                    m_IAVertexBufferStride;
		UINT                    m_IAVertexBufferOffset;

		ID3D11BlendState *      m_OMBlendState;
		FLOAT                   m_OMBlendFactor[4];
		UINT                    m_OMSampleMask;
		ID3D11DepthStencilState*m_OMDepthStencilState;
		UINT                    m_OMStencilRef;

		UINT                    m_RSScissorNumRects;
		D3D11_RECT              m_RSScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		ID3D11RasterizerState * m_RSRasterizerState;
		UINT                    m_RSNumViewports;
		D3D11_VIEWPORT          m_RSViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

		D3D11StateBlock(ID3D11Device *, ID3D11DeviceContext *);
		~D3D11StateBlock();
		void                    Capture();
		void                    Apply();
		void                    Release();

	private:
		ID3D11Device *          m_device;
		ID3D11DeviceContext *   m_context;
	};
};

gg::D3D11StateBlock::D3D11StateBlock(ID3D11Device *device, ID3D11DeviceContext *context)
{
	ZeroMemory(this, sizeof(D3D11StateBlock));
	m_device = device;
	m_context = context;
}

gg::D3D11StateBlock::~D3D11StateBlock()
{
	Release();
	m_device = NULL;
	m_context = NULL;
}

void gg::D3D11StateBlock::Capture()
{
	// Release previous state if needed
	Release();

	// Save shaders.
	// Not sure how xxGetShader works, D3D11 doc is evasive... Attempt:
	// First call GetShader with NULL ClassInstances to get the number of class instances.
	// Second, if not zero allocate an array of class instances and call GetShader again
	// with this array ptr to get the class instances and release the shader since its
	// ref count has been incremented a second time.

	m_CSShader = NULL;
	m_CSClassInstances = NULL;
	m_CSNumClassInstances = 0;
	m_context->CSGetShader(&m_CSShader, NULL, &m_CSNumClassInstances);
	if (m_CSNumClassInstances > 0)
	{
		m_CSClassInstances = new ID3D11ClassInstance*[m_CSNumClassInstances];
		for (UINT i = 0; i < m_CSNumClassInstances; i++)
			m_CSClassInstances[i] = NULL;
		m_context->CSGetShader(&m_CSShader, m_CSClassInstances, &m_CSNumClassInstances);
		if (m_CSShader != NULL)
			m_CSShader->Release();
	}

	m_DSShader = NULL;
	m_DSClassInstances = NULL;
	m_DSNumClassInstances = 0;
	m_context->DSGetShader(&m_DSShader, NULL, &m_DSNumClassInstances);
	if (m_DSNumClassInstances > 0)
	{
		m_DSClassInstances = new ID3D11ClassInstance*[m_DSNumClassInstances];
		for (UINT i = 0; i < m_DSNumClassInstances; i++)
			m_DSClassInstances[i] = NULL;
		m_context->DSGetShader(&m_DSShader, m_DSClassInstances, &m_DSNumClassInstances);
		if (m_DSShader != NULL)
			m_DSShader->Release();
	}

	m_GSShader = NULL;
	m_GSClassInstances = NULL;
	m_GSNumClassInstances = 0;
	m_context->GSGetShader(&m_GSShader, NULL, &m_GSNumClassInstances);
	if (m_GSNumClassInstances > 0)
	{
		m_GSClassInstances = new ID3D11ClassInstance*[m_GSNumClassInstances];
		for (UINT i = 0; i < m_GSNumClassInstances; i++)
			m_GSClassInstances[i] = NULL;
		m_context->GSGetShader(&m_GSShader, m_GSClassInstances, &m_GSNumClassInstances);
		if (m_GSShader != NULL)
			m_GSShader->Release();
	}

	m_HSShader = NULL;
	m_HSClassInstances = NULL;
	m_HSNumClassInstances = 0;
	m_context->HSGetShader(&m_HSShader, NULL, &m_HSNumClassInstances);
	if (m_HSNumClassInstances > 0)
	{
		m_HSClassInstances = new ID3D11ClassInstance*[m_HSNumClassInstances];
		for (UINT i = 0; i < m_HSNumClassInstances; i++)
			m_HSClassInstances[i] = NULL;
		m_context->HSGetShader(&m_HSShader, m_HSClassInstances, &m_HSNumClassInstances);
		if (m_HSShader != NULL)
			m_HSShader->Release();
	}

	m_PSShader = NULL;
	m_PSClassInstances = NULL;
	m_PSNumClassInstances = 0;
	m_context->PSGetShader(&m_PSShader, NULL, &m_PSNumClassInstances);
	if (m_PSNumClassInstances > 0)
	{
		m_PSClassInstances = new ID3D11ClassInstance*[m_PSNumClassInstances];
		for (UINT i = 0; i < m_PSNumClassInstances; i++)
			m_PSClassInstances[i] = NULL;
		m_context->PSGetShader(&m_PSShader, m_PSClassInstances, &m_PSNumClassInstances);
		if (m_PSShader != NULL)
			m_PSShader->Release();
	}
	m_context->PSGetConstantBuffers(0, 1, &m_PSConstantBuffer);
	m_context->PSGetSamplers(0, 1, &m_PSSampler);
	m_context->PSGetShaderResources(0, 1, &m_PSShaderResourceView);

	m_VSShader = NULL;
	m_VSClassInstances = NULL;
	m_VSNumClassInstances = 0;
	m_context->VSGetShader(&m_VSShader, NULL, &m_VSNumClassInstances);
	if (m_VSNumClassInstances > 0)
	{
		m_VSClassInstances = new ID3D11ClassInstance*[m_VSNumClassInstances];
		for (UINT i = 0; i < m_VSNumClassInstances; i++)
			m_VSClassInstances[i] = NULL;
		m_context->VSGetShader(&m_VSShader, m_VSClassInstances, &m_VSNumClassInstances);
		if (m_VSShader != NULL)
			m_VSShader->Release();
	}
	m_context->VSGetConstantBuffers(0, 1, &m_VSConstantBuffer);

	// Save Input-Assembler states
	m_context->IAGetIndexBuffer(&m_IAIndexBuffer, &m_IAIndexBufferFormat, &m_IAIndexBufferOffset);
	m_context->IAGetInputLayout(&m_IAInputLayout);
	m_context->IAGetPrimitiveTopology(&m_IATopology);
	m_context->IAGetVertexBuffers(0, 1, &m_IAVertexBuffer, &m_IAVertexBufferStride, &m_IAVertexBufferOffset);

	// Save Ouput-Merger states
	m_context->OMGetBlendState(&m_OMBlendState, m_OMBlendFactor, &m_OMSampleMask);
	m_context->OMGetDepthStencilState(&m_OMDepthStencilState, &m_OMStencilRef);

	// Save Rasterizer states
	m_context->RSGetScissorRects(&m_RSScissorNumRects, NULL);
	if (m_RSScissorNumRects > 0)
		m_context->RSGetScissorRects(&m_RSScissorNumRects, m_RSScissorRects);
	m_context->RSGetViewports(&m_RSNumViewports, NULL);
	if (m_RSNumViewports > 0)
		m_context->RSGetViewports(&m_RSNumViewports, m_RSViewports);
	m_context->RSGetState(&m_RSRasterizerState);
}

void gg::D3D11StateBlock::Apply()
{
	// Restore shaders
	m_context->CSSetShader(m_CSShader, m_CSClassInstances, m_CSNumClassInstances);
	m_context->DSSetShader(m_DSShader, m_DSClassInstances, m_DSNumClassInstances);
	m_context->GSSetShader(m_GSShader, m_GSClassInstances, m_GSNumClassInstances);
	m_context->HSSetShader(m_HSShader, m_HSClassInstances, m_HSNumClassInstances);
	m_context->PSSetShader(m_PSShader, m_PSClassInstances, m_PSNumClassInstances);
	m_context->PSSetConstantBuffers(0, 1, &m_PSConstantBuffer);
	m_context->PSSetSamplers(0, 1, &m_PSSampler);
	m_context->PSSetShaderResources(0, 1, &m_PSShaderResourceView);
	m_context->VSSetShader(m_VSShader, m_VSClassInstances, m_VSNumClassInstances);
	m_context->VSSetConstantBuffers(0, 1, &m_VSConstantBuffer);

	// Restore Input-Assembler
	m_context->IASetIndexBuffer(m_IAIndexBuffer, m_IAIndexBufferFormat, m_IAIndexBufferOffset);
	m_context->IASetInputLayout(m_IAInputLayout);
	m_context->IASetPrimitiveTopology(m_IATopology);
	m_context->IASetVertexBuffers(0, 1, &m_IAVertexBuffer, &m_IAVertexBufferStride, &m_IAVertexBufferOffset);

	// Restore Ouput-Merger
	m_context->OMSetBlendState(m_OMBlendState, m_OMBlendFactor, m_OMSampleMask);
	m_context->OMSetDepthStencilState(m_OMDepthStencilState, m_OMStencilRef);

	// Restore Rasterizer states
	m_context->RSSetScissorRects(m_RSScissorNumRects, m_RSScissorRects);
	m_context->RSSetViewports(m_RSNumViewports, m_RSViewports);
	m_context->RSSetState(m_RSRasterizerState);
}

void gg::D3D11StateBlock::Release()
{
	// Release stored shaders

	if (m_CSClassInstances != NULL)
	{
		for (UINT i = 0; i < m_CSNumClassInstances; i++)
			if (m_CSClassInstances[i] != NULL)
				m_CSClassInstances[i]->Release();
		delete[] m_CSClassInstances;
		m_CSClassInstances = NULL;
		m_CSNumClassInstances = 0;
	}
	if (m_CSShader != NULL)
	{
		m_CSShader->Release();
		m_CSShader = NULL;
	}

	if (m_DSClassInstances != NULL)
	{
		for (UINT i = 0; i < m_DSNumClassInstances; i++)
			if (m_DSClassInstances[i] != NULL)
				m_DSClassInstances[i]->Release();
		delete[] m_DSClassInstances;
		m_DSClassInstances = NULL;
		m_DSNumClassInstances = 0;
	}
	if (m_DSShader != NULL)
	{
		m_DSShader->Release();
		m_DSShader = NULL;
	}

	if (m_GSClassInstances != NULL)
	{
		for (UINT i = 0; i < m_GSNumClassInstances; i++)
			if (m_GSClassInstances[i] != NULL)
				m_GSClassInstances[i]->Release();
		delete[] m_GSClassInstances;
		m_GSClassInstances = NULL;
		m_GSNumClassInstances = 0;
	}
	if (m_GSShader != NULL)
	{
		m_GSShader->Release();
		m_GSShader = NULL;
	}

	if (m_HSClassInstances != NULL)
	{
		for (UINT i = 0; i < m_HSNumClassInstances; i++)
			if (m_HSClassInstances[i] != NULL)
				m_HSClassInstances[i]->Release();
		delete[] m_HSClassInstances;
		m_HSClassInstances = NULL;
		m_HSNumClassInstances = 0;
	}
	if (m_HSShader != NULL)
	{
		m_HSShader->Release();
		m_HSShader = NULL;
	}

	if (m_PSClassInstances != NULL)
	{
		for (UINT i = 0; i < m_PSNumClassInstances; i++)
			if (m_PSClassInstances[i] != NULL)
				m_PSClassInstances[i]->Release();
		delete[] m_PSClassInstances;
		m_PSClassInstances = NULL;
		m_PSNumClassInstances = 0;
	}
	if (m_PSShader != NULL)
	{
		m_PSShader->Release();
		m_PSShader = NULL;
	}
	if (m_PSConstantBuffer != NULL)
	{
		m_PSConstantBuffer->Release();
		m_PSConstantBuffer = NULL;
	}
	if (m_PSSampler != NULL)
	{
		m_PSSampler->Release();
		m_PSSampler = NULL;
	}
	if (m_PSShaderResourceView != NULL)
	{
		m_PSShaderResourceView->Release();
		m_PSShaderResourceView = NULL;
	}

	if (m_VSClassInstances != NULL)
	{
		for (UINT i = 0; i < m_VSNumClassInstances; i++)
			if (m_VSClassInstances[i] != NULL)
				m_VSClassInstances[i]->Release();
		delete[] m_VSClassInstances;
		m_VSClassInstances = NULL;
		m_VSNumClassInstances = 0;
	}
	if (m_VSShader != NULL)
	{
		m_VSShader->Release();
		m_VSShader = NULL;
	}
	if (m_VSConstantBuffer != NULL)
	{
		m_VSConstantBuffer->Release();
		m_VSConstantBuffer = NULL;
	}

	// Release Input-Assembler states
	if (m_IAIndexBuffer != NULL)
	{
		m_IAIndexBuffer->Release();
		m_IAIndexBuffer = NULL;
	}
	if (m_IAInputLayout != NULL)
	{
		m_IAInputLayout->Release();
		m_IAInputLayout = 0;
	}
	if (m_IAVertexBuffer != NULL)
	{
		m_IAVertexBuffer->Release();
		m_IAVertexBuffer = NULL;
	}

	// Release Output-Merger states
	if (m_OMBlendState != NULL)
	{
		m_OMBlendState->Release();
		m_OMBlendState = NULL;
	}
	if (m_OMDepthStencilState != NULL)
	{
		m_OMDepthStencilState->Release();
		m_OMDepthStencilState = NULL;
	}

	// Release Rasterizer state
	if (m_RSRasterizerState != 0)
	{
		m_RSRasterizerState->Release();
		m_RSRasterizerState = NULL;
	}
	m_RSNumViewports = 0;
	m_RSScissorNumRects = 0;
}


gg::D3D11Renderer::D3D11Renderer(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context) :
	m_hwnd(hwnd),
	m_device(device),
	m_context(context),
	m_stateblock(nullptr),
	m_drawing(false)
{
}

gg::D3D11Renderer::~D3D11Renderer()
{
	reset();
}

gg::IRenderer::Backend gg::D3D11Renderer::getBackend() const
{
	return Backend::DIRECT3D11;
}

void* gg::D3D11Renderer::getBackendHandle() const
{
	return (void*)m_device;
}

bool gg::D3D11Renderer::getWindowDimensions(unsigned* width, unsigned* height) const
{
	RECT client_rect;
	GetClientRect(m_hwnd, &client_rect);
	*width = client_rect.right;
	*height = client_rect.bottom;
	return true;
}

gg::WindowHandle gg::D3D11Renderer::getWindowHandle() const
{
	return (WindowHandle)m_hwnd;
}

gg::D3D11TextObject* gg::D3D11Renderer::createTextObject() const
{
	return new D3D11TextObject();
}

void gg::D3D11Renderer::render()
{
	if (m_stateblock == nullptr)
		m_stateblock = new D3D11StateBlock(m_device, m_context);
	
	m_stateblock->Capture();

	unsigned width, height;
	getWindowDimensions(&width, &height);

	/*D3DVIEWPORT9 vp;
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
	m_device->SetTransform(D3DTS_PROJECTION, &proj);*/

	m_drawing = true;
	IRenderer::invokeRenderCallback();
	m_drawing = false;

	m_stateblock->Apply();
}

void gg::D3D11Renderer::reset()
{
	if (m_stateblock != nullptr)
	{
		m_stateblock->Release();
		delete m_stateblock;
		m_stateblock = nullptr;
	}

	for (auto& it : m_font_textures)
		it.texture->Release();
	m_font_textures.clear();
}

bool gg::D3D11Renderer::drawTextObject(const gg::ITextObject* itext, int x, int y, Color* color_ptr)
{
	if (!m_drawing)
		return false;

	if (itext->getBackend() != Backend::DIRECT3D11)
		return false;

	const D3D11TextObject* text = static_cast<const D3D11TextObject*>(itext);

	if (text->m_vertices.size() < 4)
		return false;

	/*D3DMATRIX model;
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
	m_device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, text->m_vertices.size() / 3, &(text->m_vertices[0]), sizeof(D3D11TextObject::Vertex));*/

	return true;
}

bool gg::D3D11Renderer::drawCaret(const gg::ITextObject* itext, int x, int y, int pos, gg::Color color)
{
	if (!m_drawing)
		return false;

	if (itext->getBackend() != Backend::DIRECT3D11)
		return false;

	const D3D11TextObject* text = static_cast<const D3D11TextObject*>(itext);

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

bool gg::D3D11Renderer::drawLine(int x1, int y1, int x2, int y2, gg::Color color)
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

	/*m_device->SetTextureStageState(0, D3DTSS_CONSTANT, color);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	m_device->SetFVF(D3DFVF_XYZRHW);
	m_device->DrawPrimitiveUP(D3DPT_LINELIST, 1, p, sizeof(Vertex));*/

	return true;
}

bool gg::D3D11Renderer::drawRectangle(int x, int y, int width, int height, gg::Color color)
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

	/*m_device->SetTextureStageState(0, D3DTSS_CONSTANT, color);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	m_device->SetFVF(D3DFVF_XYZRHW);
	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, p, sizeof(Vertex));*/

	return true;
}

static ID3D11Texture2D* createD3D11Texture(ID3D11Device* device, const unsigned char* bitmap, unsigned width, unsigned height)
{
	ID3D11Texture2D* texture = nullptr;

	gg::Color *font32 = new gg::Color[width * height];
	gg::Color *p = font32;
	for (unsigned i = 0; i < width * height; ++i, ++p)
		*p = 0x00ffffff | (((gg::Color)(bitmap[i])) << 24);

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = font32;
	data.SysMemPitch = width * sizeof(gg::Color);
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateTexture2D(&desc, &data, &texture);
	delete[] font32;

	if (SUCCEEDED(hr))
	{
		//ID3D11ShaderResourceView* texture_rv;
		//Dev->CreateShaderResourceView(texture, NULL, &texture_rv);
		return texture;
	}

	return nullptr;
}

ID3D11Texture2D* gg::D3D11Renderer::getFontTexture(const Font* font)
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
	ID3D11Texture2D* texture = createD3D11Texture(m_device, bitmap, width, height);

	if (texture)
		m_font_textures.push_back({ font, texture });

	return texture;
}


gg::D3D11TextObject::D3D11TextObject() :
	m_color(0xffffffff),
	m_font(gg::getNormalFont()),
	m_height(0)
{
}

gg::D3D11TextObject::~D3D11TextObject()
{
}

bool gg::D3D11TextObject::setText(const std::string& text, unsigned line_spacing, const gg::Font* font)
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

	D3D11TextObject::Vertex vtx;
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

bool gg::D3D11TextObject::setColor(gg::Color color)
{
	m_color = color;
	return true;
}

unsigned gg::D3D11TextObject::getHeight() const
{
	return m_height;
}

gg::IRenderer::Backend gg::D3D11TextObject::getBackend() const
{
	return IRenderer::Backend::DIRECT3D11;
}

#endif // _WIN32
