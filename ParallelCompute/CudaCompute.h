#pragma once

#include <windows.h>

#include "ParallelCompute.h"



class CudaCompute : public IParallelCompute
{
public:
	virtual int init(Render pRender) override;

	virtual void release() override;

	//virtual Render getRender() override { return nullptr; }

	virtual ComputeBuffer createBuffer(int, int, uint32_t) override { return nullptr; }

	virtual int debugDumpBuffer(void*, ComputeBuffer) override { return 0; }

	virtual Texture createImage(UINT, UINT, ITexture::PixelFormat, ITexture::Type, uint32_t, BYTE*) override { return nullptr; }

	virtual int createImageIntoTexture(Texture, uint32_t) override { return 0; }

	virtual void releaseImage(Texture&) {}

	virtual ComputeTask createComputeTask(cstring, CShaderMacroStrings) override { return nullptr; }

	virtual void setComputeTask(ComputeTask) override { }

	virtual ComputeBuffer createConstantBuffer(size_t) override { return nullptr; }

	virtual void writeBuffer(ComputeBuffer, const void*, size_t) override {}

	virtual void setConstantBuffer(ComputeBuffer, uint32_t) override { }

	virtual void setComputeBuffer(ComputeBuffer, uint32_t) override { }

	//virtual void bindBufferToTextureVS(ComputeBuffer, UINT) override { }

	virtual void setImage(Texture, uint32_t) override { }

	virtual void setSampler(uint32_t, uint32_t) override { }

	virtual void compute(uint32_t, uint32_t, uint32_t) override { }

	virtual int finish() override { return 0; }

	friend struct IParallelCompute;

private:
	CudaCompute();
	~CudaCompute();

	CudaCompute(const CudaCompute&) = delete;
	CudaCompute& operator= (const CudaCompute&) = delete;
};
