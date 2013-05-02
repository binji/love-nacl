/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

#ifndef LOVE_GRAPHICS_OPENGL_CONTEXT_H
#define LOVE_GRAPHICS_OPENGL_CONTEXT_H

#include "GLES2/gl2.h"

#include "common/Matrix.h"
#include "graphics/Color.h"
#include "graphics/Image.h"

#include <vector>
#include <map>
#include <stack>

namespace love
{
namespace graphics
{
namespace gles2
{

class Shader;

/**
 * Thin layer between OpenGL and the rest of the program.
 * Shadows OpenGL context state internally for better efficiency, and
 * automatically uses the correct OpenGL functions depending on runtime support.
 **/
class Context
{
	friend Context *getContext();
	friend Context *resetContext();

public:

	// Standard vertex attributes.
	enum VertexAttribType
	{
		ATTRIB_NONE = 0x00,
		ATTRIB_VERTEX = 0x01,
		ATTRIB_COLOR = 0x02,
		ATTRIB_TEXCOORD = 0x04,
	};

	// Structure for viewport state.
	struct Viewport
	{
		GLint x, y;
		GLsizei width, height;

		Viewport() : x(0), y(0), width(0), height(0) { };
		Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
		: x(x), y(y), width(width), height(height) { };
	};

	// Structure for blend mode state.
	struct BlendState
	{
		GLenum function;
		GLenum src_rgb, src_a;
		GLenum dst_rgb, dst_a;
	};

	// Transformation matrix stacks used when rendering.
	std::stack<Matrix> modelViewStack;
	std::stack<Matrix> projectionStack;
	std::vector<Viewport> viewportStack;
        Matrix screenToWindowMatrix;

	Context();
	~Context();

	void initState();

	/**
	 * Sets state required for rendering.
	 * Call this method directly before every draw call!
	 **/
	void setupRender();

	/**
	 * Wrapper for glEnable/glDisable, enables or disables the specified OpenGL
	 * capability.
	 **/
	void setCapability(GLenum capability, bool enable);

	/**
	 * Wrapper for glIsEnabled, returns whether the specified OpenGL capability
	 * is enabled.
	 **/
	bool getCapability(GLenum capability);

	/**
	 * Sets the current constant color (replaces glColor4ub).
	 **/
	void setColor(const Color &color);

	/**
	 * Gets the current constant color.
	 **/
	const Color &getColor() const;

	/**
	 * Sets the color used when clearing the screen for the currently active
	 * framebuffer.
	 **/
	void setClearColor(const Color &color);

	/**
	 * Gets the last clear color set using setClearColor. May be different from
	 * the clear color on the currently active framebuffer!
	 **/
	const Color &getClearColor() const;

	/**
	 * Sets the current point size.
	 **/
	void setPointSize(float size);

	/**
	 * Gets the current point size.
	 **/
	float getPointSize() const;

	/**
	 * Gets the implementation-dependent maximum point size.
	 **/
	float getMaxPointSize() const;

	/**
	 * Sets the current blend state.
	 **/
	void setBlendState(const BlendState &s);
	void setBlendState(GLenum function, GLenum source, GLenum destination);

	/**
	 * Gets the current blend state.
	 **/
	const BlendState &getBlendState() const;


	void setMainViewport(GLint x, GLint y, GLsizei width, GLsizei height);
	void pushViewport(const Viewport &v);
	void pushViewport(GLint x, GLint y, GLsizei width, GLsizei height);
	void popViewport();

	/**
	 * Gets the last window viewport set with setViewport.
	 **/
	const Viewport &getViewport() const;

	/**
	 * Gets whether a vertex attribute solely uses the programmable pipeline
	 * (AKA is generic). Generic vertex attributes sometimes require slightly
	 * different code paths compared to the standard fixed-function ones.
	 *
	 * @param attrib See VertexAttribType.
	 **/
	bool isGenericVertexAttrib(unsigned int attrib) const;

	/**
	 * Gets the OpenGL representation of the specified vertex attribute.
	 * Will be the attribute index for generic vertex attributes, or a
	 * GL_*_ARRAY for fixed-function ones.
	 *
	 * @param attrib See VertexAttribType.
	 **/
	GLint getVertexAttribID(unsigned int attrib) const;

	/**
	 * Sets whether a vertex attribute is active when rendering a vertex array.
	 *
	 * @param attrib The vertex attribute to change the active state of (see
	 * VertexAttribType.)
	 * @param enable Whether to enable or disable this vertex attribute for the
	 * array.
	 **/
	void setVertexAttribArray(unsigned int attrib, bool enable);

	/**
	 * Sets the per-vertex attributes used when rendering a vertex array.
	 *
	 * @param attribs Bitfield of vertex attributes to be enabled (see
	 * VertexAttribType.)
	 **/
	void useVertexAttribArrays(unsigned int attribs);

	/**
	 * Define an array of vertex attribute data for a specific vertex attribute.
	 * See http://www.opengl.org/sdk/docs/man2/xhtml/glVertexAttribPointer.xml
	 *
	 * @param attrib The attribute type to define the array for.
	 * @param size The number of vector components per attribute.
	 * @param type The data type of each component in the array, e.g. GL_FLOAT.
	 * @param stride Offset in bytes between consecutive vertex attributes.
	 * @param pointer Pointer to the attribute array, or byte offset into the
	 * currently bound vertex buffer if applicable.
	 **/
	void vertexAttribPointer(VertexAttribType attrib, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) const;

	/**
	 * Gets the default framebuffer used when no Canvas is set.
	 **/
	GLuint getDefaultFramebuffer() const;

	/**
	 * Sets the active texture unit.
	 *
	 * @param textureunit Index in the range of [0, maxtextureunits-1]
	 **/
	void setActiveTextureUnit(int textureunit);

	/**
	 * Binds an OpenGL texture to the active texture unit.
	 * Makes sure we aren't redundantly binding textures.
	 **/
	void bindTexture(GLuint texture);

	/**
	 * Binds a texture to a specific texture unit.
	 *
	 * @param textureunit Index in the range of [0, maxtextureunits-1]
	 * @param resoreprev Restore previously bound texture unit when done.
	 **/
	void bindTextureToUnit(GLuint texture, int textureunit, bool restoreprev);

	/**
	 * Gets the number of texture units supported on the current system.
	 **/
	int getNumTextureUnits() const { return state.textureUnits.size(); };

	/**
	 * Deletes an OpenGL texture.
	 * Cleans up if the texture is currently bound.
	 **/
	void deleteTexture(GLuint texture);

	/**
	 * Sets the image filter mode for the currently bound texture.
	 * Returns the actual amount of anisotropic filtering set.
	 */
	float setTextureFilter(const graphics::Image::Filter &f) const;

	/**
	 * Gets the image filter mode for the currently bound texture.
	 */
	graphics::Image::Filter getTextureFilter() const;

	/**
	 * Sets the image wrap mode for the currently bound texture.
	 */
	void setTextureWrap(const graphics::Image::Wrap &w) const;

	/**
	 * Gets the image wrap mode for the currently bound texture.
	 */
	graphics::Image::Wrap getTextureWrap() const;

	void setScreenToWindowMatrix(const Matrix& m);

private:

	void initCapabilityState();
	void initMatrixState();
	void initVertexAttribState();
	void initTextureState();
	void createDefaultTexture();

	bool isCapabilitySupported(GLenum capability) const;

	// Tracked OpenGL state.
	struct
	{
		// Tracks glEnable/glDisable.
		std::map<GLenum, bool> enabledCapabilities;

		// Current color.
		Color color;

		// The last clear color set with setClearColor.
		Color clearColor;

		float pointSize;
		float maxPointSize;
		float lastShaderPointSize;

		// The current blending state.
		BlendState blend;

		// The currently active transformation matrices used when rendering.
		Matrix modelViewMatrix;
		Matrix projectionMatrix;
		Matrix screenToWindowMatrix;

		// Map of vertex attributes to internal OpenGL attribute indices.
		std::map<unsigned int, GLenum> vertexAttribMap;

		// List of currently enabled vertex attribute arrays.
		std::map<unsigned int, bool> enabledVertexAttribArrays;

		// Tracks bound texture for each unit.
		std::vector<GLuint> textureUnits;
		int curTextureUnit;

		// The last active shader used when rendering.
		Shader *lastUsedShader;

		GLuint defaultFramebuffer;

	} state;

	bool shadersSupported;
	float maxAnisotropy;

	// Pointer to the currently active context.
	static Context *current;
};

/**
 * Returns a pointer to the currently active context. Creates a new context if
 * one doesn't exist yet.
 **/
Context *getContext();

/**
 * Resets the context state by destroying the current context and creating a new
 * one. Invalidates old context pointers!
 **/
Context *resetContext();

} // gles2
} // graphics
} // love

#endif
