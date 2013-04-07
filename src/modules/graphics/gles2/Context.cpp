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

#include "common/config.h"
#include "common/Exception.h"

#include "Context.h"
#include "Image.h"
#include "Shader.h"

#include <algorithm>

namespace love
{
namespace graphics
{
namespace gles2
{

Context *Context::current = NULL;

Context *getContext()
{
	if (Context::current == NULL)
		Context::current = new Context;

	return Context::current;
}

Context *resetContext()
{
	if (Context::current != NULL)
	{
		delete Context::current;
		Context::current = NULL;
	}

	return getContext();
}

Context::Context()
	: shadersSupported(false)
	, maxAnisotropy(1.0f)
{
	initState();
	createDefaultTexture();
}

Context::~Context()
{
	if (current == this)
		current = NULL;
}

void Context::initCapabilityState()
{
	state.enabledCapabilities.clear();

	// getCapability starts tracking the state if the capability isn't in the map yet
	getCapability(GL_TEXTURE_2D);
	getCapability(GL_BLEND);
	getCapability(GL_SCISSOR_TEST);
	getCapability(GL_STENCIL_TEST);
	getCapability(GL_SCISSOR_TEST);
}

void Context::initTextureState()
{
	state.textureUnits.clear();

	// Initialize multiple texture unit support, if available
	if (shadersSupported)
	{
		GLint maxtextureunits;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxtextureunits);

		state.textureUnits.resize(maxtextureunits, 0);

		GLenum curgltextureunit;
		glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint *) &curgltextureunit);

		state.curTextureUnit = curgltextureunit - GL_TEXTURE0;

		// Retrieve currently bound textures for each texture unit
		for (size_t i = 0; i < state.textureUnits.size(); ++i)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint *) &state.textureUnits[i]);
		}

		glActiveTexture(curgltextureunit);
	}
	else
	{
		// Multitexturing not supported, so we only have 1 texture unit
		state.textureUnits.resize(1, 0);
		state.curTextureUnit = 0;

		glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint *) &state.textureUnits[0]);
	}
}

void Context::createDefaultTexture()
{
	// Set the 'default' texture (id 0) as a repeating white pixel.
	// Otherwise, texture2D inside a shader would return black when drawing graphics primitives,
	// which would require the creation of different default shaders for untextured primitives vs images.

	GLuint curtexture = state.textureUnits[state.curTextureUnit];
	bindTexture(0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	GLubyte pixel = 255;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 1, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &pixel);

	bindTexture(curtexture);
}

void Context::initMatrixState()
{
	// Set up transformation matrix stacks

	while (!modelViewStack.empty())
		modelViewStack.pop();

	modelViewStack.push(Matrix());

	while (!projectionStack.empty())
		projectionStack.pop();

	projectionStack.push(Matrix());
}

void Context::initVertexAttribState()
{
	// set vertex attribute index map
	state.vertexAttribMap.clear();

	// use explicit generic attribute index locations with shaders
	state.vertexAttribMap[ATTRIB_VERTEX] = 0;
	state.vertexAttribMap[ATTRIB_COLOR] = 1;
	state.vertexAttribMap[ATTRIB_TEXCOORD] = 2;

	// making sure all vertex attributes are disabled is much easier than querying them
	state.enabledVertexAttribArrays.clear();
	useVertexAttribArrays(ATTRIB_NONE);
}

void Context::initState()
{
	shadersSupported = Shader::isSupported();

	initCapabilityState();
	initTextureState();
	initMatrixState();
	initVertexAttribState();

	// get blend state
	glGetIntegerv(GL_BLEND_SRC_RGB, (GLint *) &state.blend.src_rgb);
	glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint *) &state.blend.src_a);
	glGetIntegerv(GL_BLEND_DST_RGB, (GLint *) &state.blend.dst_rgb);
	glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint *) &state.blend.dst_a);
	glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint *) &state.blend.function);


	// get the current color
	GLfloat color[4];

	glGetVertexAttribfv(state.vertexAttribMap[ATTRIB_COLOR], GL_CURRENT_VERTEX_ATTRIB, color);

	state.color = Color(color[0] * 255, color[1] * 255, color[2] * 255, color[3] * 255);

	// get the current clear color
	glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
	state.clearColor = Color(color[0] * 255, color[1] * 255, color[2] * 255, color[3] * 255);

	// get the current point size
	state.lastShaderPointSize = state.pointSize;

	// get the maximum point size
	GLfloat pointsizerange[2];

	glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, pointsizerange);

	state.maxPointSize = pointsizerange[1];

	state.lastUsedShader = NULL;

	// use the currently bound framebuffer as the default
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &state.defaultFramebuffer);

	// get the maximum anisotropic filtering value
	maxAnisotropy = 1.0f;
}

void Context::setupRender()
{
	const Matrix &projectionMatrix = projectionStack.top();
	const Matrix &modelViewMatrix = modelViewStack.top();

	Shader *shader = Shader::currentShader;

	bool shaderchanged = shader != state.lastUsedShader;

	bool mvmatrixchanged = shaderchanged || modelViewMatrix != state.modelViewMatrix;
	bool pmatrixchanged = shaderchanged || projectionMatrix != state.projectionMatrix;

	if (shader != NULL)
	{
		bool pointsizechanged = shaderchanged || state.pointSize != state.lastShaderPointSize;

		if (pointsizechanged && shader->hasUniform("PointSize"))
		{
			shader->sendFloat("PointSize", 1, (GLfloat *) &state.pointSize, 1);
			state.lastShaderPointSize = state.pointSize;
		}

		// send transformation matrices to the active shader for use when rendering

		if (mvmatrixchanged && shader->hasUniform("ModelViewMatrix"))
			shader->sendMatrix("ModelViewMatrix", 4, modelViewMatrix.getElements(), 1);

		if (pmatrixchanged && shader->hasUniform("ProjectionMatrix"))
			shader->sendMatrix("ProjectionMatrix", 4, projectionMatrix.getElements(), 1);

		if ((mvmatrixchanged || pmatrixchanged) && shader->hasUniform("ModelViewProjectionMatrix"))
		{
			Matrix mvpMatrix = projectionMatrix * modelViewMatrix;
			shader->sendMatrix("ModelViewProjectionMatrix", 4, mvpMatrix.getElements(), 1);
		}

		// TODO: normal matrix
		// "transpose of the inverse of the upper leftmost 3x3 of the Model-View Matrix"
	}

	if (mvmatrixchanged)
		state.modelViewMatrix = modelViewMatrix;

	if (pmatrixchanged)
		state.projectionMatrix = projectionMatrix;

	state.lastUsedShader = shader;
}

bool Context::isCapabilitySupported(GLenum capability) const
{
	// Only a few glEnable / glDisable capabilities are supported in ES2
	switch (capability)
	{
	case GL_BLEND:
	case GL_STENCIL_TEST:
	case GL_SCISSOR_TEST:
	case GL_DEPTH_TEST:
	case GL_CULL_FACE:
	case GL_DITHER:
	case GL_POLYGON_OFFSET_FILL:
	case GL_SAMPLE_COVERAGE:
	case GL_SAMPLE_ALPHA_TO_COVERAGE:
		return true;
	default:
		return false;
	}
}

void Context::setCapability(GLenum capability, bool enable)
{
	if (!isCapabilitySupported(capability))
		return;

	// is this capability already set to the state we want?
	std::map<GLenum, bool>::const_iterator cap = state.enabledCapabilities.find(capability);
	if (cap != state.enabledCapabilities.end() && cap->second == enable)
		return;

	state.enabledCapabilities[capability] = enable;

	if (enable)
		glEnable(capability);
	else
		glDisable(capability);
}

bool Context::getCapability(GLenum capability)
{
	// Assume disabled if toggling it isn't supported
	if (!isCapabilitySupported(capability))
		return false;

	// Return cached enable state for the capability, if we have it
	std::map<GLenum, bool>::const_iterator cap = state.enabledCapabilities.find(capability);
	if (cap != state.enabledCapabilities.end())
		return cap->second;

	// Get the current enable state from GL and stick it in the map
	bool isenabled = glIsEnabled(capability) == GL_TRUE;
	state.enabledCapabilities[capability] = isenabled;

	return isenabled;
}

void Context::setColor(const Color &color)
{
//	glVertexAttrib4Nub(state.vertexAttribMap[ATTRIB_COLOR], color.r, color.g, color.b, color.a);
	glVertexAttrib4f(
            state.vertexAttribMap[ATTRIB_COLOR], color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0);

	state.color = color;
}

const Color &Context::getColor() const
{
	return state.color;
}

void Context::setClearColor(const Color &color)
{
	glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
	state.clearColor = color;
}

const Color &Context::getClearColor() const
{
	return state.clearColor;
}

void Context::setPointSize(float size)
{
	state.pointSize = size;
}

float Context::getPointSize() const
{
	return state.pointSize;
}

float Context::getMaxPointSize() const
{
	return state.maxPointSize;
}


void Context::setViewport(const Context::Viewport &v)
{
	glViewport(v.x, v.y, v.width, v.height);
	state.viewport = v;
}

void Context::setViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	setViewport(Viewport(x, y, width, height));
}

const Context::Viewport &Context::getViewport() const
{
	return state.viewport;
}

void Context::setBlendState(const Context::BlendState &s)
{
	const BlendState &cur_s = state.blend;

	if (s.function != cur_s.function)
	{
		glBlendEquation(s.function);
	}

	if (s.src_rgb != cur_s.src_rgb || s.src_a != cur_s.src_a || s.dst_rgb != cur_s.dst_rgb || s.dst_a != cur_s.dst_a)
	{
		if (s.src_rgb == s.src_a && s.dst_rgb == s.dst_a)
			glBlendFunc(s.src_rgb, s.dst_rgb);
		else
			glBlendFuncSeparate(s.src_rgb, s.dst_rgb, s.src_a, s.dst_a);
	}

	state.blend = s;
}

void Context::setBlendState(GLenum function, GLenum source, GLenum destination)
{
	BlendState b = {function, source, destination};
	setBlendState(b);
}

const Context::BlendState &Context::getBlendState() const
{
	return state.blend;
}

bool Context::isGenericVertexAttrib(unsigned int attrib) const
{
	return true;
}

GLint Context::getVertexAttribID(unsigned int attrib) const
{
	std::map<unsigned int, GLenum>::const_iterator glattrib = state.vertexAttribMap.find(attrib);
	if (glattrib == state.vertexAttribMap.end())
		return -1;

	return glattrib->second;
}

void Context::setVertexAttribArray(unsigned int attrib, bool enable)
{
	if (attrib == ATTRIB_NONE)
		return;

	// is this attribute already set to the state we want?
	std::map<unsigned int, bool>::const_iterator attribenabled = state.enabledVertexAttribArrays.find(attrib);
	if (attribenabled != state.enabledVertexAttribArrays.end() && attribenabled->second == enable)
		return;

	// get the internal OpenGL representation for this attribute, if it exists
	GLint glattrib = getVertexAttribID(attrib);
	if (glattrib < 0)
		return;

	if (enable)
		glEnableVertexAttribArray(glattrib);
	else
		glDisableVertexAttribArray(glattrib);

	// save the attribute's state
	state.enabledVertexAttribArrays[attrib] = enable;
}

void Context::useVertexAttribArrays(unsigned int attribs)
{
	for (int i = 0; i < 3; i++)
	{
		unsigned int attrib = 1u << i;
		bool enable = (attribs & attrib) != 0;
		setVertexAttribArray(attrib, enable);
	}
}

void Context::vertexAttribPointer(Context::VertexAttribType attrib, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) const
{
	// get the internal OpenGL representation for this attribute, if it exists
	GLint glattrib = getVertexAttribID(attrib);
	if (glattrib < 0)
		return;

	// TODO: better check for normalization?
	GLboolean normalize = (type == GL_UNSIGNED_BYTE) ? GL_TRUE : GL_FALSE;
	glVertexAttribPointer(glattrib, size, type, normalize, stride, pointer);
}

GLuint Context::getDefaultFramebuffer() const
{
	return state.defaultFramebuffer;
}

void Context::setActiveTextureUnit(int textureunit)
{
	if (textureunit < 0 || (size_t) textureunit >= state.textureUnits.size())
		throw love::Exception("Invalid texture unit index (%d).", textureunit);

	if (textureunit != state.curTextureUnit)
	{
		if (shadersSupported)
			glActiveTexture(GL_TEXTURE0 + textureunit);
		else
			throw love::Exception("Multitexturing not supported.");
	}

	state.curTextureUnit = textureunit;
}

void Context::bindTexture(GLuint texture)
{
	if (texture != state.textureUnits[state.curTextureUnit])
	{
		state.textureUnits[state.curTextureUnit] = texture;
		glBindTexture(GL_TEXTURE_2D, texture);
	}
}

void Context::bindTextureToUnit(GLuint texture, int textureunit, bool restoreprev)
{
	if (textureunit < 0 || (size_t) textureunit >= state.textureUnits.size())
		throw love::Exception("Invalid texture unit index.");

	if (texture != state.textureUnits[textureunit])
	{
		int oldtextureunit = state.curTextureUnit;
		setActiveTextureUnit(textureunit);

		state.textureUnits[textureunit] = texture;
		glBindTexture(GL_TEXTURE_2D, texture);

		if (restoreprev)
			setActiveTextureUnit(oldtextureunit);
	}
}

void Context::deleteTexture(GLuint texture)
{
	// glDeleteTextures binds textureid 0 to all texture units the deleted texture was bound to
	std::vector<GLuint>::iterator it;
	for (it = state.textureUnits.begin(); it != state.textureUnits.end(); ++it)
	{
		if (*it == texture)
			*it = 0;
	}

	glDeleteTextures(1, &texture);
}

float Context::setTextureFilter(const graphics::Image::Filter &f) const
{
#if 0
	GLint gmin, gmag;

	if (f.mipmap == Image::FILTER_NONE)
	{
		if (f.min == Image::FILTER_NEAREST)
			gmin = GL_NEAREST;
		else // f.min == Image::FILTER_LINEAR
			gmin = GL_LINEAR;
	}
	else
	{
		if (f.min == Image::FILTER_NEAREST && f.mipmap == Image::FILTER_NEAREST)
			gmin = GL_NEAREST_MIPMAP_NEAREST;
		else if (f.min == Image::FILTER_NEAREST && f.mipmap == Image::FILTER_LINEAR)
			gmin = GL_NEAREST_MIPMAP_LINEAR;
		else if (f.min == Image::FILTER_LINEAR && f.mipmap == Image::FILTER_NEAREST)
			gmin = GL_LINEAR_MIPMAP_NEAREST;
		else if (f.min == Image::FILTER_LINEAR && f.mipmap == Image::FILTER_LINEAR)
			gmin = GL_LINEAR_MIPMAP_LINEAR;
		else
			gmin = GL_LINEAR;
	}

	switch (f.mag)
	{
	case Image::FILTER_NEAREST:
		gmag = GL_NEAREST;
		break;
	case Image::FILTER_LINEAR:
	default:
		gmag = GL_LINEAR;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gmin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gmag);

	float anisotropy = 1.0f;

	if (GLEW_EXT_texture_filter_anisotropic)
	{
		anisotropy = std::min(std::max(f.anisotropy, 1.0f), maxAnisotropy);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
	}

	return anisotropy;
#endif
        return 1.0f;
}

graphics::Image::Filter Context::getTextureFilter() const
{
#if 0
	GLint gmin, gmag;
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &gmin);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &gmag);

	Image::Filter f;

	switch (gmin)
	{
	case GL_NEAREST:
		f.min = Image::FILTER_NEAREST;
		f.mipmap = Image::FILTER_NONE;
		break;
	case GL_NEAREST_MIPMAP_NEAREST:
		f.min = f.mipmap = Image::FILTER_NEAREST;
		break;
	case GL_NEAREST_MIPMAP_LINEAR:
		f.min = Image::FILTER_NEAREST;
		f.mipmap = Image::FILTER_LINEAR;
		break;
	case GL_LINEAR_MIPMAP_NEAREST:
		f.min = Image::FILTER_LINEAR;
		f.mipmap = Image::FILTER_NEAREST;
		break;
	case GL_LINEAR_MIPMAP_LINEAR:
		f.min = f.mipmap = Image::FILTER_LINEAR;
		break;
	case GL_LINEAR:
	default:
		f.min = Image::FILTER_LINEAR;
		f.mipmap = Image::FILTER_NONE;
		break;
	}

	switch (gmag)
	{
	case GL_NEAREST:
		f.mag = Image::FILTER_NEAREST;
		break;
	case GL_LINEAR:
	default:
		f.mag = Image::FILTER_LINEAR;
		break;
	}

	if (GLEW_EXT_texture_filter_anisotropic)
		glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &f.anisotropy);

	return f;
#endif
	return graphics::Image::Filter();
}

void Context::setTextureWrap(const graphics::Image::Wrap &w) const
{
	GLint gs, gt;

	switch (w.s)
	{
	case Image::WRAP_CLAMP:
		gs = GL_CLAMP_TO_EDGE;
		break;
	case Image::WRAP_REPEAT:
	default:
		gs = GL_REPEAT;
		break;
	}

	switch (w.t)
	{
	case Image::WRAP_CLAMP:
		gt = GL_CLAMP_TO_EDGE;
		break;
	case Image::WRAP_REPEAT:
	default:
		gt = GL_REPEAT;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gs);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gt);
}

graphics::Image::Wrap Context::getTextureWrap() const
{
	GLint gs, gt;

	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &gs);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &gt);

	Image::Wrap w;

	switch (gs)
	{
	case GL_CLAMP_TO_EDGE:
		w.s = Image::WRAP_CLAMP;
		break;
	case GL_REPEAT:
	default:
		w.s = Image::WRAP_REPEAT;
		break;
	}

	switch (gt)
	{
	case GL_CLAMP_TO_EDGE:
		w.t = Image::WRAP_CLAMP;
		break;
	case GL_REPEAT:
	default:
		w.t = Image::WRAP_REPEAT;
		break;
	}

	return w;
}

} // gles2
} // graphics
} // love
