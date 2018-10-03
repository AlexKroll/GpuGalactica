#pragma once

#include <d3d11.h>

#include "Buffers.h"




class ConstantBufferDX11 : public IConstantBuffer
{
public:
	virtual void* getNativeBuffer() override final
	{
		return pDxBuffer_;
	}

	virtual ~ConstantBufferDX11()
	{
		SAFE_RELEASE_DX(pDxBuffer_);
	}

private:
	ID3D11Buffer* pDxBuffer_ = nullptr;

	friend class RenderDX11;
};



class VertexBufferDX11 : public IVertexBuffer
{
public:
	virtual void* getNativeBuffer() override final
	{
		return pDxBuffer_;
	}

	virtual UINT getVertexSize() override final
	{
		return mStride_;
	}

	virtual ~VertexBufferDX11()
	{
		SAFE_RELEASE_DX(pDxBuffer_);
	}

private:
	ID3D11Buffer* pDxBuffer_ = nullptr;

	UINT mStride_ = 0;

	friend class RenderDX11;
};



class IndexBufferDX11 : public IIndexBuffer
{
public:
	virtual void* getNativeBuffer() override final
	{
		return pDxBuffer_;
	}

	virtual ~IndexBufferDX11()
	{
		SAFE_RELEASE_DX(pDxBuffer_);
	}

	virtual bool isIndices32() override final
	{
		return bIndices32_;
	}

private:
	ID3D11Buffer* pDxBuffer_ = nullptr;

	bool bIndices32_ = false;

	friend class RenderDX11;
};



class UABufferDX11 : public IUABuffer
{
public:
	virtual void* getNativeBuffer() override final
	{
		return pDxBuffer_;
	}

	virtual ~UABufferDX11()
	{
		SAFE_RELEASE_DX(pDxBuffer_);
		SAFE_RELEASE_DX(pDxUAView_);
		SAFE_RELEASE_DX(pDxShaderView_);
	}

private:
	ID3D11Buffer* pDxBuffer_ = nullptr;
	ID3D11UnorderedAccessView* pDxUAView_ = nullptr;
	ID3D11ShaderResourceView* pDxShaderView_ = nullptr;

	friend class RenderDX11;
};