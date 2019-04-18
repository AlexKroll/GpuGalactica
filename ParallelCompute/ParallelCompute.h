#pragma once

#include <windows.h>
#include <memory>

#include "../Framework/Utils/Utils.h"
#include "../Framework/Graphics/Render.h"
#include "ComputeItems.h"
#include "../Framework/ComputeBuffer.h"


// Interface parallel computing. It uses the hardware accelerator (GPU, Cell, DSP etc).
// It is the base of DX, OpenCL, cuda APIs.

struct IParallelCompute abstract
{
	enum Type
	{
		UNKNOWN,
		DX11,
		OPENCL,
		CUDA
	};

	static const uint32_t kBufferReadWrite = 1;	// The compute unit can read/write to buffer.
	static const uint32_t kBufferWrite = 2;		// The compute unit can write to buffer.
	static const uint32_t kBufferRead = 4;		// The compute unit can read to buffer.
	static const uint32_t kShared = 8;			// The compute unit shares the resource with graphics (DX/GL).
	static const uint32_t kAutoLength = 16;		// The compute buffer has an auto-length counter.
	static const uint32_t kCpuNoAccess = 32;	// The CPU won`t read/write to the buffer.

	virtual ~IParallelCompute() {}

	static IParallelCompute* getCompute(Type type);

	virtual int init(Render pRender) = 0;

	virtual void release() = 0;

	Type getType() const;

	//virtual Render getRender() = 0;

	// Creates built-in memory buffer.
	virtual ComputeBuffer createBuffer(int amountOfElems, int elemSize, uint32_t flags) = 0;
		// The buffer is treated as an array of elements.
		// amountOfElems, elemSize - number of elements and size of one element.
		// flags - IParallelCompute::constants (see above).

	// Receive the buffer data after processing.
	template<class Type>
	std::vector<Type> debugDumpBuffer(ComputeBuffer buffer)
	{
		std::vector<Type> dump;

		if (buffer)
		{	size_t bytes = buffer->getSizeBytes();
			size_t elems = bytes / sizeof(Type);
			if (elems)
			{
				dump.resize(elems);
				debugDumpBuffer(dump.data(), buffer);
			}
		}

		return dump;
	}

	virtual int debugDumpBuffer(void* dest, ComputeBuffer buffer) = 0;

	virtual Texture createImage(UINT width, UINT height, ITexture::PixelFormat format, ITexture::Type type, uint32_t flags, BYTE* pPixels) = 0;

	virtual int createImageIntoTexture(Texture texture, uint32_t flags) = 0;

	virtual void releaseImage(Texture& texture) = 0;

	virtual ComputeTask createComputeTask(cstring csFile, CShaderMacroStrings defines) = 0;

	virtual void setComputeTask(ComputeTask task) = 0;

	template<class Type>
	void writeBuffer(ComputeBuffer buffer, const Type& data)
	{
		writeBuffer(buffer, &data, sizeof(Type));
	}

	virtual ComputeBuffer createConstantBuffer(size_t bytes) = 0;

	virtual void writeBuffer(ComputeBuffer buffer, const void* data, size_t bytes) = 0;

	virtual void setConstantBuffer(ComputeBuffer buffer, uint32_t slot) = 0;

	virtual void setComputeBuffer(ComputeBuffer buffer, uint32_t slot) = 0;

	//virtual void bindBufferToTextureVS(ComputeBuffer buffer, UINT slot) = 0;

	virtual void setImage(Texture texture, uint32_t slot) = 0;

	virtual void setSampler(uint32_t samplerType, uint32_t slot) = 0;

	virtual void compute(uint32_t dim_x, uint32_t dim_y, uint32_t num_groups) = 0;

	virtual int finish() = 0;

protected:
	Type type_ = UNKNOWN;

	HWND hWindow_ = nullptr;

	int screenW_ = 0;
	int screenH_ = 0;
};

typedef std::shared_ptr<IParallelCompute> ParallelCompute;

