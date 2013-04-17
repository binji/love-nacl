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
		  height(0),
		  screenWidth(0),
		  screenHeight(0)
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
			if (!g_Instance->BindGraphics(*graphics_3d))
                        {
				delete graphics_3d;
				graphics_3d = NULL;
				return false;
			}
                        printf("New buffer: %dx%d\n", width, height);
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
                        printf("Resized buffer: %dx%d\n", width, height);
		}
		this->width = width;
		this->height = height;

		glSetCurrentContextPPAPI(graphics_3d->pp_resource());
		created = true;
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

	void Window::onScreenChanged(int w, int h)
	{
		screenWidth = w;
		screenHeight = h;

		float screen_aspect = screenWidth / float(screenHeight);
		float graphics_aspect = width / float(height);
		float aspect_scale = graphics_aspect / screen_aspect;

		float screen_x_offset = 0;
		float screen_y_offset = 0;
		float graphics_x_offset = 0;
		float graphics_y_offset = 0;
		float scaled_width = width;
		float scaled_height = height;
		float width_scale = width / float(screenWidth);
		float height_scale = height / float(screenHeight);

		screenToWindowMatrix.setIdentity();
		if (aspect_scale > 1.0f)
		{
			screen_y_offset = 0.5f * (screenHeight * (1 / aspect_scale - 1));
                        scaled_height = height / aspect_scale;
                        height_scale *= aspect_scale;
			graphics_y_offset = 0.5f * (height - scaled_height);
                }
		else
                {
			screen_x_offset = 0.5f * (screenWidth * (aspect_scale - 1));
                        scaled_width = width * aspect_scale;
                        width_scale /= aspect_scale;
			graphics_x_offset = 0.5f * (width - scaled_width);
		}

		printf("screen: %d x %d\n", screenWidth, screenHeight);
		printf("graphics: %d x %d\n", width, height);
		printf("scale: %g x %g\n", width_scale, height_scale);
		printf("screen_offset %g x %g\n", screen_x_offset, screen_y_offset);
                printf("viewport: <%g, %g, %g, %g>\n", graphics_x_offset, graphics_y_offset, scaled_width, scaled_height);

		screenToWindowMatrix.scale(width_scale, height_scale);
		screenToWindowMatrix.translate(screen_x_offset, screen_y_offset);
                love::graphics::gles2::getContext()->setMainViewport(
                    graphics_x_offset, graphics_y_offset, scaled_width, scaled_height);
	}

	void Window::screenToWindow(int x, int y, int &out_x, int &out_y)
	{
		Vector out = screenToWindowMatrix.transform(Vector(x, y));
		out_x = static_cast<int>(out.x);
		out_y = static_cast<int>(out.y);
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
