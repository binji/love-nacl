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

	Window::Window()
		: created(false),
                  graphics_3d(NULL),
                  width(0),
                  height(0)
	{
	}

	Window::~Window()
	{
	}

	bool Window::setWindow(int width, int height, bool fullscreen, bool vsync, int fsaa)
	{
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
                        this->width = width;
                        this->height = height;

			if (!g_Instance->BindGraphics(*graphics_3d))
                        {
				delete graphics_3d;
				graphics_3d = NULL;
				return false;
			}
		}
                else
                {
			int32_t result = graphics_3d->ResizeBuffers(width, height);
			if (result != PP_OK)
                        {
				delete graphics_3d;
				graphics_3d = NULL;
				return false;
			}
		}
		glSetCurrentContextPPAPI(graphics_3d->pp_resource());
		created = true;
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
		return this->width;
	}

	int Window::getHeight()
	{
		return this->height;
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

	void Window::onScreenChanged(int width, int height)
	{
		float screen_aspect = width / float(height);
		float graphics_aspect = this->width / float(this->height);
		float aspect_scale = graphics_aspect / screen_aspect;
		love::graphics::gles2::getContext()->setAspectScale(aspect_scale);
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
} // ppapi
} // window
} // love
