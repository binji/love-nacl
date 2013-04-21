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
#include <common/config.h>
#include "Font.h"
#include <font/GlyphData.h>
#include "Context.h"
#include "Quad.h"

#include <libraries/utf8/utf8.h>

#include <common/math.h>
#include <common/Matrix.h>
#include <math.h>

#include <sstream>

#include <algorithm> // for max

namespace love
{
namespace graphics
{
namespace gles2
{

	Font::Font(love::font::Rasterizer * r, const Image::Filter& filter)
	: rasterizer(r), elementBuffer(0), height(r->getHeight()), lineHeight(1), mSpacing(1), filter(filter)
	{
		r->retain();

		love::font::GlyphData *gd = 0;
		try
		{
			gd = r->getGlyphData(32);
			type = (gd->getFormat() == love::font::GlyphData::FORMAT_LUMINANCE_ALPHA ? FONT_TRUETYPE : FONT_IMAGE);

			// pre-allocate the element buffer with a small amount of space (can grow later)
			elementBuffer = new VertexIndex(100);

			loadVolatile();
		}
		catch (love::Exception &)
		{
			delete gd;
			delete elementBuffer;
			throw;
		}

		delete gd;
	}

	Font::~Font()
	{
		rasterizer->release();
		unloadVolatile();
	}

	void Font::createTexture()
	{
		texture_x = texture_y = rowHeight = TEXTURE_PADDING;

		GLuint t;
		glGenTextures(1, &t);
		textures.push_back(t);

		Context *ctx = getContext();

		ctx->bindTexture(t);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLint format = (type == FONT_TRUETYPE ? GL_LUMINANCE_ALPHA : GL_RGBA);

		glTexImage2D(GL_TEXTURE_2D,
					 0,
					 format,
					 (GLsizei)TEXTURE_WIDTH,
					 (GLsizei)TEXTURE_HEIGHT,
					 0,
					 format,
					 GL_UNSIGNED_BYTE,
					 NULL);

		// Fill the texture with transparent black
		std::vector<GLubyte> emptyData(TEXTURE_WIDTH * TEXTURE_HEIGHT * (type == FONT_TRUETYPE ? 2 : 4), 0);
		glTexSubImage2D(GL_TEXTURE_2D,
						0,
						0, 0,
						(GLsizei)TEXTURE_WIDTH,
						(GLsizei)TEXTURE_HEIGHT,
						format,
						GL_UNSIGNED_BYTE,
						&emptyData[0]);

//		setFilter(filter);
	}

	Font::Glyph * Font::addGlyph(int glyph)
	{
		love::font::GlyphData *gd = rasterizer->getGlyphData(glyph);
		int w = gd->getWidth();
		int h = gd->getHeight();

		if (texture_x + w + TEXTURE_PADDING > TEXTURE_WIDTH)
		{
			// out of space - new row!
			texture_x = TEXTURE_PADDING;
			texture_y += rowHeight;
			rowHeight = TEXTURE_PADDING;
		}
		if (texture_y + h + TEXTURE_PADDING > TEXTURE_HEIGHT)
		{
			// totally out of space - new texture!
			createTexture();
		}

		Glyph *g = new Glyph;

		g->texture = 0;
		g->spacing = gd->getAdvance();

		memset(&g->quad, 0, sizeof(GlyphQuad));

		// don't waste space for empty glyphs. also fixes a division by zero bug with ati drivers
		if (w > 0 && h > 0)
		{
			const GLuint t = textures.back();

			getContext()->bindTexture(t);
			glTexSubImage2D(GL_TEXTURE_2D,
							0,
							texture_x,
							texture_y,
							w, h,
							(type == FONT_TRUETYPE ? GL_LUMINANCE_ALPHA : GL_RGBA),
							GL_UNSIGNED_BYTE,
							gd->getData());

//			setFilter(filter);

			g->texture = t;

			Quad::Viewport v;
			v.x = (float) texture_x;
			v.y = (float) texture_y;
			v.w = (float) w;
			v.h = (float) h;

			Quad q = Quad(v, (const float) TEXTURE_WIDTH, (const float) TEXTURE_HEIGHT);
			const vertex *verts = q.getVertices();

			// copy vertex data to the glyph and set proper bearing
			for (int i = 0; i < 4; i++)
			{
				g->quad.vertices[i] = verts[i];
				g->quad.vertices[i].x += gd->getBearingX();
				g->quad.vertices[i].y -= gd->getBearingY();
			}
		}

		if (w > 0)
			texture_x += (w + TEXTURE_PADDING);
		if (h > 0)
			rowHeight = std::max(rowHeight, h + TEXTURE_PADDING);

		delete gd;

		glyphs[glyph] = g;

		return g;
	}

	Font::Glyph *Font::findGlyph(unsigned int glyph)
	{
		Glyph *g = glyphs[glyph];
		if (!g)
			g = addGlyph(glyph);

		return g;
	}


	float Font::getHeight() const
	{
		return static_cast<float>(height);
	}

	void Font::print(std::string text, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
	{
		float dx = 0.0f; // spacing counter for newline handling
		float dy = 0.0f;

		Context *ctx = getContext();

		// keeps track of when we need to switch textures in our vertex array
		std::vector<GlyphArrayDrawInfo> glyphinfolist;

		std::vector<GlyphQuad> glyphquads;
		glyphquads.reserve(text.size()); // pre-allocate space for the maximum possible number of quads

		int quadindex = 0;

		ctx->modelViewStack.push(ctx->modelViewStack.top());

		static Matrix t;
		t.setTransformation(ceil(x), ceil(y), angle, sx, sy, ox, oy, kx, ky);
		ctx->modelViewStack.top() *= t;

		try
		{
			utf8::iterator<std::string::const_iterator> i(text.begin(), text.begin(), text.end());
			utf8::iterator<std::string::const_iterator> end(text.end(), text.begin(), text.end());

			while (i != end)
			{
				unsigned int g = *i++;

				if (g == '\n')
				{
					// wrap newline, but do not print it
					dy += floor(getHeight() * getLineHeight() + 0.5f);
					dx = 0.0f;
					continue;
				}

				Glyph *glyph = findGlyph(g);

				// we only care about the vertices of glyphs which have a texture
				if (glyph->texture != 0)
				{
					// copy glyphquad (4 vertices) from original glyph to our current quad list
					glyphquads.push_back(glyph->quad);

					float lineheight = getBaseline();

					// set proper relative position
					for (int i = 0; i < 4; i++)
					{
						glyphquads[quadindex].vertices[i].x += dx;
						glyphquads[quadindex].vertices[i].y += dy + lineheight;
					}

					size_t listsize = glyphinfolist.size();

					// check if current glyph texture has changed since the previous iteration
					if (listsize == 0 || glyphinfolist[listsize-1].texture != glyph->texture)
					{
						// keep track of each sub-section of the string whose glyphs use different textures than the previous section
						GlyphArrayDrawInfo glyphdrawinfo;
						glyphdrawinfo.startQuad = quadindex;
						glyphdrawinfo.numQuads = 0;
						glyphdrawinfo.texture = glyph->texture;
						glyphinfolist.push_back(glyphdrawinfo);
					}

					++quadindex;
					++glyphinfolist[glyphinfolist.size()-1].numQuads;
				}

				// advance the x position for the next glyph
//				dx += glyph->spacing + letter_spacing;
				dx += glyph->spacing;
			}
		}
		catch (love::Exception &)
		{
			ctx->modelViewStack.pop();
			throw;
		}
		catch (utf8::exception &e)
		{
			ctx->modelViewStack.pop();
			throw love::Exception("%s", e.what());
		}

		if (quadindex > 0 && glyphinfolist.size() > 0)
		{
			// Resize the element index buffer to fit the string size, if necessary
			if (elementBuffer == 0 || (size_t) quadindex > elementBuffer->getSize())
			{
				VertexIndex *newelementbuffer = 0;
				try
				{
					newelementbuffer = new VertexIndex(quadindex);
				}
				catch (love::Exception &)
				{
					ctx->modelViewStack.pop();
					throw;
				}
				if (newelementbuffer)
				{
					delete elementBuffer;
					elementBuffer = newelementbuffer;
				}
			}

			// bind the index buffer for use with glDrawElements
			VertexBuffer::Bind indexbind(*elementBuffer->getVertexBuffer());

			// sort glyph draw info list by texture first, and quad position in memory second (using the struct's < operator)
			std::sort(glyphinfolist.begin(), glyphinfolist.end());

			ctx->useVertexAttribArrays(Context::ATTRIB_VERTEX | Context::ATTRIB_TEXCOORD);

			ctx->vertexAttribPointer(Context::ATTRIB_VERTEX, 2, GL_FLOAT, sizeof(vertex), (GLvoid *)&glyphquads[0].vertices[0].x);
			ctx->vertexAttribPointer(Context::ATTRIB_TEXCOORD, 2, GL_FLOAT, sizeof(vertex), (GLvoid *)&glyphquads[0].vertices[0].s);

			ctx->setupRender();

			// we need to draw a new vertex array for every section of the string that uses a different texture than the previous section
			std::vector<GlyphArrayDrawInfo>::const_iterator it;
			for (it = glyphinfolist.begin(); it != glyphinfolist.end(); ++it)
			{
				ctx->bindTexture(it->texture);

				size_t numelements = elementBuffer->getIndexCount(it->numQuads);
				size_t elementoffset = elementBuffer->getIndexCount(it->startQuad) * elementBuffer->getElementSize();

				glDrawElements(GL_TRIANGLES, numelements, elementBuffer->getType(), elementBuffer->getPointer(elementoffset));
			}
		}

		ctx->modelViewStack.pop();
	}

	void Font::print(char character, float x, float y)
	{
		print(std::string(1, character), x, y);
	}

	int Font::getWidth(const std::string & line)
	{
		if (line.size() == 0) return 0;
		int temp = 0;

		Glyph * g;

		try
		{
			utf8::iterator<std::string::const_iterator> i (line.begin(), line.begin(), line.end());
			utf8::iterator<std::string::const_iterator> end (line.end(), line.begin(), line.end());
			while (i != end)
			{
				int c = *i++;
				g = glyphs[c];
				if (!g) g = addGlyph(c);
				temp += static_cast<int>(g->spacing * mSpacing);
			}
		}
		catch (utf8::exception & e)
		{
			throw love::Exception("%s", e.what());
		}

		return temp;
	}

	int Font::getWidth(const char * line)
	{
		return this->getWidth(std::string(line));
	}

	int Font::getWidth(const char character)
	{
		Glyph * g = glyphs[character];
		if (!g) g = addGlyph(character);
		return g->spacing;
	}

	std::vector<std::string> Font::getWrap(const std::string text, float wrap, int * max_width)
	{
		using namespace std;
		const float width_space = static_cast<float>(getWidth(' '));
		vector<string> lines_to_draw;
		int maxw = 0;

		//split text at newlines
		istringstream iss( text );
		string line;
		ostringstream string_builder;
		while (getline(iss, line, '\n'))
		{
			// split line into words
			vector<string> words;
			istringstream word_iss(line);
			copy(istream_iterator<string>(word_iss), istream_iterator<string>(),
					back_inserter< vector<string> >(words));

			// put words back together until a wrap occurs
			float width = 0.0f;
			float oldwidth = 0.0f;
			string_builder.str("");
			vector<string>::const_iterator word_iter, wend = words.end();
			for (word_iter = words.begin(); word_iter != wend; ++word_iter)
			{
				const string& word = *word_iter;
				width += getWidth( word );

				// on wordwrap, push line to line buffer and clear string builder
				if (width >= wrap && oldwidth > 0)
				{
					int realw = (int) width;

					// remove trailing space
					string tmp = string_builder.str();
					lines_to_draw.push_back( tmp.substr(0,tmp.size()-1) );
					string_builder.str("");
					width = static_cast<float>(getWidth( word ));
					realw -= (int) width;
					if (realw > maxw)
						maxw = realw;
				}
				string_builder << word << " ";
				width += width_space;
				oldwidth = width;
			}
			// push last line
			if (width > maxw)
				maxw = (int) width;
			string tmp = string_builder.str();
			lines_to_draw.push_back( tmp.substr(0,tmp.size()-1) );
		}

		if (max_width)
			*max_width = maxw;

		return lines_to_draw;
	}

	void Font::setLineHeight(float height)
	{
		this->lineHeight = height;
	}

	float Font::getLineHeight() const
	{
		return lineHeight;
	}

	void Font::setSpacing(float amount)
	{
		mSpacing = amount;
	}

	float Font::getSpacing() const
	{
		return mSpacing;
	}

	bool Font::loadVolatile()
	{
		createTexture();
		return true;
	}

	void Font::unloadVolatile()
	{
		Context *ctx = getContext();
		
		// nuke everything from orbit
		std::map<unsigned int, Glyph *>::iterator it = glyphs.begin();
		Glyph *g;
		while (it != glyphs.end())
		{
			g = it->second;
			delete g;
			glyphs.erase(it++);
		}
		std::vector<GLuint>::iterator iter = textures.begin();
		while (iter != textures.end())
		{
			ctx->deleteTexture(*iter);
			iter++;
		}
		textures.clear();
		delete elementBuffer;
		elementBuffer = 0;
	}

	int Font::getAscent() const
	{
		return rasterizer->getAscent();
	}

	int Font::getDescent() const
	{
		return rasterizer->getDescent();
	}

	float Font::getBaseline() const
	{
		// 1.25 is magic line height for true type fonts
		return (type == FONT_TRUETYPE) ? floor(getHeight() / 1.25f + 0.5f) : 0.0f;
	}

} // gles2
} // graphics
} // love
