#pragma once

#include <d3d11.h>
#include <dxgi.h>


struct RenderContext
{
	ID3D11RenderTargetView* pRenderTargetView = nullptr;

	ID3D11DepthStencilView* pDepthStencilView = nullptr;

	D3D11_VIEWPORT viewPort;

	RenderContext()
	{
		viewPort.TopLeftX = viewPort.TopLeftY = 0.0f;

		viewPort.Width = viewPort.Height = 0.0f;

		viewPort.MinDepth = 0.0f;
		viewPort.MaxDepth = 1.0f;
	}
};



class VertexInputDX11 : public IVertexInput
{
public:
	virtual void* getNativeVertexLayout() final
	{
		return pInputLayout_;
	}

	virtual ~VertexInputDX11()
	{
		SAFE_RELEASE_DX(pInputLayout_);
	}

private:
	ID3D11InputLayout* pInputLayout_ = nullptr;

	friend class RenderDX11;
};



