/**
**/

// STL
#include <iostream>

// LOVE
#include <common/config.h>
#include "Window.h"

namespace love
{
namespace window
{
namespace ppapi
{
	Window::Window()
		: created(false)
	{
	}

	Window::~Window()
	{
	}

	bool Window::setWindow(int width, int height, bool fullscreen, bool vsync, int fsaa)
	{
                // TODO(binji): implement
		created = true;
		return true;
	}

	void Window::getWindow(int &width, int &height, bool &fullscreen, bool &vsync, int &fsaa)
	{
		width = 800;
		height = 600;
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
		return 800;
	}

	int Window::getHeight()
	{
		return 600;
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
