#include <cassert>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include "../Utils/Utils.h"

#include "FontDX11.h"
#include "RenderDX11.h"
#include "FileDDS.h"
#include "../../ParallelCompute/DirectX11Compute.h"
//#include "FileTGA.h"



RenderDX11::RenderDX11()
{
}


RenderDX11::~RenderDX11()
{
	release();
}


int RenderDX11::init(HWND hWnd, bool bWindowed)
{
	TextureDX11impl::initNativeFormats();

	hWindow_ = hWnd;

	UINT device_flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

	//device_flags |= D3D11_CREATE_DEVICE_DEBUG;

	if (bWindowed)
	{	RECT ClientRect;
		GetClientRect(hWindow_, &ClientRect);

		screenW_ = ClientRect.right - ClientRect.left;
		screenH_ = ClientRect.bottom - ClientRect.top;
	}

	// Create device
	{
		DXGI_SWAP_CHAIN_DESC desc;
		::ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));

		desc.BufferCount = 3;
		desc.BufferDesc.Width = screenW_;
		desc.BufferDesc.Height = screenH_;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.OutputWindow = hWnd;
		desc.SampleDesc.Count = multiSampling_;
		desc.SampleDesc.Quality = 0;
		desc.Windowed = bWindowed;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_0;
		D3D_FEATURE_LEVEL featureLevelsSupported;

		HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, device_flags,
							&featureLevels, 1, D3D11_SDK_VERSION, &desc, &pSwapChain_, &pDevice3D_, &featureLevelsSupported, &pDeviceContext_);
		if (hr != S_OK)
			return hr;

		screenW_ = desc.BufferDesc.Width;
		screenH_ = desc.BufferDesc.Height;
	}

	// Create a render target view.
	ID3D11Texture2D* pback_buffer = nullptr;
	HRESULT hr = pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pback_buffer);
	if (hr != S_OK)
		return hr;

	hr = pDevice3D_->CreateRenderTargetView(pback_buffer, nullptr, &pMainRenderTargetView_);
	if (hr != S_OK)
		return hr;

	pCurrRenderTargetView_ = pMainRenderTargetView_;

	//SAFE_RELEASE_DX(pback_buffer); // Release because the CreateRenderTargetView incs the ref counter.
	//ULONG refs = pback_buffer->Release();

	cShaderFolder_ = "Shaders/HLSL/";

	// Create depth stencil texture.
	backBuffDepthFormat_ = ITexture::Depth32;
	Texture pDepth = createDepthBuffer(screenW_, screenH_, backBuffDepthFormat_, multiSampling_);
	if (pDepth)
	{
		pDepthBuffer_ = std::dynamic_pointer_cast<TextureDX11impl>(pDepth);
		// or simple pDepthBuffer_ = pDepth;

		pMainDepthStencilView_ = pDepthBuffer_->pDepthView_;
		pCurrDepthStencilView_ = pMainDepthStencilView_;

		pDeviceContext_->OMSetRenderTargets(1, &pCurrRenderTargetView_, pCurrDepthStencilView_);
	}
	else
	{
		//  Check available depth formats.
	}

	// Create popular depth states.
	{
		D3D11_DEPTH_STENCIL_DESC depth_desc;

		// Default depth-enabled state.
		::ZeroMemory(&depth_desc, sizeof(depth_desc));
		depth_desc.DepthEnable = true;
		depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depth_desc.DepthFunc = D3D11_COMPARISON_LESS;
		hr = pDevice3D_->CreateDepthStencilState(&depth_desc, &depthEnabledState_);
		if (hr != S_OK ) {}

		// Depth-off state.
		::ZeroMemory(&depth_desc, sizeof(depth_desc));
		depth_desc.DepthEnable = false;
		depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depth_desc.DepthFunc = D3D11_COMPARISON_NEVER;
		hr = pDevice3D_->CreateDepthStencilState(&depth_desc, &depthDisabledState_);
		if (hr != S_OK ) {};

		// Depth-write-off state.
		::ZeroMemory(&depth_desc, sizeof(depth_desc));
		depth_desc.DepthEnable = true;
		depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depth_desc.DepthFunc = D3D11_COMPARISON_LESS;
		hr = pDevice3D_->CreateDepthStencilState(&depth_desc, &depthWriteOffState_);
		if (hr != S_OK ) {};

	}

	// Create popular blend states.
	{
		pBlendNormalState_ = createBlendState(true, D3D11_COLOR_WRITE_ENABLE_ALL,
									D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
									D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD);

		pBlendAdditiveState_ = createBlendState(true, D3D11_COLOR_WRITE_ENABLE_ALL,
									D3D11_BLEND_SRC_COLOR, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD,
									D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD);

		pBlendOffState_ = createBlendState(false, D3D11_COLOR_WRITE_ENABLE_ALL,
									D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
									D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD);
	}

	// Create popular sampler states.
	{
		pDefaultSampler_ = createSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP);

		pPointClampSampler_ = createSampler(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP);

		pLinearWrapSampler_ = createSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);
	}

	// Create popular raster states.
	pDefaultRasterState_ = createRasterState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	pCullOffRasterState_ = createRasterState(D3D11_FILL_SOLID, D3D11_CULL_NONE, false);

	//pFontsTexture_ = createTexture(fontsTextureW_, fontsTextureH_, ITexture::ColorBGRA8, ITexture::GdiCompatible);

	// IWICImagingFactory for loading textures.
	//hr = CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory2), reinterpret_cast<LPVOID*>(&pImagingFactory_));
	//if (hr != S_OK)
	//	return hr;

	// Sprite data: vertex input, shader program, quad geometry.
	{
		pSpriteVertInput_ = createVertexInput(kVertPosition | kVertTexCoord0, kInstFloat4_0 | kInstFloat4_1 | kVertColor0);

		pSpriteCBufferVS_ = createConstantBuffer(16);

		//pSpriteCBufferPS_ = createConstantBuffer(16);

		TCHAR* vsText = "float4 ScreenProj : register(c0);	\
						void Main(in float4 VertexPos: POSITION, in float2 VertexTexCrd: TEXCOORD0,	\
								  in float4 SizePos: INSTANCE4F0, in float4 UvMulAdd: INSTANCE4F1, in float4 Color: COLOR0, \
								  out float4 OutPos: SV_Position, out float2 OutTexCrd: TEXCOORD0, out float4 OutColor: COLOR0)	\
						{ \
							OutPos.xy = VertexPos.xy * SizePos.xy + SizePos.zw; \
							OutPos.xy *= ScreenProj.xy; \
							OutPos.zw = VertexPos.zw; \
							OutTexCrd.xy = VertexTexCrd.xy * UvMulAdd.xy + UvMulAdd.zw; \
							OutColor = Color; \
						}";
		TCHAR* psText = "Texture2D Tex: register(t0);  \
						 sampler Samp: register(s0);   \
						 void Main( in float4 Pos: SV_POSITION, in float2 TexCoord: TEXCOORD0, in float4 Color: COLOR0, out float4 ColorOut : SV_Target ) \
						 { \
							ColorOut.rgba = Tex.Sample(Samp,TexCoord).rgba * Color.rgba; \
						 }";

		pSpriteShaderProgram_ = createShaderProgram(vsText, psText);

		// The quad of two triangles. Top-left point is(0,0). Right-bottom point is (1,-1).
		VertexPosUv verts[4];
		verts[0] = VertexPosUv(Vec3(0.0f,  0.0f, 0.0f),  Vec2(0.0f, 0.0f));
		verts[1] = VertexPosUv(Vec3(1.0f,  0.0f, 0.0f),  Vec2(1.0f, 0.0f));
		verts[2] = VertexPosUv(Vec3(0.0f, -1.0f, 0.0f),  Vec2(0.0f, 1.0f));
		verts[3] = VertexPosUv(Vec3(1.0f, -1.0f, 0.0f),  Vec2(1.0f, 1.0f));

		pSpriteVertexBuffer_ = createVertexBuffer(sizeof(verts), false, verts, sizeof(VertexPosUv));

		WORD indices[6] = { 0,1,2, 1,3,2 }; // Two triangles - 6 indices.

		pSpriteIndexBuffer_ = createIndexBuffer(sizeof(indices), false, false, indices);

		// Vertex buffer for instances.
		pSpriteVBufferInst_ = createVertexBuffer(sizeof(SpriteInstance) * kMaxSpritesPerDIP, true, nullptr, sizeof(SpriteInstance));

		if (nullptr == pSpriteVertexBuffer_ || nullptr == pSpriteIndexBuffer_ || nullptr == pSpriteVBufferInst_ )
			return ERROR_UNINITIALIZED;
	}

	setViewport(0, 0, screenW_, screenH_);

	commonInit();

	return S_OK;
}


void RenderDX11::release()
{
	FontDX11::finalize();

	pDepthBuffer_ = nullptr;

	SAFE_RELEASE_DX(depthEnabledState_);
	SAFE_RELEASE_DX(depthDisabledState_);
	SAFE_RELEASE_DX(depthWriteOffState_);

	SAFE_RELEASE_DX(pBlendNormalState_);
	SAFE_RELEASE_DX(pBlendAdditiveState_);
	SAFE_RELEASE_DX(pBlendOffState_);

	SAFE_RELEASE_DX(pDefaultSampler_);
	SAFE_RELEASE_DX(pPointClampSampler_);
	SAFE_RELEASE_DX(pLinearWrapSampler_);

	SAFE_RELEASE_DX(pDefaultRasterState_);
	SAFE_RELEASE_DX(pCullOffRasterState_);

	pSpriteVertInput_ = nullptr;
	pSpriteCBufferVS_ = nullptr;
	pSpriteShaderProgram_ = nullptr;
	pSpriteVertexBuffer_ = nullptr;
	pSpriteVBufferInst_ = nullptr;
	pSpriteIndexBuffer_ = nullptr;

	pCurrVertexInput_ = nullptr;
	pCurrShaderProgram_ = nullptr;

	for (Texture& pTexture : pCurrTextures_)
		pTexture = nullptr;

	for (VertexBuffer& vb : pCurrVertexBuffers_)
		vb = nullptr;

	pCurrIndexBuffer_ = nullptr;

	commonRelease();

	SAFE_RELEASE_DX(pMainRenderTargetView_);
	pCurrRenderTargetView_ = nullptr;

	SAFE_RELEASE_DX(pMainDepthStencilView_);
	pCurrDepthStencilView_ = nullptr;

	//pImagingFactory_ = nullptr;

	SAFE_RELEASE_DX(pSwapChain_);

	SAFE_RELEASE_DX(pDeviceContext_);


//ID3D11Debug* pDebug = nullptr;
//HRESULT hr = pDevice3D_->QueryInterface(__uuidof(ID3D11Debug), (void**)&pDebug);
//hr = pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

	SAFE_RELEASE_DX(pDevice3D_);
}


Mesh RenderDX11::createMesh(uint32_t sizeBytes, void* pVertices, byte vertexSize, void* pIndices, int numPrimitives, IMesh::MeshUsage* pUsage)
{
	Mesh pMesh = std::shared_ptr<IMesh>(new IMesh);
	if (pMesh)
	{
		pMesh->pVertBuffer = createVertexBuffer(sizeBytes, false, pVertices, vertexSize);

		bool bInds32 = false;
		uint32_t indSizeBytes = numPrimitives * 6; // 3 indices of 2 bytes.
		if (pUsage && pUsage->indType == IMesh::Ind32)
		{	bInds32 = true;
			indSizeBytes = numPrimitives * 12; // 3 indices of 4 bytes.
		}

		pMesh->pIndBuffer = createIndexBuffer(indSizeBytes, false, bInds32, pIndices);

		if (pMesh->pVertBuffer && pMesh->pIndBuffer)
		{
			pMesh->vertexSize_ = vertexSize;

			pMesh->numPrimitives_ = numPrimitives;

			pMesh->indexType_ = (bInds32) ? IMesh::Ind32 : IMesh::Ind16;
		}
	}

	return pMesh;
}


void RenderDX11::releaseMesh(Mesh& pMesh)
{
	if (pMesh)
		pMesh = nullptr;
}


Texture RenderDX11::createTexture(UINT width, UINT height, ITexture::PixelFormat format, ITexture::Type type, uint32_t flags, BYTE* pPixels)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = TextureDX11impl::PixFormats[format];
    desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;

	desc.CPUAccessFlags = 0;

	desc.MiscFlags = 0;
	if (flags & ITexture::kMiscShared)
		desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;

    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	if (type == ITexture::Type::StaticTexture && pPixels)
		desc.Usage = D3D11_USAGE_IMMUTABLE;

	if (type == ITexture::Type::RenderTarget)
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;

	if (type == ITexture::Type::GdiCompatible)
	{
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags |= D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // only this format is compatible for GDI.
	}

	if (type == ITexture::Type::DynamicTexture)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	D3D11_SUBRESOURCE_DATA data;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	D3D11_SUBRESOURCE_DATA* pdata = nullptr;

	if (pPixels)
	{	data.pSysMem = pPixels;
		data.SysMemPitch = width * ITexture::getBytesPerPixel(format);
		pdata = &data;
	}

	ID3D11Texture2D* pDxApiTexture = nullptr;

	HRESULT hr = pDevice3D_->CreateTexture2D(&desc, pdata, &pDxApiTexture);
	if (hr != S_OK)
	{
		ShowError(hWindow_, __FUNCTION__, ". Error: ID3D11Device::CreateTexture2D failed.");
		return nullptr;
	}

	ID3D11RenderTargetView* pRndrTrgtView = nullptr;

	if (desc.BindFlags & D3D11_BIND_RENDER_TARGET)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rt_desc;
		rt_desc.Format = desc.Format;
		rt_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rt_desc.Texture2D.MipSlice = 0;

		hr = pDevice3D_->CreateRenderTargetView(pDxApiTexture, &rt_desc, &pRndrTrgtView);
		if (hr != S_OK)
		{
			ShowError(hWindow_, __FUNCTION__, ". Error: ID3D11Device::CreateRenderTargetView failed.");

			if (pDxApiTexture)
				pDxApiTexture->Release();
			return nullptr;
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC view_desc;
	view_desc.Format = desc.Format;
	view_desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	view_desc.Texture2D.MostDetailedMip = 0;
	view_desc.Texture2D.MipLevels = 1;

	ID3D11ShaderResourceView* pShaderView = nullptr;

	hr = pDevice3D_->CreateShaderResourceView(pDxApiTexture, &view_desc, &pShaderView);
	if (hr != S_OK)
	{
		ShowError(hWindow_, __FUNCTION__, ". Error: ID3D11Device::CreateShaderResourceView failed.");

		if (pDxApiTexture)
			pDxApiTexture->Release();
		return nullptr;
	}

	TextureDX11 pTexture = std::shared_ptr<TextureDX11impl>(new TextureDX11impl);
	//TextureDX11impl* pTexture = new TextureDX11impl;
	if (nullptr == pTexture)
	{
		ShowError(hWindow_, __FUNCTION__, ". Memory allocation failed.");

		return nullptr;
	}

	pTexture->pDxTexture_ = pDxApiTexture;
	pTexture->pShaderView_ = pShaderView;
	pTexture->pRndrTrgtView_ = pRndrTrgtView;

	pTexture->fillDesc();

	return pTexture;
}


Texture RenderDX11::createTexture1D(UINT width, ITexture::PixelFormat format, ITexture::Type type, uint32_t flags, BYTE* pPixels)
{
	D3D11_TEXTURE1D_DESC desc;
	desc.Width = width;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = TextureDX11impl::PixFormats[format];
    desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;

	desc.MiscFlags = 0;
	if (flags & ITexture::kMiscShared)
		desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;

    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	if (type == ITexture::Type::StaticTexture && pPixels)
		desc.Usage = D3D11_USAGE_IMMUTABLE;

	if (type == ITexture::Type::DynamicTexture)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	D3D11_SUBRESOURCE_DATA data;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	D3D11_SUBRESOURCE_DATA* pdata = nullptr;

	if (pPixels)
	{	data.pSysMem = pPixels;
		data.SysMemPitch = width * ITexture::getBytesPerPixel(format);
		pdata = &data;
	}

	ID3D11Texture1D* pDxApiTexture = nullptr;

	HRESULT hr = pDevice3D_->CreateTexture1D(&desc, pdata, &pDxApiTexture);
	if (hr != S_OK)
	{
		ShowError(hWindow_, __FUNCTION__, ". Error: ID3D11Device::CreateTexture2D failed.");
		return nullptr;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC view_desc;
	view_desc.Format = desc.Format;
	view_desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE1D;
	view_desc.Texture2D.MostDetailedMip = 0;
	view_desc.Texture2D.MipLevels = 1;

	ID3D11ShaderResourceView* pShaderView = nullptr;

	hr = pDevice3D_->CreateShaderResourceView(pDxApiTexture, &view_desc, &pShaderView);
	if (hr != S_OK)
	{
		ShowError(hWindow_, __FUNCTION__, ". Error: ID3D11Device::CreateShaderResourceView failed.");

		if (pDxApiTexture)
			pDxApiTexture->Release();
		return nullptr;
	}

	TextureDX11 pTexture = std::shared_ptr<TextureDX11impl>(new TextureDX11impl);
	//TextureDX11impl* pTexture = new TextureDX11impl;
	if (nullptr == pTexture)
	{
		ShowError(hWindow_, __FUNCTION__, ". Memory allocation failed.");

		return nullptr;
	}

	pTexture->pDxTexture_ = reinterpret_cast<ID3D11Texture2D*>(pDxApiTexture);
	pTexture->pShaderView_ = pShaderView;

	pTexture->fillDesc();

	return pTexture;
}


Texture RenderDX11::createDepthBuffer(UINT width, UINT height, ITexture::PixelFormat format, UINT multisamples)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = TextureDX11impl::PixFormats[format];

	if( multisamples > 1 )
	{	desc.SampleDesc.Count = multisamples;
		desc.SampleDesc.Quality = multiSamplingQuality_;
	}
	else
	{	desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
	}

    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

	ID3D11Texture2D* pDxApiTexture = nullptr;

	HRESULT hr = pDevice3D_->CreateTexture2D(&desc, nullptr, &pDxApiTexture);
	if (hr != S_OK)
	{
		ShowError(hWindow_, __FUNCTION__, ". Error: ID3D11Device::CreateTexture2D failed.");
		return nullptr;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depth_desc;
	depth_desc.Format = desc.Format;
	depth_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_desc.Texture2D.MipSlice = 0;
	depth_desc.Flags = 0;

	ID3D11DepthStencilView* pDepthView = nullptr;

	hr = pDevice3D_->CreateDepthStencilView(pDxApiTexture, &depth_desc, &pDepthView);
	if (hr != S_OK)
	{
		ShowError(hWindow_, __FUNCTION__, ". Error: ID3D11Device::CreateDepthStencilView failed.");

		if (pDxApiTexture)
			pDxApiTexture->Release();
		return nullptr;
	}

	//TextureDX11impl* pTexture = new TextureDX11impl;
	//if (pTexture == nullptr)
	TextureDX11 pTexture = std::shared_ptr<TextureDX11impl>(new TextureDX11impl);
	if (nullptr == pTexture)
	{
		ShowError(hWindow_, __FUNCTION__, ". Memory allocation failed.");

		return nullptr;
	}

	pTexture->pDxTexture_ = pDxApiTexture;
	pTexture->pDepthView_ = pDepthView;

	pTexture->fillDesc();

	return pTexture;
}


Texture RenderDX11::LoadTextureFromFile(cstring filePath)
{
	if (filePath.length() == 0)
		return nullptr;

	std::unique_ptr<byte[]> pixels;

	UINT width = 0, height = 0;

	ITexture::PixelFormat texFormat = ITexture::UnknownFormat;

	// Get file extension.
	if (filePath.find("tga") != std::string::npos)
	{
		//TgaHeader header;
		//hr = LoadTGA(filePath, &header, pixels);
		//if (hr != S_OK )
		//	return nullptr;
				
	}

	if (filePath.find("dds") != std::string::npos)
	{
		DDS_Header header;
		HRESULT hr = LoadDDS(filePath, &header, pixels);
		if (hr != S_OK )
			return nullptr;

		if (header.ddspf.RGBBitCount == 32)
		{	texFormat = ITexture::ColorBGRA8;
		}
		else
		{
			if (header.ddspf.RGBBitCount == 8)
			{	if (header.ddspf.ABitMask)
					texFormat = ITexture::ColorA8;
				else
					texFormat = ITexture::ColorR8;
			}
		}

		width = header.width;
		height = header.height;
	}

	if (pixels.get() == nullptr || ITexture::UnknownFormat == texFormat)
	{
		wchar_t filePathWide[512];
		MultiByteToWideChar( CP_ACP, 0, filePath.c_str(), -1, filePathWide, _countof(filePathWide) );

		/*ComPtr<IWICBitmapDecoder> decoder;
		HRESULT hr = pImagingFactory_->CreateDecoderFromFilename(filePathWide, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());
		if (hr == S_OK)
		{
			ComPtr<IWICBitmapFrameDecode> frame;
			hr = decoder->GetFrame(0, frame.GetAddressOf());
			if (FAILED(hr))
				return nullptr;

			frame->GetSize(&width, &height);
			assert(width > 0 && height > 0);

			// Find the pixel format.
			WICPixelFormatGUID pixelFormat;
			hr = frame->GetPixelFormat(&pixelFormat);
			if (FAILED(hr))
				return nullptr;

			bool bAlphaPresent = true;
			//bool swapRB = true;
			UINT bpp = 32;

			if (memcmp(&GUID_WICPixelFormat32bppBGR, &pixelFormat, sizeof(GUID)) == 0)
			{
				texFormat = ITexture::ColorRGBA8;
			}

			if (memcmp(&GUID_WICPixelFormat32bppBGRA, &pixelFormat, sizeof(GUID)) == 0)
			{
				texFormat = ITexture::ColorRGBA8;
				//swapRB = true;
			}

			if (memcmp(&GUID_WICPixelFormat32bppRGBA, &pixelFormat, sizeof(GUID)) == 0)
			{
				texFormat = ITexture::ColorRGBA8;
			}

			if (memcmp(&GUID_WICPixelFormat24bppBGR, &pixelFormat, sizeof(GUID)) == 0)
			{
				texFormat = ITexture::ColorRGBA8;
				bAlphaPresent = false;
				bpp = 24;  // DirectX11 does not support 24-bit RGB textures. We will convert to 32 bits.
			}

			if (memcmp(&GUID_WICPixelFormat24bppRGB, &pixelFormat, sizeof(GUID)) == 0)
			{
				texFormat = ITexture::ColorRGBA8;
				bAlphaPresent = false;
				bpp = 24;  // DirectX11 does not support 24-bit RGB textures. We will convert to 32 bits.
			}

			if (memcmp(&GUID_WICPixelFormat8bppAlpha, &pixelFormat, sizeof(GUID)) == 0)
			{
				texFormat = ITexture::ColorA8;
				bpp = 8;
			}

			if (memcmp(&GUID_WICPixelFormat8bppIndexed, &pixelFormat, sizeof(GUID)) == 0)
			{
				texFormat = ITexture::ColorRGBA8;
				bpp = 8;
			}

			UINT row_pitch = width * 4; //bpp / 8;
			UINT image_size = row_pitch * height;

			pixels = std::make_unique<byte[]>(image_size);

			// Convert to desired format.
			if (memcmp(&GUID_WICPixelFormat32bppRGBA, &pixelFormat, sizeof(GUID)) != 0)
			{	ComPtr<IWICFormatConverter> converter;
				hr = pImagingFactory_->CreateFormatConverter(converter.GetAddressOf());
				if (FAILED(hr))
					return nullptr;

				BOOL canConvert = FALSE;
				hr = converter->CanConvert(pixelFormat, GUID_WICPixelFormat32bppRGBA, &canConvert);
				if (FAILED(hr) || !canConvert)
					return nullptr;

				hr = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom);
				if (FAILED(hr))
					return nullptr;

				hr = converter->CopyPixels(nullptr, row_pitch, image_size, pixels.get());
			}
			else
			{	hr = frame->CopyPixels(nullptr, row_pitch, image_size, pixels.get());
			}

			if (FAILED(hr))
				return nullptr;

			//decoder.Get()->Release();
			//CloseHandle(hf);
		}*/
	}

	if (pixels.get() && ITexture::UnknownFormat != texFormat)
	{
//LL:
		Texture texture = createTexture(width, height, texFormat, ITexture::StaticTexture, 0, pixels.get());
		return texture;
	}

	return nullptr;
}


void RenderDX11::setTexture(int slot, Texture texture)
{
	if (slot >=0 && slot < kMaxTextureSlots && pCurrTextures_[slot] != texture)
	{
		pCurrTextures_[slot] = texture;

		if (texture)
		{
			//TextureDX11* pTextureDX = dynamic_cast<TextureDX11*>(texture);
			TextureDX11 pTextureDX = std::dynamic_pointer_cast<TextureDX11impl>(texture);

			if (pTextureDX)
				pDeviceContext_->PSSetShaderResources(slot, 1, &pTextureDX->pShaderView_);
		}
	}
}


void RenderDX11::releaseTexture(Texture& pTexture)
{
	if (pTexture)
	{
		for (int slot = 0; slot < kMaxTextureSlots; ++slot)
		{	if (pCurrTextures_[slot] == pTexture)
				pCurrTextures_[slot] = nullptr;
				//pCurrTextures_[slot]._Decref();
		}

		//long refs = pTexture.use_count();
		//if (1 == refs)
		//	pTexture->release();
		pTexture = nullptr;
	}
}


Font RenderDX11::loadFont(cstring filePath)
{
	IRender* base = this;
	Render render = std::shared_ptr<IRender>(base);

	Font pFont = std::shared_ptr<FontDX11>(FontDX11::createFont(render, filePath));
	//FontDX11* pFont = FontDX11::createFont(this, filePath);
	return pFont;
}


void RenderDX11::releaseFont(Font& pFont)
{
	if (pFont)
	{
		pFont->release();
		pFont = nullptr;
	}
}


CFont RenderDX11::getDefaultFont()
{
	return pDefaultFont_;
}


int RenderDX11::keepRenderTargetContext()
{
	if (pDeviceContext_ == nullptr)
		return -1;

	RenderContext context;

	pDeviceContext_->OMGetRenderTargets(1, &context.pRenderTargetView, &context.pDepthStencilView);

	context.viewPort = currViewPort_;

	renderContexts_.push_back(context);

	return 0;
}


int RenderDX11::restoreRenderTargetContext()
{
	if (pDeviceContext_ == nullptr)
		return -1;

	if (renderContexts_.empty())
		return -2;

	RenderContext context = renderContexts_.back();

	pDeviceContext_->OMSetRenderTargets(1, &context.pRenderTargetView, context.pDepthStencilView);

	pCurrRenderTargetView_ = context.pRenderTargetView;
	pCurrDepthStencilView_ = context.pDepthStencilView;

	if (pCurrDepthStencilView_)
		setEnableDepthBuffer(true);
	else
		setEnableDepthBuffer(false);

	SAFE_RELEASE_DX(context.pRenderTargetView);  // Dec refs.
	SAFE_RELEASE_DX(context.pDepthStencilView);  // Dec refs.

	setViewport(context.viewPort);

	renderContexts_.pop_back();

	return 0;
}


int RenderDX11::setRenderTargetContext(Texture pTexture, Texture pDepth, RECT* pRect)
{
	if (pDeviceContext_ == nullptr)
		return -1;

	if (pTexture == nullptr)
		return -2;

	//TextureDX11* pTextureDX = dynamic_cast<TextureDX11*>(pTexture);
	TextureDX11 pTextureDX = std::dynamic_pointer_cast<TextureDX11impl>(pTexture);
	if (pTextureDX == nullptr)
		return -3;

	if (pTextureDX->pRndrTrgtView_ == nullptr)
		return -4;

	ID3D11DepthStencilView* pDepthStencilView = nullptr;
	if (pDepth)
	{	//TextureDX11* pDepthDX = dynamic_cast<TextureDX11*>(pDepth);
		TextureDX11 pDepthDX = std::dynamic_pointer_cast<TextureDX11impl>(pTexture);
		if (pDepthDX == nullptr)
			return -3;
		pDepthStencilView = pDepthDX->pDepthView_;
	}

	pDeviceContext_->OMSetRenderTargets(1, &pTextureDX->pRndrTrgtView_, pDepthStencilView);

	pCurrRenderTargetView_ = pTextureDX->pRndrTrgtView_;
	pCurrDepthStencilView_ = pDepthStencilView;

	D3D11_VIEWPORT vp;

	if (pRect)
	{	
		vp.TopLeftX = static_cast<float>(pRect->left);
		vp.TopLeftY = static_cast<float>(pRect->top);
		vp.Width = static_cast<float>(pRect->right - pRect->left + 1);
		vp.Height = static_cast<float>(pRect->bottom - pRect->top + 1);
	}
	else  if (pTexture)
	{
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.Width = static_cast<float>(pTextureDX->width_);
		vp.Height = static_cast<float>(pTextureDX->height_);
	}

	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	setViewport(vp);

	return 0;
}


void RenderDX11::setEnableDepthBuffer(bool bEnabled)
{
	if (bEnabled)
		setDepthState(depthEnabledState_);
	else
		setDepthState(depthDisabledState_);
}


void RenderDX11::setEnableDepthWrite(bool bEnabled)
{
	if (bEnabled)
		setDepthState(depthEnabledState_);
	else
		setDepthState(depthWriteOffState_);
}


void RenderDX11::setDepthState(ID3D11DepthStencilState* pDepthState)
{
	if (pDeviceContext_ && pDepthState && currDepthState_ != pDepthState)
	{
		currDepthState_ = pDepthState;

		pDeviceContext_->OMSetDepthStencilState(currDepthState_, 1);
	}
}


void RenderDX11::setAlphaBlendNormal()
{
	setBlendState(pBlendNormalState_);
}


void RenderDX11::setAlphaBlendAdditive()
{
	setBlendState(pBlendAdditiveState_);
}


void RenderDX11::setAlphaBlendOff()
{
	setBlendState(pBlendOffState_);
}


ID3D11BlendState* RenderDX11::createBlendState(bool bEnable, BYTE colorMask,
											   D3D11_BLEND srcBlendColor, D3D11_BLEND destBlendColor, D3D11_BLEND_OP BlendOpColor, 
											   D3D11_BLEND srcBlendAlpha, D3D11_BLEND destBlendAlpha, D3D11_BLEND_OP blendOpAlpha )
{
	D3D11_BLEND_DESC desc;
	::ZeroMemory(&desc, sizeof(desc));

	D3D11_RENDER_TARGET_BLEND_DESC* pRT_Desc = &desc.RenderTarget[0];
	pRT_Desc->BlendEnable = bEnable;
	pRT_Desc->RenderTargetWriteMask = colorMask;

	pRT_Desc->SrcBlend = srcBlendColor;
	pRT_Desc->DestBlend = destBlendColor;
	pRT_Desc->BlendOp = BlendOpColor;

	pRT_Desc->SrcBlendAlpha = srcBlendAlpha;
	pRT_Desc->DestBlendAlpha = destBlendAlpha;
	pRT_Desc->BlendOpAlpha = blendOpAlpha;

	ID3D11BlendState* pBlendState = nullptr;

	HRESULT hr = pDevice3D_->CreateBlendState(&desc, &pBlendState);
	if (hr == S_OK)
		return pBlendState;

	return nullptr;
}


void RenderDX11::setBlendState(ID3D11BlendState* pBlendState)
{
	if (pCurrBlendState_ != pBlendState )
	{	pCurrBlendState_ = pBlendState;
		if (pCurrBlendState_)
		{	float BlendFactor[4] = {0};
			pDeviceContext_->OMSetBlendState( pCurrBlendState_, BlendFactor, 0xFFFFFFFF );
		}
	}
}


ID3D11SamplerState* RenderDX11::createSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE texAddress)
{
	D3D11_SAMPLER_DESC desc;
	desc.Filter = filter;
	desc.AddressU = desc.AddressV = desc.AddressW = texAddress;
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = 1;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	for (int i = 0; i < 4; ++i)
		desc.BorderColor[i] = 0.0f;
	desc.MinLOD = -D3D11_FLOAT32_MAX;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* pState = nullptr;

	HRESULT hr = pDevice3D_->CreateSamplerState( &desc, &pState );
	if (hr == S_OK)
		return pState;

	return nullptr;
}


void RenderDX11::setSampler(ID3D11SamplerState* pSampler, int slot)
{
	if (pCurrSamplers_[slot] != pSampler)
	{	pCurrSamplers_[slot] = pSampler;

		if (pCurrSamplers_[slot])
			pDeviceContext_->PSSetSamplers( slot, 1, &pCurrSamplers_[slot] );
	}
}


void RenderDX11::setDefaultSampler(int slot)
{
	setSampler(pDefaultSampler_, slot);
}


ID3D11RasterizerState* RenderDX11::createRasterState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool bMultisample)
{
	D3D11_RASTERIZER_DESC desc;
	desc.FillMode = fillMode;
	desc.CullMode = cullMode;
	desc.FrontCounterClockwise = false;
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0.0f;
	desc.SlopeScaledDepthBias = 0.0f;
	desc.DepthClipEnable = true;
	desc.ScissorEnable = false;
	desc.MultisampleEnable = bMultisample;
	desc.AntialiasedLineEnable = false;

	ID3D11RasterizerState* pState = nullptr;

	HRESULT hr = pDevice3D_->CreateRasterizerState(&desc, &pState);
	if (hr == S_OK)
		return pState;

	return nullptr;
}


void RenderDX11::setRasterState(ID3D11RasterizerState* pRasterState)
{
	if (pCurrRasterState_ != pRasterState)
	{	pCurrRasterState_ = pRasterState;

		if (pCurrRasterState_)
			pDeviceContext_->RSSetState(pCurrRasterState_);
	}
}


void RenderDX11::setDefaultRasterState()
{
	setRasterState(pDefaultRasterState_);
}


void RenderDX11::setRasterState(uint32_t rasterType)
{
	if (kRasterCullOff == rasterType)
	{	setRasterState(pCullOffRasterState_);
		return;
	}

	setDefaultRasterState();
}


void RenderDX11::setViewport(int x, int y, int w, int h)
{
	D3D11_VIEWPORT vp;
	vp.TopLeftX = static_cast<float>(x);
	vp.TopLeftY = static_cast<float>(y);
	vp.Width = static_cast<float>(w);
	vp.Height = static_cast<float>(h);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	setViewport(vp);
}


void RenderDX11::setViewport(const D3D11_VIEWPORT& vp)
{
	currViewPort_ = vp;
	pDeviceContext_->RSSetViewports(1, &currViewPort_);

	// screen projection for 2D sprites
	Vec4* pProj = static_cast<Vec4*>(openConstantBuffer(pSpriteCBufferVS_));
	if (pProj)
		*pProj = Vec4(2.0f / currViewPort_.Width, 2.0f / currViewPort_.Height, 0.0f, 1.0f );
	closeConstantBuffer(pSpriteCBufferVS_);
}


ShaderProgram RenderDX11::createShaderProgram(PCCHAR pVertText, PCCHAR pPixText)
{
	ShaderProgramDX11* pProgramDX11 = new ShaderProgramDX11;
	if (nullptr == pProgramDX11)
		return nullptr;

	D3D_SHADER_MACRO* pDefines = nullptr;

	ComPtr<ID3DBlob> pDxVsCode = pProgramDX11->compileShaderCode(pVertText, "vs_5_0", pDefines);
	if (pDxVsCode)
	{
		ID3D11VertexShader* pDxVertexShader = nullptr;
		HRESULT hr = pDevice3D_->CreateVertexShader(pDxVsCode->GetBufferPointer(), pDxVsCode->GetBufferSize(), nullptr, &pDxVertexShader);
		if (S_OK == hr)
		{
			//hr = pDxVertexShader->SetPrivateData( WKPDID_D3DDebugObjectName, strlen("vs"), "vs" );

			ComPtr<ID3DBlob> pDxPsCode = pProgramDX11->compileShaderCode(pPixText, "ps_5_0", pDefines);
			if (pDxPsCode)
			{
				ID3D11PixelShader* pDxPixelShader = nullptr;
				hr = pDevice3D_->CreatePixelShader(pDxPsCode->GetBufferPointer(), pDxPsCode->GetBufferSize(), nullptr, &pDxPixelShader);
				if (S_OK == hr)
				{
					//hr = pDxPixelShader->SetPrivateData( WKPDID_D3DDebugObjectName, strlen("ps"), "ps" );

					linkShaderProgram(pProgramDX11, pDxVertexShader, pDxPixelShader);

					ShaderProgram pProgram = std::shared_ptr<ShaderProgramDX11>(pProgramDX11);
					return pProgram;
				}
			}
		}
	}

	SAFE_DELETE(pProgramDX11);
	return nullptr;
}


ShaderProgram RenderDX11::createShaderProgram(Shader pVShader, PCCHAR pPixText)
{
	VertexShaderDX11 pVertexShader = std::dynamic_pointer_cast<VShaderDX11>(pVShader);
	if (nullptr == pVertexShader || nullptr == pPixText)
		return nullptr;

	ID3D11VertexShader* pDxVertexShader = pVertexShader->getDxShader();
	if (nullptr == pDxVertexShader)
		return nullptr;

	ShaderProgramDX11* pProgramDX11 = new ShaderProgramDX11;
	if (nullptr == pProgramDX11)
		return nullptr;

	D3D_SHADER_MACRO* pDefines = nullptr;

	ComPtr<ID3DBlob> pDxPsCode = pProgramDX11->compileShaderCode(pPixText, "ps_5_0", pDefines);
	if (pDxPsCode)
	{
		ID3D11PixelShader* pDxPixelShader = nullptr;
		HRESULT hr = pDevice3D_->CreatePixelShader(pDxPsCode->GetBufferPointer(), pDxPsCode->GetBufferSize(), nullptr, &pDxPixelShader);
		if (S_OK == hr)
		{
			linkShaderProgram(pProgramDX11, pDxVertexShader, pDxPixelShader);

			ShaderProgram pProgram = std::shared_ptr<ShaderProgramDX11>(pProgramDX11);
			return pProgram;
		}
	}

	return nullptr;
}


ShaderProgram RenderDX11::createShaderProgram(PCCHAR pVertText, Shader pPShader)
{
	PixelShaderDX11 pPixelShader = std::dynamic_pointer_cast<PShaderDX11>(pPShader);
	if (nullptr == pPixelShader || nullptr == pVertText)
		return nullptr;

	ID3D11PixelShader* pDxPixelShader = pPixelShader->getDxShader();
	if (nullptr == pDxPixelShader)
		return nullptr;

	ShaderProgramDX11* pProgramDX11 = new ShaderProgramDX11;
	if (nullptr == pProgramDX11)
		return nullptr;

	D3D_SHADER_MACRO* pDefines = nullptr;

	ComPtr<ID3DBlob> pDxVsCode = pProgramDX11->compileShaderCode(pVertText, "vs_5_0", pDefines);
	if (pDxVsCode)
	{
		ID3D11VertexShader* pDxVertexShader = nullptr;
		HRESULT hr = pDevice3D_->CreateVertexShader(pDxVsCode->GetBufferPointer(), pDxVsCode->GetBufferSize(), nullptr, &pDxVertexShader);
		if (S_OK == hr)
		{
			linkShaderProgram(pProgramDX11, pDxVertexShader, pDxPixelShader);

			ShaderProgram pProgram = std::shared_ptr<ShaderProgramDX11>(pProgramDX11);
			return pProgram;
		}
	}

	return nullptr;
}


ShaderProgram RenderDX11::createShaderProgram(cstring vsFile, cstring psFile, CShaderMacroStrings defines)
{
	ShaderProgramDX11* pProgramDX11 = new ShaderProgramDX11;
	if (nullptr == pProgramDX11)
		return nullptr;

	std::string path = cShaderFolder_ + vsFile;
	ID3D11VertexShader* pDxVertexShader = static_cast<ID3D11VertexShader*>(pProgramDX11->createShaderFromFile(path, "vs_5_0", defines, pDevice3D_));

	path = cShaderFolder_ + psFile;
	ID3D11PixelShader* pDxPixelShader = static_cast<ID3D11PixelShader*>(pProgramDX11->createShaderFromFile(path, "ps_5_0", defines, pDevice3D_));

	if (pDxVertexShader && pDxPixelShader)
	{
		linkShaderProgram(pProgramDX11, pDxVertexShader, pDxPixelShader);

		ShaderProgram pProgram = std::shared_ptr<ShaderProgramDX11>(pProgramDX11);
		return pProgram;
	}

	SAFE_DELETE(pProgramDX11);
	return nullptr;
}


void RenderDX11::linkShaderProgram(ShaderProgramDX11* pProgram, ID3D11VertexShader* pDxVShader, ID3D11PixelShader* pDxPShader)
{
	if (pProgram)
	{
		pProgram->pVertexShader_ = std::shared_ptr<VShaderDX11>(new VShaderDX11);
		pProgram->pPixelShader_ = std::shared_ptr<PShaderDX11>(new PShaderDX11);

		if (pProgram->pVertexShader_ && pProgram->pPixelShader_)
		{
			pProgram->pVertexShader_->pDxShader_ = pDxVShader;
			pProgram->pPixelShader_->pDxShader_ = pDxPShader;
		}
	}
}


void RenderDX11::setShaderProgram(ShaderProgram pProgram)
{
	if (pDeviceContext_ == nullptr)
		return;

	if (pCurrShaderProgram_ != pProgram)
	{	pCurrShaderProgram_ = pProgram;

		ShaderProgramDX11* pProgramDX11 = dynamic_cast<ShaderProgramDX11*>(pCurrShaderProgram_.get());
		if (pProgramDX11)
		{
			VertexShaderDX11 pVertexShader = std::dynamic_pointer_cast<VShaderDX11>(pProgramDX11->getVertexShader());
			PixelShaderDX11 pPixelShader = std::dynamic_pointer_cast<PShaderDX11>(pProgramDX11->getPixelShader());

			if (pVertexShader && pPixelShader)
			{
				pDeviceContext_->VSSetShader(pVertexShader->getDxShader(), nullptr, 0);
				pDeviceContext_->PSSetShader(pPixelShader->getDxShader(), nullptr, 0);
			}
		}
	}
}


ShaderProgram RenderDX11::getSpriteShaderProgram()
{
	return pSpriteShaderProgram_;
}


ID3D11ComputeShader* RenderDX11::createComputeShader(cstring csFile, CShaderMacroStrings defines)
{
	ShaderProgramDX11 program;
	std::string path = cShaderFolder_ + csFile;

	ID3D11ComputeShader* pDxComputeShader = static_cast<ID3D11ComputeShader*>(program.createShaderFromFile(path, "cs_5_0", defines, pDevice3D_));
	if (pDxComputeShader)
	{
		/*ComputeShaderDX11 pCompShader = std::shared_ptr<CShaderDX11>(new CShaderDX11);
		if (pCompShader)
		{	pCompShader->pDxShader_ = pDxComputeShader;
			return pCompShader;
		}*/

		return pDxComputeShader;
	}

	return nullptr;
}


void RenderDX11::setComputeShader(ID3D11ComputeShader* pShader)
{
	if (nullptr == pDeviceContext_ || nullptr == pShader)
		return;

	//IShader* pComputeShader = shader.get();
	//if (pComputeShader)
	{
		//ComputeShaderDX11 pComputeShaderDX11 = std::dynamic_pointer_cast<CShaderDX11>(shader);

		//if (pComputeShaderDX11)
		{
			pDeviceContext_->CSSetShader(pShader, nullptr, 0); //pComputeShaderDX11->getDxShader()
		}
	}
}


void RenderDX11::setUABuffer(UABufferDX11* buffer, uint32_t slot)
{
	UINT count = 0xFFFFFFFF;
	ID3D11UnorderedAccessView* ua_view = nullptr;

	//UABufferDX11* pBufferDX11 = dynamic_cast<UABufferDX11*>(buffer);
	if (buffer) //pBufferDX11
	{	ua_view = buffer->pDxUAView_; //pBufferDX11
	}

	pDeviceContext_->CSSetUnorderedAccessViews(slot, 1, &ua_view, &count);
}


void RenderDX11::setConstantBufferCS(ComputeBuffer buffer, UINT slot)
{
	if (buffer)
	{
		UABufferDX11* pUABuffer = static_cast<UABufferDX11*>(buffer->getNativeBuffer());
		if (pUABuffer)
		{
			ID3D11Buffer* pDxApiBuffer = pUABuffer->getDxBuffer();
			if (pDxApiBuffer)
				pDeviceContext_->CSSetConstantBuffers(slot, 1, &pDxApiBuffer);
		}
	}
}


/*void RenderDX11::bindUABufferToTextureVS(UABufferDX11* buffer, UINT slot)
{
	if (buffer)
	{	pDeviceContext_->VSSetShaderResources(slot, 1, &buffer->pDxShaderView_);
	}
	else
	{	ID3D11ShaderResourceView* pDxShaderView_[1] = { nullptr };
		pDeviceContext_->VSSetShaderResources(slot, 1, pDxShaderView_);
	}
}*/


void RenderDX11::setComputeTexture(Texture texture, uint32_t slot)
{
	if (slot >= 0 && slot < kMaxTextureSlots)
	{
		if (texture)
		{
			TextureDX11 pTextureDX = std::dynamic_pointer_cast<TextureDX11impl>(texture);

			if (pTextureDX)
				pDeviceContext_->CSSetShaderResources(slot, 1, &pTextureDX->pShaderView_);
		}
		else
		{	ID3D11ShaderResourceView* vw[1] = {nullptr};
			pDeviceContext_->CSSetShaderResources(slot, 1, vw);
		}
	}
}


void RenderDX11::setComputeSampler(uint32_t samplerType, uint32_t slot)
{
	if (kTexSamplerPointClamp == samplerType)
		pDeviceContext_->CSSetSamplers(slot, 1, &pPointClampSampler_);
	else if (kTexSamplerLinearWrap == samplerType)
		pDeviceContext_->CSSetSamplers(slot, 1, &pLinearWrapSampler_);
	else
		pDeviceContext_->CSSetSamplers(slot, 1, &pDefaultSampler_);
}


void RenderDX11::compute(uint32_t dim_x, uint32_t dim_y, uint32_t)
{
	pDeviceContext_->Dispatch(dim_x, dim_y, 1);

}


void AssembleCodeVS(PCCHAR cppType, PCCHAR variable, PCCHAR shaderType, std::string& vs_input, std::string& vs_code, UINT semantic)
{
	char chars[8];
	_itoa_s(semantic, chars, 10);
	std::string chDigit = chars;

	std::string chVariable = variable + chDigit;

	vs_input += cppType;
	vs_input += " " + chVariable + ": " + shaderType + chDigit + "; ";

	vs_code += "Out." + chVariable + " = Vertex." + chVariable + "; ";
}


VertexInput RenderDX11::createVertexInput(uint32_t vertexFlags, uint32_t instanceFlags)
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> input_elems;

	UINT byte_offset = 0;

	VertexInputDX11* pVertInputDX11 = nullptr;

	// Assemble the input elements and code for the shader blob.

	std::string vs_input("struct VS_INPUT  { ");
	std::string vs_code("VS_INPUT Main( VS_INPUT Vertex )  { VS_INPUT Out; ");

	uint32_t flags[2] = { vertexFlags, instanceFlags };

	BYTE strides[kMaxBindVertexBuffers] = {0};

	D3D11_INPUT_CLASSIFICATION input_class = D3D11_INPUT_PER_VERTEX_DATA;
	UINT slot = 0;
	UINT step_rate = 0;

	for (int i = 0; i < 2; ++i)
	{
		if (flags[i] == 0)
			continue;

		/*if (flags[i] & kInstanceId)
		{
			UINT semantic = 0;

			AssembleCodeVS("uint", "InstanceId", "INSTANCEID", vs_input, vs_code, semantic);

			D3D11_INPUT_ELEMENT_DESC desc = { "INSTANCEID", semantic, DXGI_FORMAT_R32_UINT, slot, byte_offset, input_class, step_rate };
			input_elems.push_back(desc);

			byte_offset += 4;
		}*/

		if (flags[i] & kVertPosition)
		{
			vs_input += "float4 Pos: POSITION; ";
			vs_code += "Out.Pos = Vertex.Pos; ";

			D3D11_INPUT_ELEMENT_DESC desc = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot, byte_offset, input_class, step_rate };
			input_elems.push_back(desc);

			byte_offset += 12;
		}

		if (flags[i] & kVertPosition2)
		{
			vs_input += "float4 Pos: POSITION; ";
			vs_code += "Out.Pos = Vertex.Pos; ";

			D3D11_INPUT_ELEMENT_DESC desc = { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, slot, byte_offset, input_class, step_rate };
			input_elems.push_back(desc);

			byte_offset += 8;
		}

		if (flags[i] & kVertNormal)
		{
			vs_input += "float3 Normal: NORMAL; ";
			vs_code += "Out.Normal = Vertex.Normal; ";

			D3D11_INPUT_ELEMENT_DESC desc = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot, byte_offset, input_class, step_rate };
			input_elems.push_back(desc);

			byte_offset += 12;
		}

		UINT semantic = 0;
		for (uint32_t flag = kVertTexCoord0;  flag <= kVertTexCoord3;  flag <<= 1, semantic++ )
		{
			if (flags[i] & flag)
			{
				AssembleCodeVS("float2", "TexCoord", "TEXCOORD", vs_input, vs_code, semantic);

				D3D11_INPUT_ELEMENT_DESC desc = { "TEXCOORD", semantic, DXGI_FORMAT_R32G32_FLOAT, slot, byte_offset, input_class, step_rate };
				input_elems.push_back(desc);

				byte_offset += 8;
			}
		}

		if (flags[i] & kVertBoneIndices)
		{
			vs_input += "uint4 Indices: BLENDINDICES; ";
			vs_code += "Out.Indices = Vertex.Indices; ";

			D3D11_INPUT_ELEMENT_DESC desc = { "BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, slot, byte_offset, input_class, step_rate };
			input_elems.push_back(desc);

			byte_offset += 4;
		}

		semantic = 0;
		for (uint32_t flag = kInstBegin;  flag >= kInstEnd;  flag >>= 1, semantic++ )
		{
			if (flags[i] & flag)
			{
				AssembleCodeVS("float4", "Instance4F", "INSTANCE4F", vs_input, vs_code, semantic);

				D3D11_INPUT_ELEMENT_DESC desc = { "INSTANCE4F", semantic, DXGI_FORMAT_R32G32B32A32_FLOAT, slot, byte_offset, input_class, step_rate };
				input_elems.push_back(desc);

				byte_offset += 16;
			}
		}

		semantic = 0;
		for (uint32_t flag = kVertColor0;  flag <= kVertColor1;  flag <<= 1, semantic++ )
		{
			if (flags[i] & flag)
			{
				AssembleCodeVS("float4", "Color", "COLOR", vs_input, vs_code, semantic);

				D3D11_INPUT_ELEMENT_DESC desc = { "COLOR", semantic, DXGI_FORMAT_R8G8B8A8_UNORM, slot, byte_offset, input_class, step_rate };
				input_elems.push_back(desc);

				byte_offset += 4;
			}
		}

		strides[i] = static_cast<BYTE>(byte_offset);
		byte_offset = 0;

		input_class = D3D11_INPUT_PER_INSTANCE_DATA;
		slot++;
		step_rate++;
	}

	if (input_elems.size() == 0)
		return nullptr;

	vs_code += "return Out; }";
	vs_input += "};\n\n";
	vs_input += vs_code;

	ShaderProgramDX11* pProgram = new ShaderProgramDX11;
	if (pProgram == nullptr)
		return nullptr;

	ID3DBlob* pVsCode = pProgram->compileShaderCode(vs_input.c_str(), "vs_5_0", nullptr);
	if (pVsCode)
	{
		ID3D11InputLayout* pInputLayout = nullptr;
		D3D11_INPUT_ELEMENT_DESC* pdesc = &input_elems[0];
		UINT nums = static_cast<UINT>(input_elems.size());
		HRESULT hr = pDevice3D_->CreateInputLayout(pdesc, nums, pVsCode->GetBufferPointer(), pVsCode->GetBufferSize(),
							&pInputLayout);
		if (hr == S_OK)
		{
			pVertInputDX11 = new VertexInputDX11;
			if (pVertInputDX11)
			{	pVertInputDX11->strides[0] = strides[0];
				pVertInputDX11->strides[1] = strides[1];
				pVertInputDX11->numVertexComponents = static_cast<BYTE>(input_elems.size());
				pVertInputDX11->pInputLayout_ = pInputLayout;
			}
		}

		SAFE_RELEASE_DX(pVsCode);
	}

	SAFE_DELETE(pProgram);

	VertexInput pVertInput = std::shared_ptr<VertexInputDX11>(pVertInputDX11);
	return pVertInput;
}


void RenderDX11::setVertexInput(VertexInput pVertexInput)
{
	if (pDeviceContext_ == nullptr)
		return;

	if (pCurrVertexInput_ != pVertexInput)
	{
		pCurrVertexInput_ = pVertexInput;

		if (pCurrVertexInput_)
		{	ID3D11InputLayout* pInputLayout = static_cast<ID3D11InputLayout*>(pCurrVertexInput_->getNativeVertexLayout());
			if (pInputLayout)
				pDeviceContext_->IASetInputLayout(pInputLayout);
		}
	}
}


void RenderDX11::releaseVertexInput(VertexInput& pVertexInput)
{
	if (pVertexInput)
		pVertexInput = nullptr;
}


ConstantBuffer RenderDX11::createConstantBuffer(size_t sizeBytes)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = static_cast<UINT>(sizeBytes);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	ComPtr<ID3D11Buffer> pDxApiBuffer = nullptr;
	HRESULT hr = pDevice3D_->CreateBuffer(&desc, nullptr, &pDxApiBuffer);
	if (hr == S_OK)
	{
		ConstantBufferDX11* pBufferDX11 = new ConstantBufferDX11;
		if (pBufferDX11)
		{	pDxApiBuffer.CopyTo(&pBufferDX11->pDxBuffer_);

			ConstantBuffer pBuff = std::shared_ptr<ConstantBufferDX11>(pBufferDX11);
			return pBuff;
		}
	}

	return nullptr;
}


void* RenderDX11::openConstantBuffer(ConstantBuffer pBuffer)
{
	if (pBuffer)
	{
		ID3D11Buffer* pDxApiBuffer = static_cast<ID3D11Buffer*>(pBuffer->getNativeBuffer());
		if (pDxApiBuffer)
		{
			void* pData = openDxApiBuffer(pDxApiBuffer);
			return pData;
		}
	}

	return nullptr;
}


void RenderDX11::closeConstantBuffer(ConstantBuffer pBuffer)
{
	if (pBuffer)
	{
		ID3D11Buffer* pDxApiBuffer = static_cast<ID3D11Buffer*>(pBuffer->getNativeBuffer());
		if (pDxApiBuffer)
			closeDxApiBuffer(pDxApiBuffer);
	}
}


void RenderDX11::updateConstantBuffer(ConstantBuffer pBuffer, void* pData, int dataSize)
{
	if (pBuffer && pData && dataSize > 0)
	{
		void* pDest = openConstantBuffer(pBuffer);
		if (pDest)
		{
			memcpy(pDest, pData, dataSize);

			closeConstantBuffer(pBuffer);
		}
	}
}


void RenderDX11::setConstantBuffers(ConstantBuffer pVertexCBuffer, ConstantBuffer pPixelCBuffer)
{
	if (pVertexCBuffer)
	{
		ID3D11Buffer* pDxApiBuffer = static_cast<ID3D11Buffer*>(pVertexCBuffer->getNativeBuffer());
		if (pDxApiBuffer)
			pDeviceContext_->VSSetConstantBuffers(0, 1, &pDxApiBuffer);
	}

	if (pPixelCBuffer)
	{
		ID3D11Buffer* pDxApiBuffer = static_cast<ID3D11Buffer*>(pPixelCBuffer->getNativeBuffer());
		if (pDxApiBuffer)
			pDeviceContext_->PSSetConstantBuffers(0, 1, &pDxApiBuffer);
	}
}


void RenderDX11::setConstantBufferVS(ConstantBuffer pCBuffer, UINT slot)
{
	if (pCBuffer)
	{
		ID3D11Buffer* pDxApiBuffer = static_cast<ID3D11Buffer*>(pCBuffer->getNativeBuffer());
		if (pDxApiBuffer)
			pDeviceContext_->VSSetConstantBuffers(slot, 1, &pDxApiBuffer);
	}
}


void RenderDX11::setConstantBufferPS(ConstantBuffer pCBuffer, UINT slot)
{
	if (pCBuffer)
	{
		ID3D11Buffer* pDxApiBuffer = static_cast<ID3D11Buffer*>(pCBuffer->getNativeBuffer());
		if (pDxApiBuffer)
			pDeviceContext_->PSSetConstantBuffers(slot, 1, &pDxApiBuffer);
	}
}


VertexBuffer RenderDX11::createVertexBuffer(uint32_t sizeBytes, bool bWritable, void* pVertices, int stride)
{
	ID3D11Buffer* pDxApiBuffer = createDxApiBuffer(D3D11_BIND_VERTEX_BUFFER, sizeBytes, bWritable, false, pVertices);
	if (pDxApiBuffer)
	{
		VertexBufferDX11* pBufferDX11 = new VertexBufferDX11;
		if (pBufferDX11)
		{	pBufferDX11->pDxBuffer_ = pDxApiBuffer;
			pBufferDX11->mStride_ = stride;

			VertexBuffer pBuff = std::shared_ptr<IVertexBuffer>(pBufferDX11);
			return pBuff;
			//return pBufferDX11;
		}
	}

	return nullptr;
}


void* RenderDX11::openVertexBuffer(VertexBuffer pBuffer)
{
	if (pBuffer)
	{
		ID3D11Buffer* pDxApiBuffer = static_cast<ID3D11Buffer*>(pBuffer->getNativeBuffer());
		if (pDxApiBuffer)
		{
			void* pData = openDxApiBuffer(pDxApiBuffer);
			return pData;
		}
	}

	return nullptr;
}


void RenderDX11::closeVertexBuffer(VertexBuffer pBuffer)
{
	if (pBuffer)
	{
		ID3D11Buffer* pDxApiBuffer = static_cast<ID3D11Buffer*>(pBuffer->getNativeBuffer());
		if (pDxApiBuffer)
			closeDxApiBuffer(pDxApiBuffer);
	}
}


void RenderDX11::setVertexBuffer(VertexBuffer pVertexBuffer, UINT slot)
{
	if (pCurrVertexBuffers_[slot] != pVertexBuffer)
	{	pCurrVertexBuffers_[slot] = pVertexBuffer;

		if (pVertexBuffer)
		{
			ID3D11Buffer* pDxApiBuffer = static_cast<ID3D11Buffer*>(pVertexBuffer->getNativeBuffer());
			if (pDxApiBuffer)
			{
				UINT offsets[1] = {0};
				UINT stride = pVertexBuffer->getVertexSize();

				pDeviceContext_->IASetVertexBuffers(slot, 1, &pDxApiBuffer, &stride, offsets);
			}
		}
		else
		{	ID3D11Buffer* pDxApiBuffer[1] = {nullptr};
			UINT offsets[1] = {0};
			UINT stride = 0;
			pDeviceContext_->IASetVertexBuffers(slot, 1, pDxApiBuffer, &stride, offsets);
		}
	}
}


IndexBuffer RenderDX11::createIndexBuffer(uint32_t sizeBytes, bool bWritable, bool bIndices32, void* pIndices)
{
	ID3D11Buffer* pDxApiBuffer = createDxApiBuffer(D3D11_BIND_INDEX_BUFFER, sizeBytes, bWritable, false, pIndices);
	if (pDxApiBuffer)
	{
		IndexBufferDX11* pBufferDX11 = new IndexBufferDX11;
		if (pBufferDX11)
		{	pBufferDX11->pDxBuffer_ = pDxApiBuffer;
			pBufferDX11->bIndices32_ = bIndices32;

			IndexBuffer pBuff = std::shared_ptr<IIndexBuffer>(pBufferDX11);
			return pBuff;
			//return pBufferDX11;
		}
	}

	return nullptr;
}


void* RenderDX11::openIndexBuffer(IndexBuffer pBuffer)
{
	if (pBuffer)
	{
		ID3D11Buffer* pDxApiBuffer = static_cast<ID3D11Buffer*>(pBuffer->getNativeBuffer());
		if (pDxApiBuffer)
		{
			void* pData = openDxApiBuffer(pDxApiBuffer);
			return pData;
		}
	}

	return nullptr;
}


void RenderDX11::closeIndexBuffer(IndexBuffer pBuffer)
{
	if (pBuffer)
	{
		ID3D11Buffer* pDxApiBuffer = static_cast<ID3D11Buffer*>(pBuffer->getNativeBuffer());
		if (pDxApiBuffer)
			closeDxApiBuffer(pDxApiBuffer);
	}
}


void RenderDX11::setIndexBuffer(IndexBuffer pIndexBuffer)
{
	if (pCurrIndexBuffer_ != pIndexBuffer)
	{	pCurrIndexBuffer_ = pIndexBuffer;

		if (pIndexBuffer)
		{
			ID3D11Buffer* pDxApiBuffer = static_cast<ID3D11Buffer*>(pIndexBuffer->getNativeBuffer());
			if (pDxApiBuffer)
			{
				if (pIndexBuffer->isIndices32())
					pDeviceContext_->IASetIndexBuffer(pDxApiBuffer, DXGI_FORMAT_R32_UINT, 0);
				else
					pDeviceContext_->IASetIndexBuffer(pDxApiBuffer, DXGI_FORMAT_R16_UINT, 0);
			}
		}
	}
}


ComputeBuffer RenderDX11::createComputeBuffer(int amountOfElems, int elemSize, uint32_t flags)
{
	UABufferDX11* pUABufferDX = createUABuffer(amountOfElems, elemSize, flags);
	if (pUABufferDX)
	{
		ComputeBufferDX11 pBuffer = std::shared_ptr<ComputeBufferDX11impl>(new ComputeBufferDX11impl);
		pBuffer->pUABuffer_ = pUABufferDX;
		return pBuffer;
	}
	return nullptr;
}


void RenderDX11::bindComputeBufferToTextureVS(ComputeBuffer buffer, UINT slot)
{
	/*ComputeBufferDX11 pBufferDX11 = std::dynamic_pointer_cast<ComputeBufferDX11impl>(buffer);
	if (pBufferDX11)
		bindUABufferToTextureVS(pBufferDX11->pUABuffer_, slot);
	else
		bindUABufferToTextureVS(nullptr, slot);*/

	if (buffer)
	{
		UABufferDX11* pUABuffer = static_cast<UABufferDX11*>(buffer->getGraphicsBuffer());
		if (pUABuffer)
		{	pDeviceContext_->VSSetShaderResources(slot, 1, &pUABuffer->pDxShaderView_);
			return;
		}
	}

	ID3D11ShaderResourceView* pDxShaderView_[1] = { nullptr };
	pDeviceContext_->VSSetShaderResources(slot, 1, pDxShaderView_);
}


ComputeBuffer RenderDX11::createConstantComputeBuffer(size_t sizeBytes)
{
	ComPtr<ID3D11Buffer> pDxBuffer = createDxApiBuffer(D3D11_BIND_CONSTANT_BUFFER, sizeBytes, true, false, nullptr);
	if (pDxBuffer)
	{
		ComputeBufferDX11 pBuffer = std::shared_ptr<ComputeBufferDX11impl>(new ComputeBufferDX11impl);
		pBuffer->pUABuffer_ = new UABufferDX11;
		if (pBuffer->pUABuffer_)
			pDxBuffer.CopyTo(&pBuffer->pUABuffer_->pDxBuffer_);

		return pBuffer;
	}
	return nullptr;
}


UABufferDX11* RenderDX11::createUABuffer(int amountOfElems, int elemSize, uint32_t flags)
{
	// Buffer
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = amountOfElems * elemSize;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	if (flags & IParallelCompute::kShared)
		desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;

	desc.StructureByteStride = elemSize;
	
	//ComPtr<ID3D11Buffer> pDxApiBuffer;
	ID3D11Buffer* pDxApiBuffer;
	HRESULT hr = pDevice3D_->CreateBuffer(&desc, nullptr, &pDxApiBuffer);
	if (S_OK != hr )
		return nullptr;

	// Unordered access view.
	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
    uav_desc.Format = DXGI_FORMAT_UNKNOWN;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav_desc.Buffer.FirstElement = 0;
    uav_desc.Buffer.NumElements = amountOfElems;
	uav_desc.Buffer.Flags = (flags & IParallelCompute::kAutoLength) ? D3D11_BUFFER_UAV_FLAG_APPEND : 0;

	//ComPtr<ID3D11UnorderedAccessView> pUAView;
	ID3D11UnorderedAccessView* pUAView;
	hr = pDevice3D_->CreateUnorderedAccessView(pDxApiBuffer, &uav_desc, &pUAView);
	if (S_OK != hr )
		return nullptr;

	// Shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
    srv_desc.Format = DXGI_FORMAT_UNKNOWN;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srv_desc.Buffer.FirstElement = 0;
    srv_desc.Buffer.ElementWidth = amountOfElems;

	//ComPtr<ID3D11ShaderResourceView> pSRView;
	ID3D11ShaderResourceView* pSRView;
	hr = pDevice3D_->CreateShaderResourceView(pDxApiBuffer, &srv_desc, &pSRView);
	if (S_OK != hr )
		return nullptr;

	// Our IUABuffer.
	UABufferDX11* pBufferDX11 = new UABufferDX11;
	if (nullptr == pBufferDX11)
		return nullptr;

	pBufferDX11->pDxBuffer_ = pDxApiBuffer;
	pBufferDX11->pDxUAView_ = pUAView;
	pBufferDX11->pDxShaderView_ = pSRView;

	return pBufferDX11;
}


ID3D11Buffer* RenderDX11::createDxApiBuffer(D3D11_BIND_FLAG bind, size_t sizeBytes, bool bWritable, bool bReadable, void* pData)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = static_cast<UINT>(sizeBytes);
	desc.BindFlags = bind;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	desc.CPUAccessFlags = 0;

	if (bWritable)
	{	desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	if (bReadable)
	{	desc.Usage = D3D11_USAGE_STAGING;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		if (bWritable)
			desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
	}

	if (!bWritable && !bReadable)
	{	if (pData)
			desc.Usage = D3D11_USAGE_IMMUTABLE;
		else
			desc.Usage = D3D11_USAGE_DEFAULT;
	}

if (bind == D3D11_BIND_SHADER_RESOURCE)
{	desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED; //D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.StructureByteStride = 16;
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
}

	D3D11_SUBRESOURCE_DATA* pInitData = nullptr;;
	D3D11_SUBRESOURCE_DATA vbInitData;
	if (pData)
	{	::ZeroMemory(&vbInitData, sizeof(vbInitData));
		vbInitData.pSysMem = pData;
		pInitData = &vbInitData;
	}

	ID3D11Buffer* pDxApiBuffer = nullptr;
	HRESULT hr = pDevice3D_->CreateBuffer(&desc, pInitData, &pDxApiBuffer);
	if (S_OK == hr)
		return pDxApiBuffer;
	else
		return nullptr;
}


void* RenderDX11::openDxApiBuffer(ID3D11Buffer* pDxApiBuffer)
{
	if (pDxApiBuffer)
	{
		D3D11_BUFFER_DESC desc;
		pDxApiBuffer->GetDesc(&desc);
		D3D11_MAP map;
		if (desc.CPUAccessFlags == D3D11_CPU_ACCESS_READ)
			map = D3D11_MAP_READ;
		else
			map = D3D11_MAP_WRITE_DISCARD;

		D3D11_MAPPED_SUBRESOURCE Mapped;
		HRESULT hr = pDeviceContext_->Map(pDxApiBuffer, 0, map, 0, &Mapped);
		if (hr == S_OK)
			return Mapped.pData;    
	}

	return nullptr;
}


void RenderDX11::closeDxApiBuffer(ID3D11Buffer* pDxApiBuffer)
{
	if (pDxApiBuffer)
		pDeviceContext_->Unmap(pDxApiBuffer, 0);
}


void RenderDX11::copyBuffers(ID3D11Buffer* pDestBuffer, ID3D11Buffer* pSourBuffer)
{
	if(pDestBuffer && pSourBuffer)
		pDeviceContext_->CopyResource(pDestBuffer, pSourBuffer);
}


void RenderDX11::drawSprite(CPoint pos, Texture pTexture, CRect rect, uint32_t flags)
{
	const Rect* prect = (rect.x1 == 0 && rect.x2 == -1) ? nullptr : &rect;

	drawSprites(1, &pos, prect, pTexture, flags);
}


void RenderDX11::drawSprite(CPoint pos, Texture pTexture, CRect rect, uint32_t color, uint32_t flags)
{
	const Rect* prect = (rect.x1 == 0 && rect.x2 == -1) ? nullptr : &rect;

	drawSprites(1, &pos, prect, pTexture, color, flags);
}


void RenderDX11::drawSprites(int numSprites, const Point* pPoints, const Rect* pRects, Texture pTexture, uint32_t flags)
{
	drawSprites(numSprites, pPoints, pRects, pTexture, 0xFFFFFFFF, flags);
}


void RenderDX11::drawSprites(int numSprites, const Point* pPoints, const Rect* pRects, Texture pTexture, uint32_t color, uint32_t flags)
{
	if (nullptr == pTexture)
		return;

	//TextureDX11* pTextureDX = dynamic_cast<TextureDX11*>(pTexture);
	TextureDX11 pTextureDX = std::dynamic_pointer_cast<TextureDX11impl>(pTexture);
	if (nullptr == pTextureDX)
		return;

	if (flags & kAlphaBlendNormal)
	{	setAlphaBlendNormal();
	}
	else
	{	if (flags & kAlphaBlendAdditive)
			setAlphaBlendAdditive();
		else
			setAlphaBlendOff();
	}

	if (flags & kTexSamplerPointClamp)
		setSampler(pPointClampSampler_, 0);
	else
		setSampler(pDefaultSampler_, 0);

	setEnableDepthBuffer(false);

	setRasterState(pCullOffRasterState_);

	setTexture(0, pTexture);

	setVertexInput(pSpriteVertInput_);

	//Vec4 Color(1,1,1,1);
	//updateConstantBuffer(pSpriteCBufferPS_, &Color, sizeof(Color));

	if (!(flags &IRender::kCustomShaderProgram))
	{
		setShaderProgram(pSpriteShaderProgram_);
	}

	setConstantBuffers(pSpriteCBufferVS_, nullptr); //pSpriteCBufferPS_);

	setVertexBuffer(pSpriteVertexBuffer_, 0);

	setIndexBuffer(pSpriteIndexBuffer_);

	ID3D11Buffer* pDxApiBufferInst = static_cast<ID3D11Buffer*>(pSpriteVBufferInst_->getNativeBuffer());
	if (nullptr == pDxApiBufferInst)
		return;
	
	SpriteInstance* pInstances = static_cast<SpriteInstance*>(openDxApiBuffer(pDxApiBufferInst));
	if (nullptr == pInstances)
		return;
		
	float half_w = currViewPort_.Width / 2.0f;
	float half_h = currViewPort_.Height / 2.0f;

	int tex_width = 1, tex_height = 1;
	pTexture->getDimentions(tex_width, tex_height);

	Vec4 du_dv;
	if (pRects)
	{	float du = 1.0f / static_cast<float>(tex_width);
		float dv = 1.0f / static_cast<float>(tex_height);
		du_dv = Vec4(du, dv, du, dv);
	}

	int n = 0;
	for (int sp = 0; sp < numSprites; ++sp)
	{
		pInstances[n].sizePos.z = static_cast<float>(pPoints[sp].x - half_w);
		pInstances[n].sizePos.w = static_cast<float>(-pPoints[sp].y + half_h);//+1.0f;

		if (pRects)
		{
			float rect_w = static_cast<float>(pRects[sp].width());
			float rect_h = static_cast<float>(pRects[sp].height());

			// uv multiply
			pInstances[n].uvMulAdd.x = rect_w;// + 1.0f;
			pInstances[n].uvMulAdd.y = rect_h;// + 1.0f;
			// uv add
			pInstances[n].uvMulAdd.z = static_cast<float>(pRects[sp].x1);
			pInstances[n].uvMulAdd.w = static_cast<float>(pRects[sp].y1);
			pInstances[n].uvMulAdd *= du_dv;
			// size
			pInstances[n].sizePos.x = rect_w;// + 1.0f;
			pInstances[n].sizePos.y = rect_h;// + 1.0f;
		}
		else
		{
			// uv multiply add
			pInstances[n].uvMulAdd = Vec4(1,1,0,0);
			// size
			pInstances[n].sizePos.x = static_cast<float>(tex_width);
			pInstances[n].sizePos.y = static_cast<float>(tex_height);
		}

		pInstances[n].color = color;

		if (++n >= kMaxSpritesPerDIP)
		{
			closeDxApiBuffer(pDxApiBufferInst);
			setVertexBuffer(pSpriteVBufferInst_, 1);

			drawIndexedTrianglesInstanced(2, n);

			pInstances = static_cast<SpriteInstance*>(openDxApiBuffer(pDxApiBufferInst));
			n = 0;
		}
	}

	if (n > 0)
	{
		closeDxApiBuffer(pDxApiBufferInst);
		setVertexBuffer(pSpriteVBufferInst_, 1);

		drawIndexedTrianglesInstanced(2, n);
	}
}


void RenderDX11::drawIndexedTriangles(int numTriangles)
{
	if (mCurrTopology != D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
	{	mCurrTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		pDeviceContext_->IASetPrimitiveTopology(mCurrTopology);
	}

	pDeviceContext_->GSSetShader(nullptr, nullptr, 0);
	
	pDeviceContext_->DrawIndexed(3 * numTriangles, 0, 0);
}


void RenderDX11::drawIndexedTrianglesInstanced(int numTriangles, int numInstances)
{
	if (mCurrTopology != D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
	{	mCurrTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		pDeviceContext_->IASetPrimitiveTopology(mCurrTopology);
	}

	pDeviceContext_->GSSetShader(nullptr, nullptr, 0);
	
	pDeviceContext_->DrawIndexedInstanced(3 * numTriangles, numInstances, 0, 0, 0);
}


void RenderDX11::prepareDrawing()
{
	pDeviceContext_->ClearRenderTargetView(pCurrRenderTargetView_, clearColor_);

	pDeviceContext_->ClearDepthStencilView(pCurrDepthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// default states
	setAlphaBlendOff();
	setEnableDepthBuffer(true);
	setEnableDepthWrite(true);
	setSampler(pDefaultSampler_, 0);
	setRasterState(pDefaultRasterState_);
}


void RenderDX11::presentDrawing()
{
	if (pStatisticTexture_)
	{
	}

	if (pSwapChain_)
	{
		HRESULT hr = pSwapChain_->Present(0, 0);
		if (hr != S_OK)
		{
		}
	}
}