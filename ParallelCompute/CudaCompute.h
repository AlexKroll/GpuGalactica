#pragma once

#include <windows.h>

#include "ParallelCompute.h"



class CudaCompute : public IParallelCompute
{
public:
	virtual int init(Render pRender) override;

	virtual void release() override;

	//virtual Render getRender() override { return nullptr; }

	virtual UABuffer createUABuffer(int, int, bool) override { return nullptr; }

	virtual Shader createComputeProgram(cstring, CShaderMacroStrings) override { return nullptr; }

	virtual void setComputeProgram(Shader) override { }

	virtual void writeConstantBuffer(ConstantBuffer, const void*, size_t) override {}

	virtual void setConstantBuffer(ConstantBuffer, uint32_t) override { }

	virtual void setComputeBuffer(UABuffer, uint32_t) override { }

	virtual void bindUABufferToTextureVS(UABuffer, UINT) override { }

	virtual void setTexture(Texture, uint32_t) override { }

	virtual void setSampler(uint32_t, uint32_t) override { }

	virtual void compute(UINT, UINT, UINT) override { }

	friend struct IParallelCompute;

private:
	CudaCompute();
	~CudaCompute();

	CudaCompute(const CudaCompute&) = delete;
	CudaCompute& operator= (const CudaCompute&) = delete;
};
