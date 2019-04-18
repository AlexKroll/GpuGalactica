#pragma once

#include <d3d11.h>
#include <dxgi.h>

#include <wincodec.h>

#include "Render.h"
#include "TextureDX11.h"
#include "Font.h"
#include "MiscDX11.h"
#include "ShaderProgramDX11.h"
#include "BuffersDX11.h"
#include "../ComputeBuffer.h"




class RenderDX11 : public IRender
{
public:
	virtual int init(HWND hWnd, bool bWindowed) override final;

	virtual void release() override final;


	//  Vertex meshes..
	virtual Mesh createMesh(uint32_t sizeBytes, void* pVertices, byte vertexSize, void* pIndices, int numPrimitives, IMesh::MeshUsage* pUsage) override final;

	void releaseMesh(Mesh& pMesh) override;


	//  Textures..
	virtual Texture createTexture(UINT width, UINT height, ITexture::PixelFormat format, ITexture::Type type, uint32_t flags, BYTE* pPixels = nullptr) override final;

	virtual Texture createTexture1D(UINT width, ITexture::PixelFormat format, ITexture::Type type, uint32_t flags, BYTE* pPixels = nullptr) override final;

	virtual Texture createDepthBuffer(UINT width, UINT height, ITexture::PixelFormat format, UINT multisamples = 0) override final;

	virtual Texture LoadTextureFromFile(cstring filePath) override final;

	virtual void setTexture(int slot, Texture texture) override final;

	virtual void releaseTexture(Texture& pTexture) override final;


	//  Fonts..
	virtual Font loadFont(cstring filePath) override final;

	virtual void releaseFont(Font& pFont) override final;

	virtual CFont getDefaultFont() override final;


	//  Render targets..
	virtual int keepRenderTargetContext() override final;

	virtual int restoreRenderTargetContext() override final;

	virtual int setRenderTargetContext(Texture pTexture, Texture pDepth, RECT* pRect = nullptr) override final;


	//  States..
	virtual void setEnableDepthBuffer(bool bEnabled) override final;

	virtual void setEnableDepthWrite(bool bEnabled) override final;

	void setDepthState(ID3D11DepthStencilState* pDepthState);

	virtual void setAlphaBlendNormal() override final;

	virtual void setAlphaBlendAdditive() override final;

	virtual void setAlphaBlendOff() override final;

	ID3D11BlendState* createBlendState(bool bEnable, BYTE colorMask,
									   D3D11_BLEND srcBlendColor, D3D11_BLEND destBlendColor, D3D11_BLEND_OP BlendOpColor, 
									   D3D11_BLEND srcBlendAlpha, D3D11_BLEND destBlendAlpha, D3D11_BLEND_OP blendOpAlpha);

	void setBlendState(ID3D11BlendState* pBlendState);

	ID3D11SamplerState* createSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE texAddress);

	void setSampler(ID3D11SamplerState* pSampler, int slot);

	ID3D11RasterizerState* createRasterState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool bMultisample);

	void setRasterState(ID3D11RasterizerState* pRasterState);

	virtual void setViewport(int x, int y, int w, int h) override final;

	void setViewport(const D3D11_VIEWPORT& vp);


	//  Shaders..
	virtual ShaderProgram createShaderProgram(PCCHAR pVertText, PCCHAR pPixText) override final;

	virtual ShaderProgram createShaderProgram(Shader pVShader, PCCHAR pPixText) override final;

	virtual ShaderProgram createShaderProgram(PCCHAR pVertText, Shader pPShader) override final;

	virtual ShaderProgram createShaderProgram(cstring vsFile, cstring psFile, CShaderMacroStrings defines) override final;

	void linkShaderProgram(ShaderProgramDX11* pProgram, ID3D11VertexShader* pDxVShader, ID3D11PixelShader* pDxPShader);

	virtual void setShaderProgram(ShaderProgram pProgram) override final;

	virtual ShaderProgram getSpriteShaderProgram() override final;


	//  Vertex data..
	virtual VertexInput createVertexInput(uint32_t vertexFlags, uint32_t instanceFlags) override;

	virtual void setVertexInput(VertexInput pVertexInput) override;

	virtual void releaseVertexInput(VertexInput& pVertexInput) override;


	//  Constant buffer..
	virtual ConstantBuffer createConstantBuffer(size_t sizeBytes) override final;

	virtual void* openConstantBuffer(ConstantBuffer pBuffer) override final;

	virtual void closeConstantBuffer(ConstantBuffer pBuffer) override final;

	virtual void updateConstantBuffer(ConstantBuffer pBuffer, void* pData, int dataSize) override final;

	virtual void setConstantBuffers(ConstantBuffer pVertexCBuffer, ConstantBuffer pPixelCBuffer) override final;

	virtual void setConstantBufferVS(ConstantBuffer pCBuffer, UINT slot) override final;
	virtual void setConstantBufferPS(ConstantBuffer pCBuffer, UINT slot) override final;


	//  Vertex buffer..
	virtual VertexBuffer createVertexBuffer(uint32_t sizeBytes, bool bWritable, void* pVertices, int stride) override final;

	virtual void* openVertexBuffer(VertexBuffer pBuffer) override final;

	virtual void closeVertexBuffer(VertexBuffer pBuffer) override final;

	virtual void setVertexBuffer(VertexBuffer pVertexBuffer, UINT slot) override final;


	//  Index buffer..
	virtual IndexBuffer createIndexBuffer(uint32_t sizeBytes, bool bWritable, bool bIndices32, void* pIndices) override final;

	virtual void* openIndexBuffer(IndexBuffer pBuffer) override final;

	virtual void closeIndexBuffer(IndexBuffer pBuffer) override final;

	virtual void setIndexBuffer(IndexBuffer pIndexBuffer) override final;


	//  Compute buffer..
	virtual ComputeBuffer createComputeBuffer(int amountOfElems, int elemSize, uint32_t flags) override;
		// The buffer is treated as an array of elements.
		// amountOfElems, elemSize - number of elements and size of one element.
		// flags:
		//		IParallelCompute::kBufferReadWrite.. read/write access.
		//		IParallelCompute::kShared - buffer is shared with graphics API (for example OpenCL/DX).
		//		IParallelCompute::kAutoLength - compute buffer has auto counter of elements.

	virtual void bindComputeBufferToTextureVS(ComputeBuffer buffer, UINT slot) override;
		// Represents a compute buffer as a data from texture slot (See DirectX StructuredBuffer).

	ComputeBuffer createConstantComputeBuffer(size_t sizeBytes);

	//  Unordered access compute buffer..
	UABufferDX11* createUABuffer(int amountOfElems, int elemSize, uint32_t flags);
		// These buffers are used by DX GPU computing.
		// They have arbitrary access for general calculations.
		// flags:
		//		IParallelCompute::kShared - buffer is shared with graphics API (for example OpenCL/DX).
		//		IParallelCompute::kAutoLength - compute buffer has auto counter of elements.


	ID3D11Buffer* createDxApiBuffer(D3D11_BIND_FLAG bind, size_t sizeBytes, bool bWritable, bool bReadable, void* pData);
	void* openDxApiBuffer(ID3D11Buffer* pDxApiBuffer);
	void closeDxApiBuffer(ID3D11Buffer* pDxApiBuffer);
	void copyBuffers(ID3D11Buffer* pDestBuffer, ID3D11Buffer* pSourBuffer);


	//  2D drawing..
	virtual void drawSprite(CPoint pos, Texture pTexture, CRect rect, uint32_t flags) override final;

	virtual void drawSprite(CPoint pos, Texture pTexture, CRect rect, uint32_t color, uint32_t flags) override final;

	virtual void drawSprites(int numSprites, const Point* pPoints, const Rect* pRects, Texture pTexture, uint32_t flags) override final;

	virtual void drawSprites(int numSprites, const Point* pPoints, const Rect* pRects, Texture pTexture, uint32_t color, uint32_t flags) override final;


	//  Indexed triangles drawing..
	virtual void drawIndexedTriangles(int numTriangles) override final;

	virtual void drawIndexedTrianglesInstanced(int numTriangles, int numInstances) override final;


	//  GPU compute API.
	ID3D11ComputeShader* createComputeShader(cstring csFile, CShaderMacroStrings defines);

	void setComputeShader(ID3D11ComputeShader* pShader);

	void setUABuffer(UABufferDX11* buffer, uint32_t slot);

	void setConstantBufferCS(ComputeBuffer buffer, UINT slot);

	//void bindUABufferToTextureVS(UABufferDX11* buffer, UINT slot);

	void setComputeTexture(Texture texture, uint32_t slot);

	void setComputeSampler(uint32_t samplerType, uint32_t slot);

	void compute(UINT X, UINT Y, UINT Z);


	// Misc.
	virtual void setDefaultSampler(int slot) override final;

	virtual void setDefaultRasterState() override final;

	virtual void setRasterState(uint32_t rasterType) override final;


	//ID3D11Buffer* createConstantBufferDX(size_t sizeBytes);


	virtual void prepareDrawing() override final;

	virtual void presentDrawing() override final;


	ID3D11Device* getDevice()
	{  return pDevice3D_;
	}

	friend class IRender;

private:
	ID3D11Device* pDevice3D_ = nullptr;
	IDXGISwapChain* pSwapChain_ = nullptr;
	ID3D11DeviceContext* pDeviceContext_ = nullptr;

	ITexture::PixelFormat backBuffDepthFormat_ = ITexture::UnknownFormat;

	int multiSampling_ = 1;
	int multiSamplingQuality_ = 0;

	// Current render context.
	//////
	ID3D11RenderTargetView* pMainRenderTargetView_ = nullptr;	// Primary and current
	ID3D11RenderTargetView* pCurrRenderTargetView_ = nullptr;	// render target.

	ID3D11DepthStencilView* pMainDepthStencilView_ = nullptr;	// Primary and current
	ID3D11DepthStencilView* pCurrDepthStencilView_ = nullptr;	// depth buffer.

	D3D11_VIEWPORT currViewPort_;

	TextureDX11 pDepthBuffer_ = nullptr;

	ShaderProgram pCurrShaderProgram_ = nullptr;

	VertexInput pCurrVertexInput_ = nullptr;

	ID3D11BlendState* pCurrBlendState_ = nullptr;

	static const int kMaxTextureSlots = 16;

	Texture pCurrTextures_[kMaxTextureSlots] = { nullptr };

	ID3D11SamplerState* pCurrSamplers_[kMaxTextureSlots] = { nullptr };

	ID3D11RasterizerState* pCurrRasterState_ = nullptr;

	static const int kMaxSlotsVB = 4;
	VertexBuffer pCurrVertexBuffers_[kMaxSlotsVB] = { nullptr };
	IndexBuffer pCurrIndexBuffer_ = nullptr;

	D3D_PRIMITIVE_TOPOLOGY mCurrTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	///////

	std::vector<RenderContext> renderContexts_;  // Custom render contexts.

	// Popular depth states.
	ID3D11DepthStencilState* currDepthState_ = nullptr;
	ID3D11DepthStencilState* depthEnabledState_ = nullptr;
	ID3D11DepthStencilState* depthDisabledState_ = nullptr;
	ID3D11DepthStencilState* depthWriteOffState_ = nullptr;

	// Popular blend states.
	ID3D11BlendState* pBlendNormalState_ = nullptr;
	ID3D11BlendState* pBlendAdditiveState_ = nullptr;
	ID3D11BlendState* pBlendOffState_ = nullptr;

	// Popular texture sampler states.
	ID3D11SamplerState* pDefaultSampler_ = nullptr;		// Linear-Clamp
	ID3D11SamplerState* pPointClampSampler_ = nullptr;  // Point-Clamp
	ID3D11SamplerState* pLinearWrapSampler_ = nullptr;  // Linear-Wrap

	// Popular sampler states.
	ID3D11RasterizerState* pDefaultRasterState_ = nullptr;
	ID3D11RasterizerState* pCullOffRasterState_ = nullptr;

	// The texture with GDI support for creating fonts.
	//Texture pFontsTexture_ = nullptr;
	int fontsTextureW_ = 2048;
	int fontsTextureH_ = 2048;

	//IWICImagingFactory* pImagingFactory_ = nullptr;

	// 2D sprite data.
	VertexInput pSpriteVertInput_ = nullptr;
	ConstantBuffer pSpriteCBufferVS_ = nullptr;
	ShaderProgram pSpriteShaderProgram_ = nullptr;
	VertexBuffer pSpriteVertexBuffer_ = nullptr;
	VertexBuffer pSpriteVBufferInst_ = nullptr;
	IndexBuffer pSpriteIndexBuffer_ = nullptr;


	struct SpriteInstance
	{
		Vec4 sizePos;
		Vec4 uvMulAdd;
		uint32_t color;
	};

	RenderDX11();
	~RenderDX11();
};


