#pragma once

#include <windows.h>
#include <memory>

#include "../Framework/Graphics/Render.h"


// Interface for parallel computing.

struct IParallelCompute abstract
{
	enum  Type
	{
		NONE,
		DX11,
		OPENCL,
		CUDA
	};

	virtual ~IParallelCompute() {}

	static IParallelCompute* getCompute(Type type);

	virtual int init(Render pRender) = 0;

	virtual void release() = 0;

	//virtual Render getRender() = 0;

	virtual UABuffer createUABuffer(int amountOfElems, int elemSize, bool bGPUlength) = 0;

	virtual Shader createComputeProgram(cstring csFile, CShaderMacroStrings defines) = 0;

	virtual void setComputeProgram(Shader pProgram) = 0;

	template<class Type>
	void writeConstantBuffer(ConstantBuffer buffer, const Type& data)
	{
		writeConstantBuffer(buffer, &data, sizeof(Type));
	}

	virtual void writeConstantBuffer(ConstantBuffer buffer, const void* data, size_t bytes) = 0;

	virtual void setConstantBuffer(ConstantBuffer buffer, uint32_t slot) = 0;

	virtual void setComputeBuffer(UABuffer buffer, uint32_t slot) = 0;

	virtual void bindUABufferToTextureVS(UABuffer buffer, UINT slot) = 0;

	virtual void setTexture(Texture texture, uint32_t slot) = 0;

	virtual void setSampler(uint32_t samplerType, uint32_t slot) = 0;

	virtual void compute(UINT x, UINT y, UINT z) = 0;

protected:
	HWND hWindow_ = nullptr;

	int screenW_ = 0;
	int screenH_ = 0;
};

typedef std::shared_ptr<IParallelCompute> ParallelCompute;

