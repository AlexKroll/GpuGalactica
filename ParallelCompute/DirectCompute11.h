#pragma once

#include <windows.h>

#include "../Framework/Graphics/RenderDX11.h"

#include "ParallelCompute.h"



class DirectCompute11 : public IParallelCompute
{
public:
	virtual int init(Render pRender) override;

	virtual void release() override;

	//virtual Render getRender() override;

	virtual UABuffer createUABuffer(int amountOfElems, int elemSize, bool bGPUlength) override;

	virtual Shader createComputeProgram(cstring csFile, CShaderMacroStrings defines) override;

	virtual void setComputeProgram(Shader pProgram) override;

	virtual void writeConstantBuffer(ConstantBuffer buffer, const void* data, size_t bytes) override;

	virtual void setConstantBuffer(ConstantBuffer buffer, uint32_t slot) override;

	virtual void setComputeBuffer(UABuffer buffer, uint32_t slot) override;

	virtual void bindUABufferToTextureVS(UABuffer buffer, UINT slot) override;

	virtual void setTexture(Texture texture, uint32_t slot) override;

	virtual void setSampler(uint32_t samplerType, uint32_t slot) override;

	virtual void compute(UINT x, UINT y, UINT z) override;

	friend struct IParallelCompute;

private:
	DirectCompute11();
	~DirectCompute11();

	DirectCompute11(const DirectCompute11&) = delete;
	DirectCompute11& operator= (const DirectCompute11&) = delete;

	RenderDX11* pRender_ = nullptr;

	// Compute states.
	Shader pCurrComputeProgram_ = nullptr;
};