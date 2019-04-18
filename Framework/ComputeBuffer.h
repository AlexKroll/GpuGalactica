#pragma once

#include <windows.h>
#include <memory>



// Onboard buffer used by hardware accelerator (GPU, Cell, DSP etc).
class IComputeBuffer abstract
{
public:
	virtual void* getNativeBuffer() = 0;		// Return the compute API buffer (CL, cuda, DX).

	virtual void* getGraphicsBuffer() = 0;		// Return the graphics API buffer (DX, GL, Vulkan).

	virtual size_t getSizeBytes() = 0;

	virtual ~IComputeBuffer() = default;

	void shareBuffer()  { bShared_ = true; }	// Share the buffer between a compute API and graphics API.

	bool isShared() const  { return bShared_; }

protected:
	bool bShared_ = false;  // The resource is shared between graphics and compute APIs.
};

typedef std::shared_ptr<IComputeBuffer> ComputeBuffer;
