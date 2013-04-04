/**
**/

#ifndef LOVE_WINDOW_PPAPI_WINDOW_H
#define LOVE_WINDOW_PPAPI_WINDOW_H

// LOVE
#include <window/Window.h>

namespace love
{
namespace window
{
namespace ppapi
{
	class Window : public love::window::Window
	{
	private:
                bool created;

	public:
		Window();
		~Window();

		bool setWindow(int width = 800, int height = 600, bool fullscreen = false, bool vsync = true, int fsaa = 0);
		void getWindow(int &width, int &height, bool &fullscreen, bool &vsync, int &fsaa);

		bool checkWindowSize(int width, int height, bool fullscreen);
		WindowSize **getFullscreenSizes(int &n);

		int getWidth();
		int getHeight();

		bool isCreated();

		void setWindowTitle(std::string &title);
		std::string getWindowTitle();

		bool setIcon(love::image::ImageData *imgd);

		// default no-op implementation
		void swapBuffers();

		bool hasFocus();
		void setMouseVisible(bool visible);
		bool getMouseVisible();

		static Window *getSingleton();
	}; // Window
} // ppapi
} // window
} // love

#endif // LOVE_WINDOW_PPAPI_WINDOW_H

