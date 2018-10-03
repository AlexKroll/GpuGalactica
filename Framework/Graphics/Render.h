#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <memory>

#include "../Math/Color.h"

#include "../Utils/PointRect.h"

#include "Vertex.h"
#include "Font.h"
#include "ShaderProgram.h"
#include "Mesh.h"



// Interface for graphics render.

class IRender abstract
{
public:
	enum DeviceType
	{
		DX9,
		DX11,
		GLES20,
		GLES30,
		VULKAN
	};

	// Drawing flags..
	static const uint32_t kCustomShaderProgram	 = 0x00000001;
	static const uint32_t kAlphaBlendNormal		 = 0x00000002;
	static const uint32_t kAlphaBlendAdditive	 = 0x00000004;
	static const uint32_t kTexSamplerPointClamp	 = 0x00000008;  // Texture sampler is Point-Clamp.
	static const uint32_t kTexSamplerLinearWrap  = 0x00000020;  // Texture sampler is Linear-Wrap.

	// Raster types
	static const uint32_t kRasterCullOff = 1;


	static IRender* getRender(DeviceType type);

	virtual int init(HWND hWnd, bool bWindowed) = 0;

	virtual void release() = 0;


	//  Vertex meshes..
	virtual Mesh createMesh(uint32_t sizeBytes, void* pVertices, byte vertexSize, void* pIndices, int numPrimitives, IMesh::MeshUsage* pUsage) = 0;


	//  Textures..
	virtual Texture createTexture(UINT width, UINT height, ITexture::PixelFormat format, ITexture::Type type, BYTE* pPixels = nullptr) = 0;

	virtual Texture createTexture1D(UINT width, ITexture::PixelFormat format, ITexture::Type type, BYTE* pPixels = nullptr) = 0;

	virtual Texture createDepthBuffer(UINT width, UINT height, ITexture::PixelFormat format, UINT multisamples = 0) = 0;

	virtual Texture LoadTextureFromFile(cstring filePath) = 0;

	virtual void setTexture(int slot, Texture texture) = 0;

	virtual void releaseTexture(Texture& pTexture) = 0;


	//  Fonts..
	virtual Font loadFont(cstring filePath) = 0;

	virtual void releaseFont(Font& pFont) = 0;

	virtual CFont getDefaultFont() = 0;


	//  Render targets..
	virtual int keepRenderTargetContext() = 0;

	virtual int restoreRenderTargetContext() = 0;

	virtual int setRenderTargetContext(Texture pTexture, Texture pDepth, RECT* pRect = nullptr) = 0;


	//  States..
	virtual void setEnableDepthBuffer(bool bEnabled) = 0;

	virtual void setEnableDepthWrite(bool bEnabled) = 0;

	virtual void setAlphaBlendNormal() = 0;

	virtual void setAlphaBlendAdditive() = 0;

	virtual void setAlphaBlendOff() = 0;

	virtual void setViewport(int x, int y, int w, int h) = 0;


	//  Shaders..
	virtual ShaderProgram createShaderProgram(PCCHAR pVertText, PCCHAR pPixText) = 0;

	virtual ShaderProgram createShaderProgram(Shader pVShader, PCCHAR pPixText) = 0;

	virtual ShaderProgram createShaderProgram(PCCHAR pVertText, Shader pPShader) = 0;

	virtual ShaderProgram createShaderProgram(cstring vsFile, cstring psFile, CShaderMacroStrings defines) = 0;

	virtual void setShaderProgram(ShaderProgram pProgram) = 0;

	virtual ShaderProgram getSpriteShaderProgram() = 0;


	//  Vertex data..
	virtual VertexInput createVertexInput(uint32_t vertexFlags, uint32_t instanceFlags) = 0;

	virtual void setVertexInput(VertexInput pVertexInput) = 0;


	//  Constant buffer..
	virtual ConstantBuffer createConstantBuffer(uint32_t sizeBytes) = 0;

	virtual void* openConstantBuffer(ConstantBuffer pBuffer) = 0;

	virtual void closeConstantBuffer(ConstantBuffer pBuffer) = 0;

	virtual void updateConstantBuffer(ConstantBuffer pBuffer, void* pData, int dataSize) = 0;

	virtual void setConstantBuffers(ConstantBuffer pVertexCBuffer, ConstantBuffer pPixelCBuffer) = 0;  // For GL uses only pVertexCBuffer as uniforms.

	virtual void setConstantBufferVS(ConstantBuffer pCBuffer, UINT slot) = 0;
	virtual void setConstantBufferPS(ConstantBuffer pCBuffer, UINT slot) = 0;


	//  Vertex buffer..
	virtual VertexBuffer createVertexBuffer(uint32_t sizeBytes, bool bWritable, void* pVertices, int stride) = 0;

	virtual void* openVertexBuffer(VertexBuffer pBuffer) = 0;

	virtual void closeVertexBuffer(VertexBuffer pBuffer) = 0;

	virtual void setVertexBuffer(VertexBuffer pVertexBuffer, UINT slot) = 0;


	//  Index buffer..
	virtual IndexBuffer createIndexBuffer(uint32_t sizeBytes, bool bWritable, bool bIndices32, void* pIndices) = 0;

	virtual void* openIndexBuffer(IndexBuffer pBuffer) = 0;

	virtual void closeIndexBuffer(IndexBuffer pBuffer) = 0;

	virtual void setIndexBuffer(IndexBuffer pIndexBuffer) = 0;


	//  Unordered access buffer..
	virtual UABuffer createUABuffer(int amountOfElems, int elemSize, bool bGPUlength) = 0;
	// These buffers are used by GPU computing.
	// They have arbitrary access for general calculations.
	// bGPUlength - GPU can increase/decrease the length of the buffer (push/pop). (see particle emitters shaders).
									

	//  2D drawing..
	virtual void drawSprite(CPoint pos, Texture pTexture, CRect rect, uint32_t flags) = 0;

	virtual void drawSprite(CPoint pos, Texture pTexture, CRect rect, uint32_t color, uint32_t flags) = 0;

	virtual void drawSprites(int numSprites, const Point* pPoints, const Rect* pRects, Texture pTexture, uint32_t flags) = 0;

	virtual void drawSprites(int numSprites, const Point* pPoints, const Rect* pRects, Texture pTexture, uint32_t color, uint32_t flags) = 0;


	//  Indexed triangles drawing..
	virtual void drawIndexedTriangles(int numTriangles) = 0;

	virtual void drawIndexedTrianglesInstanced(int numTriangles, int numInstances) = 0;


	//  Misc.
	void getScreenSize(int& width, int& height);

	virtual void setDefaultSampler(int slot) = 0;

	virtual void setDefaultRasterState() = 0;

	virtual void setRasterState(uint32_t rasterType) = 0;


	virtual void prepareDrawing() = 0;

	virtual void presentDrawing() = 0;

protected:
	IRender() {}

public:
	virtual ~IRender() {}

protected:
	HWND hWindow_ = nullptr;

	int screenW_ = 0;	//  The size of the
	int screenH_ = 0;	//  graphics buffer.

	Color clearColor_ = Color(0.1f, 0.0f, 0.2f, 1.0f);

	bool bDrawStatistics_ = true;				// Draw FPS, DIPs,
	Texture pStatisticTexture_ = nullptr;		// particle amount, etc.

	Font pDefaultFont_ = nullptr;				// Default font Arial9

	static const int kMaxSpritesPerDIP = 256;	// Max of sprites drawed per one draw call.

	std::string cShaderFolder_;

	void commonInit();

	void commonRelease();

	friend std::shared_ptr<IRender>;
};

typedef std::shared_ptr<IRender> Render;