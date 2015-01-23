/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
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

	typedef uint32_t Color;
	typedef std::function<void(IRenderer*)> RenderCallback;
	typedef void* WindowHandle;

	// don't inherit from this class, as the instances will be static_cast'ed
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
	};

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

		/*// available AFTER OpenGL or Direct3D context is initialized
		static IRenderer* getInstance();*/

		// sets a callback which is called at the end of each frame rendering
		// the only way to obtain a pointer to an IRenderer instance is trough the callback
		static bool setRenderCallback(RenderCallback);

		virtual ~IRenderer() {}
		virtual Backend getBackend() const = 0;
		virtual bool getWindowDimensions(unsigned*, unsigned*) const = 0;
		virtual WindowHandle getWindowHandle() const = 0;
		virtual ITextObject* createTextObject() const = 0;

		// drawing methods only work from inside the render callback
		virtual bool drawTextObject(ITextObject*, int x, int y) = 0;
		virtual bool drawRectangle(int x, int y, int width, int height, Color color) = 0;
	};
};

#endif // GG_RENDERER_HPP_INCLUDED
