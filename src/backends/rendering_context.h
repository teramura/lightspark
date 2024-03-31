/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2011-2013  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef BACKENDS_RENDERING_CONTEXT_H
#define BACKENDS_RENDERING_CONTEXT_H 1

#include "forwards/scripting/flash/display/DisplayObject.h"
#include "forwards/scripting/flash/geom/flashgeom.h"
#include <stack>
#include "threading.h"
#include "backends/graphics.h"

namespace lightspark
{

enum VertexAttrib { VERTEX_ATTRIB=0, COLOR_ATTRIB, TEXCOORD_ATTRIB};

class Rectangle;
class EngineData;

struct Transform2D
{
	MATRIX matrix;
	ColorTransformBase colorTransform;
	AS_BLENDMODE blendmode;
	
	Transform2D(const MATRIX& _matrix, const ColorTransformBase& _colorTransform,AS_BLENDMODE _blendmode) : matrix(_matrix), colorTransform(_colorTransform),blendmode(_blendmode)
	{
	}
	Transform2D():blendmode(AS_BLENDMODE::BLENDMODE_NORMAL)
	{
	}
	~Transform2D() {}
};

class TransformStack
{
private:
	std::vector<Transform2D> transforms;
public:
	TransformStack() {}
	~TransformStack() {}
	void push(const Transform2D& transform);
	void pop() { transforms.pop_back(); }
	Transform2D& transform() { return transforms.back(); }
	Transform2D& frontTransform() { return transforms.front(); }
	const Transform2D& transform() const { return transforms.back(); }
	const Transform2D& frontTransform() const { return transforms.front(); }
};

/*
 * The RenderContext contains all (public) functions that are needed by DisplayObjects to draw themselves.
 */
class RenderContext
{
protected:
	/* Modelview matrix manipulation */
	static const float lsIdentityMatrix[16];
	float lsMVPMatrix[16];
	std::stack<float*> lsglMatrixStack;
	~RenderContext(){}
	void lsglMultMatrixf(const float *m);
	std::vector<TransformStack> transformStacks;
public:
	enum CONTEXT_TYPE { CAIRO=0, GL };
	RenderContext(CONTEXT_TYPE t,DisplayObject* startobj=nullptr);
	CONTEXT_TYPE contextType;
	const DisplayObject* currentMask;
	AS_BLENDMODE currentShaderBlendMode;
	DisplayObject* startobject;
	TransformStack& transformStack()
	{
		if (transformStacks.empty())
			createTransformStack();
		return transformStacks.back();
	}
	TransformStack& frontTransformStack()
	{
		if (transformStacks.empty())
			createTransformStack();
		return transformStacks.front();
	}
	void createTransformStack() { transformStacks.push_back(TransformStack()); }
	void removeTransformStack() { transformStacks.pop_back(); }

	/* Modelview matrix manipulation */
	void lsglLoadIdentity();
	void lsglLoadMatrixf(const float *m);
	void lsglScalef(float scaleX, float scaleY, float scaleZ);
	void lsglTranslatef(float translateX, float translateY, float translateZ);
	void lsglRotatef(float angle);

	enum COLOR_MODE { RGB_MODE=0, YUV_MODE };
	enum MASK_MODE { NO_MASK = 0, ENABLE_MASK };
	/* Textures */
	/**
		Render a quad of given size using the given chunk
	*/
	virtual void renderTextured(const TextureChunk& chunk, float alpha, COLOR_MODE colorMode,
			const ColorTransformBase& colortransform,
			bool isMask, float directMode, RGB directColor,SMOOTH_MODE smooth, const MATRIX& matrix,
			Rectangle* scalingGrid, AS_BLENDMODE blendmode)=0;
	/**
	 * Get the right CachedSurface from an object
	 */
	virtual const CachedSurface& getCachedSurface(const DisplayObject* obj) const=0;
	virtual void pushMask()=0;
	virtual void popMask()=0;
	virtual void deactivateMask()=0;
	virtual void activateMask()=0;
	virtual void suspendActiveMask()=0;
	virtual void resumeActiveMask()=0;
	virtual bool isDrawingMask() const=0;
	virtual bool isMaskActive() const=0;
};
struct filterstackentry
{
	uint32_t filterframebuffer;
	uint32_t filterrenderbuffer;
	uint32_t filtertextureID;
	number_t filterborderx;
	number_t filterbordery;
};

class GLRenderContext: public RenderContext
{
private:
	static int errorCount;
	int maskCount;
	bool inMaskRendering;
	bool maskActive;
protected:
	EngineData* engineData;
	int projectionMatrixUniform;
	int modelviewMatrixUniform;

	int yuvUniform;
	int alphaUniform;
	int maskUniform;
	int isFirstFilterUniform;
	int colortransMultiplyUniform;
	int colortransAddUniform;
	int directUniform;
	int directColorUniform;
	int blendModeUniform;
	int filterdataUniform;
	int gradientcolorsUniform;

	/* Textures */
	Mutex mutexLargeTexture;
	uint32_t largeTextureSize;
	class LargeTexture
	{
	public:
		uint32_t id;
		uint8_t* bitmap;
		LargeTexture(uint8_t* b):id(-1),bitmap(b){}
		~LargeTexture(){/*delete[] bitmap;*/}
	};
	std::vector<LargeTexture> largeTextures;

	~GLRenderContext(){}
	void renderpart(const MATRIX& matrix, const TextureChunk& chunk, float cropleft, float croptop, float cropwidth, float cropheight, float tx, float ty);
public:
	enum LSGL_MATRIX {LSGL_PROJECTION=0, LSGL_MODELVIEW};
	/*
	 * Uploads the current matrix as the specified type.
	 */
	void setMatrixUniform(LSGL_MATRIX m) const;
	GLRenderContext() : RenderContext(GL),maskCount(0),inMaskRendering(false),maskActive(false),engineData(nullptr), largeTextureSize(0)
	{
	}
	void SetEngineData(EngineData* data) { engineData = data;}
	void lsglOrtho(float l, float r, float b, float t, float n, float f);

	void renderTextured(const TextureChunk& chunk, float alpha, COLOR_MODE colorMode,
			const ColorTransformBase& colortransform,
			bool isMask, float directMode, RGB directColor, SMOOTH_MODE smooth, const MATRIX& matrix,
			Rectangle* scalingGrid, AS_BLENDMODE blendmode) override;
	/**
	 * Get the right CachedSurface from an object
	 * In the OpenGL case we just get the CachedSurface inside the object itself
	 */
	const CachedSurface& getCachedSurface(const DisplayObject* obj) const override;

	void pushMask() override;
	void popMask() override;
	void deactivateMask() override;
	void activateMask() override;
	void suspendActiveMask() override;
	void resumeActiveMask() override;
	bool isDrawingMask() const override { return inMaskRendering; }
	bool isMaskActive() const override { return maskActive; }

	void resetCurrentFrameBuffer();
	void setupRenderingState(float alpha, const ColorTransformBase& colortransform, SMOOTH_MODE smooth, AS_BLENDMODE blendmode);
	// this is used to keep track of the fbos when rendering filters and some of the ancestors of the filtered object also have filters
	std::vector<filterstackentry> filterframebufferstack;

	/* Utility */
	bool handleGLErrors() const;
};

class CairoRenderContext: public RenderContext
{
private:
	std::map<const DisplayObject*, CachedSurface> customSurfaces;
	cairo_t* cr;
	std::list<std::pair<cairo_surface_t*,MATRIX>> masksurfaces;
	static cairo_surface_t* getCairoSurfaceForData(uint8_t* buf, uint32_t width, uint32_t height, uint32_t stride);
	/*
	 * An invalid surface to be returned for objects with no content
	 */
	static const CachedSurface invalidSurface;
public:
	CairoRenderContext(uint8_t* buf, uint32_t width, uint32_t height,bool smoothing,DisplayObject* startobj=nullptr);
	virtual ~CairoRenderContext();
	void renderTextured(const TextureChunk& chunk, float alpha, COLOR_MODE colorMode,
			const ColorTransformBase& colortransform,
			bool isMask, float directMode, RGB directColor, SMOOTH_MODE smooth, const MATRIX& matrix,
			Rectangle* scalingGrid, AS_BLENDMODE blendmode) override;
	/**
	 * Get the right CachedSurface from an object
	 * In the Cairo case we get the right CachedSurface out of the map
	 */
	const CachedSurface& getCachedSurface(const DisplayObject* obj) const override;

	void pushMask() override {}
	void popMask() override {}
	void deactivateMask() override {}
	void activateMask() override {}
	void suspendActiveMask() override {}
	void resumeActiveMask() override {}
	bool isDrawingMask() const override { return false; }
	bool isMaskActive() const override { return false; }

	/**
	 * The CairoRenderContext acquires the ownership of the buffer
	 * it will be freed on destruction
	 */
	CachedSurface& allocateCustomSurface(const DisplayObject* d, uint8_t* texBuf, bool isBufferOwner);
	/**
	 * Do a fast non filtered, non scaled blit of ARGB data
	 */
	void simpleBlit(int32_t destX, int32_t destY, uint8_t* sourceBuf, uint32_t sourceTotalWidth, uint32_t sourceTotalHeight,
			int32_t sourceX, int32_t sourceY, uint32_t sourceWidth, uint32_t sourceHeight);
	/**
	 * Do an optionally filtered blit with transformation
	 */
	enum FILTER_MODE { FILTER_NONE = 0, FILTER_SMOOTH };
	void transformedBlit(const MATRIX& m, BitmapContainer* bc, ColorTransform* ct,
			FILTER_MODE filterMode, number_t x, number_t y, number_t w, number_t h);
};

}
#endif /* BACKENDS_RENDERING_CONTEXT_H */
