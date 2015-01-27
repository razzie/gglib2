/**
 * Copyright (c) 2014-2015 G�bor G�rzs�ny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#ifndef GG_RENDERER_HPP_INCLUDED
#define GG_RENDERER_HPP_INCLUDED

#include <cstdint>
#include <functional>

namespace gg
{
	class IRenderer;
	class ITextObject;

	typedef uint32_t Color;
	typedef std::function<void(IRenderer*)> RenderCallback;
	typedef void* WindowHandle;

	class IRenderer
	{
	public:
		enum Backend
		{
			UNKNOWN,
			OPENGL,
			DIRECT3D9,
			DIRECT3D10,
			DIRECT3D11
		};

		// setup hooks to initialize a renderer instance
		static bool injectHooks();

		// sets a callback which is called at the end of each frame rendering
		// the only way to obtain a pointer to an IRenderer instance is trough the callback
		static void setRenderCallback(RenderCallback);

		virtual ~IRenderer() {}
		virtual Backend getBackend() const = 0;
		virtual bool getWindowDimensions(unsigned*, unsigned*) const = 0;
		virtual WindowHandle getWindowHandle() const = 0;
		virtual ITextObject* createTextObject() const = 0;
		virtual void render() = 0;

		// drawing methods only work from inside the render callback
		virtual bool drawTextObject(ITextObject*, int x, int y) = 0;
		virtual bool drawRectangle(int x, int y, int width, int height, Color color) = 0;

	protected:
		bool invokeRenderCallback();
	};

	// don't inherit from this class, the instances are static_cast-ed internally
	class ITextObject
	{
	public:
		enum FontSize
		{
			SMALL,
			NORMAL, // default
			LARGE
		};

		virtual ~ITextObject() {}
		virtual bool setText(const char*) = 0;
		virtual bool setColor(Color) = 0;
		virtual bool setFontSize(FontSize) = 0;
		virtual unsigned getHeight() const = 0;
		virtual IRenderer::Backend getBackend() const = 0;
	};
};

#endif // GG_RENDERER_HPP_INCLUDED
