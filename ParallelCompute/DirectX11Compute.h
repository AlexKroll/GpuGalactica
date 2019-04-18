#pragma once

#include <windows.h>

#include "../Framework/Graphics/RenderDX11.h"

#include "ParallelCompute.h"



// Compute module that uses the DirectX11 GPU device.

class DirectX11Compute : public IParallelCompute
{
public:
	virtual int init(Render pRender) override;

	virtual void release() override;

	//virtual Render getRender() override;

	virtual ComputeBuffer createBuffer(int amountOfElems, int elemSize, uint32_t flags) override;

	virtual int debugDumpBuffer(void* dest, ComputeBuffer buffer) override;

	virtual Texture createImage(UINT width, UINT height, ITexture::PixelFormat format, ITexture::Type type, uint32_t flags, BYTE* pPixels) override;

	virtual int createImageIntoTexture(Texture, uint32_t) override { return 0; }

	virtual void releaseImage(Texture& texture) override;

	virtual ComputeTask createComputeTask(cstring csFile, CShaderMacroStrings defines) override;

	virtual void setComputeTask(ComputeTask task) override;

	virtual ComputeBuffer createConstantBuffer(size_t bytes) override;

	virtual void writeBuffer(ComputeBuffer buffer, const void* data, size_t bytes) override;

	virtual void setConstantBuffer(ComputeBuffer, uint32_t) override;

	virtual void setComputeBuffer(ComputeBuffer buffer, uint32_t slot) override;

	//virtual void bindBufferToTextureVS(ComputeBuffer buffer, UINT slot) override;

	virtual void setImage(Texture texture, uint32_t slot) override;

	virtual void setSampler(uint32_t samplerType, uint32_t slot) override;

	virtual void compute(uint32_t dim_x, uint32_t dim_y, uint32_t num_groups) override;

	virtual int finish() override { return 0; }

	friend struct IParallelCompute;

private:
	DirectX11Compute();
	~DirectX11Compute();

	DirectX11Compute(const DirectX11Compute&) = delete;
	DirectX11Compute& operator= (const DirectX11Compute&) = delete;

	RenderDX11* pRender_ = nullptr;

	// Compute states.
	Shader pCurrComputeProgram_ = nullptr;
};



// Buffer for access by DX11 device.
class ComputeBufferDX11impl : public IComputeBuffer
{
public:
	virtual void* getNativeBuffer() override final
	{
		return pUABuffer_;
	}

	virtual void* getGraphicsBuffer() override
	{
		return pUABuffer_;		// DX compute buffer is graphics buffer
	}

	virtual size_t getSizeBytes() override final
	{
		if (pUABuffer_)
		{
			ID3D11Buffer* dxbuff = pUABuffer_->getDxBuffer();
			if (dxbuff)
			{
				D3D11_BUFFER_DESC desc;
				dxbuff->GetDesc(&desc);
				return desc.ByteWidth;
			}
		}

		return 0;
	}

	virtual ~ComputeBufferDX11impl()
	{
		if (pUABuffer_)
			pUABuffer_->release();
		pUABuffer_ = nullptr;
	}

private:
	UABufferDX11* pUABuffer_ = nullptr;

	friend class DirectX11Compute;
	friend class RenderDX11;
};

typedef std::shared_ptr<ComputeBufferDX11impl> ComputeBufferDX11;




class ComputeTaskDX11impl : public IComputeTask
{
public:
	virtual void release() final
	{
		if (pShader_)
			pShader_->Release();
		pShader_ = nullptr;
	}

private:
	static const bool bDebugEnabled_ = false;

	ID3D11ComputeShader* pShader_ = nullptr;

public:
	ComputeTaskDX11impl() {}

	virtual ~ComputeTaskDX11impl()
	{
		release();
	}

	friend class DirectX11Compute;
};

typedef std::shared_ptr<ComputeTaskDX11impl> ComputeTaskDX11;


