/**
**/

#include <stdio.h>

// STL
#include <iostream>

// LOVE
#include <common/config.h>
#include <graphics/gles2/Context.h>
#include "Window.h"

#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/graphics_3d.h>
#include <ppapi/cpp/var.h>
#include <ppapi/lib/gl/gles2/gl2ext_ppapi.h>

namespace love
{
namespace window
{
namespace ppapi
{

extern pp::Instance* g_Instance;

	Window::Window()
		: fullscreen(g_Instance),
		  graphics3d(NULL),
		  width(0),
		  height(0),
		  created(false),
		  focused(false)
	{
	}

	Window::~Window()
	{
	}

	bool Window::setWindow(int width, int height, bool wantFullscreen, bool vsync, int fsaa)
	{
		if (wantFullscreen != fullscreen.IsFullscreen())
		{
			fullscreen.SetFullscreen(wantFullscreen);
		}

		createContext(width, height);
		return true;
	}

	void Window::getWindow(int &width, int &height, bool &isFullscreen, bool &vsync, int &fsaa)
	{
		width = this->width;
		height = this->height;
		isFullscreen = fullscreen.IsFullscreen();
		vsync = true;
		fsaa = false;
	}

	bool Window::checkWindowSize(int width, int height, bool fullscreen)
	{
		return true;
	}

	typedef Window::WindowSize WindowSize;

	WindowSize **Window::getFullscreenSizes(int &n)
	{
		n = 1;
		WindowSize **sizes = new WindowSize*[n];

		// TODO(binji): revisit
		sizes[0] = new WindowSize;
		sizes[0]->width = 800;
		sizes[0]->height = 600;
		return sizes;
	}

	int Window::getWidth()
	{
		return width;
	}

	int Window::getHeight()
	{
		return height;
	}

	bool Window::isCreated()
	{
		return created;
	}

	void Window::setWindowTitle(std::string &title)
	{
	}

	std::string Window::getWindowTitle()
	{
		return std::string();
	}

	bool Window::setIcon(love::image::ImageData *imgd)
	{
		return true;
	}

	void Window::swapBuffers()
	{
		// Blocking.
		if (graphics3d)
                {
			graphics3d->SwapBuffers(pp::BlockUntilComplete());
		}
	}

	bool Window::hasFocus()
	{
		return focused;
	}

	void Window::setMouseVisible(bool visible)
	{
	}

	bool Window::getMouseVisible()
	{
		return true;
	}

	void Window::onFocusChanged(bool hasFocus)
        {
		focused = hasFocus;
	}

	love::window::Window *Window::getSingleton()
	{
		if (!singleton)
			singleton = new Window();
		else
			singleton->retain();

		return singleton;
	}

	const char *Window::getName() const
	{
		return "love.window.ppapi";
	}

        bool Window::createContext(int w, int h)
        {
		if (!graphics3d)
		{
			int32_t attribs[] = {
				PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
				PP_GRAPHICS3DATTRIB_STENCIL_SIZE, 8,
				PP_GRAPHICS3DATTRIB_SAMPLES, 0,
				PP_GRAPHICS3DATTRIB_SAMPLE_BUFFERS, 0,
				PP_GRAPHICS3DATTRIB_WIDTH, w,
				PP_GRAPHICS3DATTRIB_HEIGHT, h,
				PP_GRAPHICS3DATTRIB_NONE
			};

			graphics3d = new pp::Graphics3D(g_Instance, attribs);
			if (!g_Instance->BindGraphics(*graphics3d))
				goto failed;

			glSetCurrentContextPPAPI(graphics3d->pp_resource());
		}
		else
		{
			// no need to resize.
			if (w == width && h == height)
			{
				return true;
			}

			int32_t result = graphics3d->ResizeBuffers(w, h);
			if (result != PP_OK)
				goto failed;
		}

		width = w;
		height = h;

		char buffer[32];
		snprintf(buffer, 32, "setWindow:%d,%d", w, h);
		g_Instance->PostMessage(&buffer[0]);

		created = true;
		return true;

	failed:
		fprintf(stderr, "Failed to create Graphics3D context!\n");
		delete graphics3d;
		graphics3d = NULL;
		return false;
	}

} // ppapi
} // window
} // love
