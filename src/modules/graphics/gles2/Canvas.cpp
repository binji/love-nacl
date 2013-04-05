/**
* Copyright (c) 2006-2012 LOVE Development Team
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
**/

#include "Canvas.h"
#include "Graphics.h"
#include <common/Matrix.h>

#include <cstring> // For memcpy

namespace love
{
namespace graphics
{
namespace gles2
{
	Canvas* Canvas::current = NULL;

	Canvas::Canvas(int width, int height) :
		width(width), height(height)
	{
		float w = static_cast<float>(width);
		float h = static_cast<float>(height);

		// world coordinates
		vertices[0].x = 0;     vertices[0].y = 0;
		vertices[1].x = 0;     vertices[1].y = h;
		vertices[2].x = w; vertices[2].y = h;
		vertices[3].x = w; vertices[3].y = 0;

		// texture coordinates
		vertices[0].s = 0;     vertices[0].t = 1;
		vertices[1].s = 0;     vertices[1].t = 0;
		vertices[2].s = 1;     vertices[2].t = 0;
		vertices[3].s = 1;     vertices[3].t = 1;

		loadVolatile();
	}

	Canvas::~Canvas()
	{
		// reset framebuffer if still using this one
		if (current == this)
			stopGrab();

		unloadVolatile();
	}

	bool Canvas::isSupported()
	{
		return true;
	}

	void Canvas::bindDefaultCanvas()
	{
		if (current != NULL)
			current->stopGrab();
	}

	void Canvas::startGrab()
	{
          // TODO(binji): implement
#if 0
		// already grabbing
		if (current == this)
			return;

		// cleanup after previous fbo
		if (current != NULL)
			current->stopGrab();

		// bind buffer and clear screen
		glPushAttrib(GL_VIEWPORT_BIT | GL_TRANSFORM_BIT);
		bindFBO(fbo);
		glViewport(0, 0, width, height);

		// Reset the projection matrix
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();

		// Set up orthographic view (no depth)
		glOrtho(0.0, width, height, 0.0, -1.0, 1.0);

		// Switch back to modelview matrix
		glMatrixMode(GL_MODELVIEW);

		// indicate we are using this fbo
		current = this;
#endif
	}

	void Canvas::stopGrab()
	{
          // TODO(binji): implement
#if 0
		// i am not grabbing. leave me alone
		if (current != this)
			return;

		// bind default
		bindFBO( 0 );
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glPopAttrib();
		current = NULL;
#endif
	}


	void Canvas::clear(const Color& c)
	{
          // TODO(binji): implement
#if 0
		GLuint previous = 0;
		if (current != NULL)
			previous = current->fbo;

		bindFBO(fbo);
		glPushAttrib(GL_COLOR_BUFFER_BIT);
		glClearColor((float)c.r/255.0f, (float)c.g/255.0f, (float)c.b/255.0f, (float)c.a/255.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glPopAttrib();

		bindFBO(previous);
#endif
	}

	void Canvas::draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const
	{
		static Matrix t;
		t.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);

		drawv(t, vertices);
	}

	void Canvas::drawq(love::graphics::Quad * quad, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const
	{
		static Matrix t;
		quad->mirror(false, true);
		const vertex * v = quad->getVertices();

		t.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);
		drawv(t, v);
		quad->mirror(false, true);
	}

	love::image::ImageData * Canvas::getImageData(love::image::Image * image)
	{
		int row = 4 * width;
		int size = row * height;
		GLubyte* pixels = new GLubyte[size];
		GLubyte* screenshot = new GLubyte[size];

		bindFBO( fbo );
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		if (current)
			bindFBO( current->fbo );
		else
			bindFBO( 0 );

		GLubyte * src = pixels - row; // second line of source image
		GLubyte * dst = screenshot + size; // last row of destination image

		for (int i = 0; i < height; ++i)
			memcpy(dst -= row, src += row, row);

		love::image::ImageData* img = image->newImageData(width, height, (void*)screenshot);

		delete[] screenshot;
		delete[] pixels;

		return img;
	}

	void Canvas::setFilter(const Image::Filter &f)
	{
		GLint gmin = (f.min == Image::FILTER_NEAREST) ? GL_NEAREST : GL_LINEAR;
		GLint gmag = (f.mag == Image::FILTER_NEAREST) ? GL_NEAREST : GL_LINEAR;

		bindTexture(img);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gmin);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gmag);
	}

	Image::Filter Canvas::getFilter() const
	{
		GLint gmin, gmag;

		bindTexture(img);
		glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &gmin);
		glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &gmag);

		Image::Filter f;
		f.min = (gmin == GL_NEAREST) ? Image::FILTER_NEAREST : Image::FILTER_LINEAR;
		f.mag = (gmag == GL_NEAREST) ? Image::FILTER_NEAREST : Image::FILTER_LINEAR;
		return f;
	}

	void Canvas::setWrap(const Image::Wrap &w)
	{
		GLint wrap_s = (w.s == Image::WRAP_CLAMP) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
		GLint wrap_t = (w.t == Image::WRAP_CLAMP) ? GL_CLAMP_TO_EDGE : GL_REPEAT;

		bindTexture(img);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
	}

	Image::Wrap Canvas::getWrap() const
	{
		GLint wrap_s, wrap_t;
		bindTexture(img);
		glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrap_s);
		glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &wrap_t);

		Image::Wrap w;
		w.s = (wrap_s == GL_CLAMP_TO_EDGE) ? Image::WRAP_CLAMP : Image::WRAP_REPEAT;
		w.t = (wrap_t == GL_CLAMP_TO_EDGE) ? Image::WRAP_CLAMP : Image::WRAP_REPEAT;

		return w;
	}

	bool Canvas::loadVolatile()
	{
		status = createFBO(fbo, depth_stencil, img, width, height);
		if (status != GL_FRAMEBUFFER_COMPLETE)
			return false;

		setFilter(settings.filter);
		setWrap(settings.wrap);
		Color c;
		c.r = c.g = c.b = c.a = 0;
		clear(c);
		return true;
	}

	void Canvas::unloadVolatile()
	{
		settings.filter = getFilter();
		settings.wrap   = getWrap();
		deleteFBO(fbo, depth_stencil, img);
	}

	int Canvas::getWidth()
	{
		return width;
	}

	int Canvas::getHeight()
	{
		return height;
	}

	void Canvas::drawv(const Matrix & t, const vertex * v) const
	{
          // TODO(binji): implement
#if 0
		glPushMatrix();

		glMultMatrixf((const GLfloat*)t.getElements());

		bindTexture(img);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)&v[0].x);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)&v[0].s);
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glPopMatrix();
#endif
	}

	GLenum Canvas::createFBO(GLuint& framebuffer, GLuint& depth_stencil,  GLuint& img, int width, int height)
	{
		// get currently bound fbo to reset to it later
		GLint current_fbo;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fbo);

		// create framebuffer
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

                        // TODO(binji): implement
#if 0
		// create stencil buffer
		glGenRenderbuffers(1, &depth_stencil);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
				GL_RENDERBUFFER, depth_stencil);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				GL_RENDERBUFFER, depth_stencil);
#endif

		// generate texture save target
		glGenTextures(1, &img);
		bindTexture(img);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
				0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		bindTexture(0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D, img, 0);

		// check status
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		// unbind framebuffer
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)current_fbo);
		return status;
	}

	void Canvas::deleteFBO(GLuint framebuffer, GLuint depth_stencil,  GLuint img)
	{
		deleteTexture(img);
		glDeleteRenderbuffers(1, &depth_stencil);
		glDeleteFramebuffers(1, &framebuffer);
	}

	void Canvas::bindFBO(GLuint framebuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	}

} // gles2
} // graphics
} // love
