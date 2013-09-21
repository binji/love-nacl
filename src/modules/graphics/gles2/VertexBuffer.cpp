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

#include "VertexBuffer.h"

#include "common/Exception.h"
#include <common/config.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace love
{
namespace graphics
{
namespace gles2
{
	// VertexBuffer

	VertexBuffer *VertexBuffer::Create(size_t size, GLenum target, GLenum usage)
	{
		return new VBO(size, target, usage);
	}

	VertexBuffer::VertexBuffer(size_t size, GLenum target, GLenum usage)
		: size(size)
		, target(target)
		, usage(usage)
	{
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	VBO::VBO(size_t size, GLenum target, GLenum usage)
		: VertexBuffer(size, target, usage)
		, vbo(0)
		, buffer_copy(0)
		, mapped(0)
	{
		bool ok = load(false);

		if (!ok)
			throw love::Exception("Could not load VBO.");
	}

	VBO::~VBO()
	{
		if (vbo != 0)
			unload(false);
	}

	void *VBO::map()
	{
		// mapping twice could result in memory leaks
		if (mapped)
			throw love::Exception("VBO is already mapped!");

		mapped = malloc(getSize());
		if (!mapped)
			throw love::Exception("Out of memory (oh the humanity!)");
                // TODO(binji): implement
		// glGetBufferSubData(getTarget(), 0, getSize(), mapped);
                memset(mapped, 0, getSize());

		return mapped;
	}

	void VBO::unmap()
	{
		glBufferSubData(getTarget(), 0, getSize(), mapped);
		free(mapped);
		mapped = 0;
	}

	void VBO::bind()
	{
		glBindBuffer(getTarget(), vbo);
	}

	void VBO::unbind()
	{
		glBindBuffer(getTarget(), 0);
	}

	void VBO::fill(size_t offset, size_t size, const void *data)
	{
		if (mapped)
			memcpy(static_cast<char*>(mapped) + offset, data, size);
		else
			glBufferSubData(getTarget(), offset, size, data);
	}

	const void *VBO::getPointer(size_t offset) const
	{
		return reinterpret_cast<const void*>(offset);
	}

	bool VBO::loadVolatile()
	{
		return load(true);
	}

	void VBO::unloadVolatile()
	{
		unload(true);
	}

	bool VBO::load(bool restore)
	{
		glGenBuffers(1, &vbo);

		VertexBuffer::Bind bind(*this);

		// Copy the old buffer only if 'restore' was requested.
		const GLvoid *src = restore ? buffer_copy : 0;

		// NOTE(binji): glGetError is very slow on NaCl.
#if 0
		while (GL_NO_ERROR != glGetError())
			/* clear error messages */;
#endif

		// Note that if 'src' is '0', no data will be copied.
		glBufferData(getTarget(), getSize(), src, getUsage());
#if 0
		GLenum err = glGetError();
#else
		GLenum err = GL_NO_ERROR;
#endif

		// Clean up buffer_copy, if it exists.
		delete[] buffer_copy;
		buffer_copy = 0;

		return (GL_NO_ERROR == err);
	}

	void VBO::unload(bool save)
	{
		// Clean up buffer_copy, if it exists.
		delete[] buffer_copy;
		buffer_copy = 0;

		// Save data before unloading.
		if (save)
		{
			VertexBuffer::Bind bind(*this);

			GLint size;
			glGetBufferParameteriv(getTarget(), GL_BUFFER_SIZE, &size);

			const char *src = static_cast<char *>(map());

			if (src)
			{
				buffer_copy = new char[size];
				memcpy(buffer_copy, src, size);
				unmap();
			}
		}

		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}

	// VertexIndex
	
	size_t VertexIndex::maxSize = 0;
	size_t VertexIndex::elementSize = 0;
	std::list<size_t> VertexIndex::sizeRefs;
	VertexBuffer *VertexIndex::element_array = NULL;
	
	VertexIndex::VertexIndex(size_t size)
		: size(size)
	{
		// The upper limit is the maximum of GLuint divided by six (the number
		// of indices per size) and divided by the size of GLuint. This guarantees
		// no overflows when calculating the array size in bytes.
		// Memory issues will be handled by other exceptions.
		if (size == 0 || size > ((GLuint) -1) / 6 / sizeof(GLuint))
			throw love::Exception("Invalid size.");
	
		addSize(size);
	}
	
	VertexIndex::~VertexIndex()
	{
		removeSize(size);
	}
	
	size_t VertexIndex::getSize() const
	{
		return size;
	}
	
	size_t VertexIndex::getIndexCount(size_t elements) const
	{
		return elements * 6;
	}
	
	GLenum VertexIndex::getType(size_t s) const
	{
#if 0
		// OpenGL ES 2.0 doesn't guarantee support for unsigned int vertex indices.
		if (GLEW_ES_VERSION_2_0 && !GLEW_OES_element_index_uint)
			return GL_UNSIGNED_SHORT;
	
		// Calculates if unsigned short is big enough to hold all the vertex indices.
		static const GLenum type_table[] = {GL_UNSIGNED_SHORT, GL_UNSIGNED_INT};
		return type_table[s * 4 > std::numeric_limits<GLushort>::max()];
		// if buffer-size > max(GLushort) then GL_UNSIGNED_INT else GL_UNSIGNED_SHORT
#endif
		return GL_UNSIGNED_SHORT;
	}
	
	size_t VertexIndex::getElementSize()
	{
		return elementSize;
	}
	
	VertexBuffer *VertexIndex::getVertexBuffer() const
	{
		return element_array;
	}
	
	const void *VertexIndex::getPointer(size_t offset) const
	{
		return element_array->getPointer(offset);
	}
	
	void VertexIndex::addSize(size_t newSize)
	{
		if (newSize <= maxSize)
		{
			// Current size is bigger. Append the size to list and sort.
			sizeRefs.push_back(newSize);
			sizeRefs.sort();
			return;
		}
	
		// Try to resize before adding it to the list because resize may throw.
		resize(newSize);
		sizeRefs.push_back(newSize);
	}
	
	void VertexIndex::removeSize(size_t oldSize)
	{
		// TODO: For debugging purposes, this should check if the size was actually found.
		sizeRefs.erase(std::find(sizeRefs.begin(), sizeRefs.end(), oldSize));
		if (sizeRefs.size() == 0)
		{
			resize(0);
			return;
		}
	
		if (oldSize == maxSize)
		{
			// Shrink if there's a smaller size.
			size_t newSize = sizeRefs.back();
			if (newSize < maxSize)
				resize(newSize);
		}
	}
	
	void VertexIndex::resize(size_t size)
	{
		if (size == 0)
		{
			delete element_array;
			element_array = NULL;
			maxSize = 0;
			return;
		}
	
		VertexBuffer *new_element_array;
	
		// Depending on the size, a switch to int and more memory is needed.
		GLenum target_type = getType(size);
		size_t elem_size = (target_type == GL_UNSIGNED_SHORT) ? sizeof(GLushort) : sizeof(GLuint);
	
		size_t array_size = elem_size * 6 * size;
	
		// Create may throw out-of-memory exceptions.
		// VertexIndex will propagate the exception and keep the old VertexBuffer.
		try
		{
			new_element_array = VertexBuffer::Create(array_size, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
		}
		catch (std::bad_alloc &)
		{
			throw love::Exception("Out of memory.");
		}
	
		// Allocation of the new VertexBuffer succeeded.
		// The old VertexBuffer can now be deleted.
		delete element_array;
		element_array = new_element_array;
		maxSize = size;
		elementSize = elem_size;
	
		switch (target_type)
		{
		case GL_UNSIGNED_SHORT:
			fill<GLushort>();
			break;
		case GL_UNSIGNED_INT:
			fill<GLuint>();
			break;
		}
	}
	
	template <typename T>
	void VertexIndex::fill()
	{
		VertexBuffer::Bind bind(*element_array);
		VertexBuffer::Mapper mapper(*element_array);
	
		T *indices = (T *) mapper.get();
	
		for (size_t i = 0; i < maxSize; ++i)
		{
			indices[i*6+0] = i * 4 + 0;
			indices[i*6+1] = i * 4 + 1;
			indices[i*6+2] = i * 4 + 2;
	
			indices[i*6+3] = i * 4 + 0;
			indices[i*6+4] = i * 4 + 2;
			indices[i*6+5] = i * 4 + 3;
		}
	}

} // gles2
} // graphics
} // love
