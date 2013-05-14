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
#include "Context.h"
#include "Graphics.h"
#include <common/Matrix.h>

#include <cstring> // For memcpy

namespace {


}  // namespace

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
		// already grabbing
		if (current == this)
			return;

		Context *ctx = getContext();

		// cleanup after previous fbo
		if (current != NULL)
			current->stopGrab();

		// bind buffer and clear screen
		bindFBO(fbo);
		ctx->pushViewport(0, 0, width, height);

		// Set the orthographic projection matrix to this canvas' dimensions
		Matrix ortho = Matrix::ortho(0.0f, width, height, 0.0f);
		ctx->projectionStack.push(ortho);

		// indicate we are using this fbo
		current = this;
	}

	void Canvas::stopGrab()
	{
		// i am not grabbing. leave me alone
		if (current != this)
			return;

		Context *ctx = getContext();

		// bind default
		bindFBO(ctx->getDefaultFramebuffer());

		// Restore previous orthographic projection matrix
		if (ctx->projectionStack.size() > 1)
			ctx->projectionStack.pop();

		// Restore the previous framebuffer's viewport
		ctx->popViewport();

		current = NULL;
	}


	void Canvas::clear(const Color& c)
	{
		Context *ctx = getContext();

		GLuint previous = ctx->getDefaultFramebuffer();
		if (current != NULL)
			previous = current->fbo;

		bindFBO(fbo);

		Color defaultcolor = ctx->getClearColor();
		ctx->setClearColor(c);

		glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// preserve the background color of the default framebuffer
		ctx->setClearColor(defaultcolor);

		bindFBO(previous);
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
		Context *ctx = getContext();
		ctx->bindTexture(img);
                ctx->setTextureFilter(f);
	}

	Image::Filter Canvas::getFilter() const
	{
		Context *ctx = getContext();
		ctx->bindTexture(img);
                return ctx->getTextureFilter();
	}

	void Canvas::setWrap(const Image::Wrap &w)
	{
		Context *ctx = getContext();
		ctx->bindTexture(img);
                ctx->setTextureWrap(w);
	}

	Image::Wrap Canvas::getWrap() const
	{
		Context *ctx = getContext();
		ctx->bindTexture(img);
		return ctx->getTextureWrap();
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
		Context *ctx = getContext();

		ctx->modelViewStack.push(ctx->modelViewStack.top());
		ctx->modelViewStack.top() *= t;

		ctx->bindTexture(img);

		ctx->useVertexAttribArrays(Context::ATTRIB_VERTEX | Context::ATTRIB_TEXCOORD);

		ctx->vertexAttribPointer(Context::ATTRIB_VERTEX, 2, GL_FLOAT, sizeof(vertex), (GLvoid *)&v[0].x);
		ctx->vertexAttribPointer(Context::ATTRIB_TEXCOORD, 2, GL_FLOAT, sizeof(vertex), (GLvoid *)&v[0].s);

		ctx->setupRender();
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		ctx->modelViewStack.pop();
	}

	GLenum Canvas::createFBO(GLuint& framebuffer, GLuint& depth_stencil,  GLuint& img, int width, int height)
	{
		Context *ctx = getContext();

		// get currently bound fbo to reset to it later
		GLint current_fbo;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fbo);

		// create framebuffer
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		// create stencil buffer
		depth_stencil = createStencil(width, height);

		// generate texture save target
		glGenTextures(1, &img);
		ctx->bindTexture(img);

		ctx->setTextureFilter(Image::Filter());

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
			0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		ctx->bindTexture(0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, img, 0);

		// check status
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		// unbind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, (GLuint) current_fbo);
		return status;
	}

	GLuint Canvas::createStencil(int width, int height)
	{
#if 0
		GLuint depth_stencil;

		glGenRenderbuffers(1, &depth_stencil);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
								  GL_RENDERBUFFER, depth_stencil);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
								  GL_RENDERBUFFER, depth_stencil);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		return depth_stencil;
#endif
                return 0;
	}

	void Canvas::deleteFBO(GLuint framebuffer, GLuint depth_stencil,  GLuint img)
	{
		getContext()->deleteTexture(img);
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
