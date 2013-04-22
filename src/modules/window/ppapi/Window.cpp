/**
**/

// STL
#include <iostream>

// LOVE
#include <common/config.h>
#include <graphics/gles2/Context.h>
#include "Window.h"

#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/graphics_3d.h>
#include <ppapi/lib/gl/gles2/gl2ext_ppapi.h>

namespace love
{
namespace window
{
namespace ppapi
{

extern pp::Instance* g_Instance;

	Window::Window(int screenWidth, int screenHeight)
		: created(false),
		  graphics_3d(NULL),
		  width(800),
		  height(600),
		  screenWidth(screenWidth),
		  screenHeight(screenHeight)
	{
		singleton = this;
	}

	Window::~Window()
	{
	}

	bool Window::setWindow(int width, int height, bool fullscreen, bool vsync, int fsaa)
	{
		this->width = width;
		this->height = height;
		onScreenChanged(screenWidth, screenHeight);
		return true;
	}

	void Window::getWindow(int &width, int &height, bool &fullscreen, bool &vsync, int &fsaa)
	{
		width = this->width;
		height = this->height;
		fullscreen = false;
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

	int Window::getScreenWidth()
	{
		return screenWidth;
	}

	int Window::getScreenHeight()
	{
		return screenHeight;
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
		graphics_3d->SwapBuffers(pp::CompletionCallback());
	}

	bool Window::hasFocus()
	{
	}

	void Window::setMouseVisible(bool visible)
	{
	}

	bool Window::getMouseVisible()
	{
		return true;
	}

	void Window::onScreenChanged(int w, int h)
	{
		screenWidth = w;
		screenHeight = h;
		if (!createContext(w, h))
			return;

		float x = 0.5f * (screenWidth - width);
		float y = 0.5f * (screenHeight - height);
		screenToWindowMatrix.setIdentity();
		screenToWindowMatrix.translate(-x, -y);
		love::graphics::gles2::getContext()->setMainViewport(
		    x, y, width, height);
	}

	void Window::screenToWindow(int x, int y, int &out_x, int &out_y)
	{
		Vector out = screenToWindowMatrix.transform(Vector(x, y));
		out_x = static_cast<int>(out.x);
		out_y = static_cast<int>(out.y);
	}

	love::window::Window *Window::getSingleton()
	{
		return singleton;
	}

	const char *Window::getName() const
	{
		return "love.window.ppapi";
	}

        bool Window::createContext(int width, int height)
        {
		printf("createContext(%d, %d)\n", width, height);
		if (!graphics_3d)
		{
			int32_t attribs[] = {
				PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
				PP_GRAPHICS3DATTRIB_SAMPLES, 0,
				PP_GRAPHICS3DATTRIB_SAMPLE_BUFFERS, 0,
				PP_GRAPHICS3DATTRIB_WIDTH, width,
				PP_GRAPHICS3DATTRIB_HEIGHT, height,
				PP_GRAPHICS3DATTRIB_NONE
			};

			graphics_3d = new pp::Graphics3D(g_Instance, attribs);
			if (!g_Instance->BindGraphics(*graphics_3d))
				goto failed;
			printf("New buffer: %dx%d\n", width, height);
		}
		else
		{
			// no need to resize.
			if (width == contextWidth && height == contextHeight)
			{
				printf("Buffer already %dx%d\n", width, height);
				return true;
			}

			printf("Old buffer: %dx%d\n", contextWidth, contextHeight);

			int32_t result = graphics_3d->ResizeBuffers(width, height);
			if (result != PP_OK)
				goto failed;
			printf("Resized buffer: %dx%d\n", width, height);
		}

		contextWidth = width;
		contextHeight = height;

		glSetCurrentContextPPAPI(graphics_3d->pp_resource());
		created = true;
		return true;

	failed:
		printf("Failed...\n");
		delete graphics_3d;
		graphics_3d = NULL;
		return false;
	}
} // ppapi
} // window
} // love
