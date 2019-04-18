#pragma once

#include <windows.h>

#include "ParallelCompute.h"

#include "OpenCL/opencl.h"
#include "OpenCL/cl_d3d11.h"
#include "OpenCL/cl_d3d11_ext.h"

#include "../Framework/Graphics/RenderDX11.h"


// The OpenCL interface.
// For interoperating with graphics, we need to acquire the resource for computing. Unfortunately it reduces the performance.

class OpenCLCompute : public IParallelCompute
{
public:
	virtual int init(Render pRender) override;

	virtual void release() override;

	//virtual Render getRender() override { return nullptr; }

	// Creates built-in memory buffer.
	virtual ComputeBuffer createBuffer(int amountOfElems, int elemSize, uint32_t flags) override;
	// The buffer is treated as an array of elements.
		// amountOfElems, elemSize - number of elements and size of one element.
		// flags - IParallelCompute::constants (see IParallelCompute).

	virtual int debugDumpBuffer(void* dest, ComputeBuffer buffer) override;

	virtual Texture createImage(UINT width, UINT height, ITexture::PixelFormat format, ITexture::Type type, uint32_t flags, BYTE* pPixels) override;

	virtual int createImageIntoTexture(Texture texture, uint32_t flags) override;

	virtual void releaseImage(Texture& texture) override;

	virtual ComputeTask createComputeTask(cstring csFile, CShaderMacroStrings defines) override;

	virtual void setComputeTask(ComputeTask task) override;

	virtual ComputeBuffer createConstantBuffer(size_t bytes) override;

	virtual void writeBuffer(ComputeBuffer buffer, const void* data, size_t bytes) override;

	virtual void setConstantBuffer(ComputeBuffer buffer, uint32_t slot) override;

	virtual void setComputeBuffer(ComputeBuffer, uint32_t) override;

	//virtual void bindBufferToTextureVS(ComputeBuffer, UINT) override { }

	virtual void setImage(Texture texture, uint32_t slot) override;

	virtual void setSampler(uint32_t, uint32_t) override { }

	virtual void compute(uint32_t dim_x, uint32_t dim_y, uint32_t num_groups) override;

	virtual int finish() override;

	friend struct IParallelCompute;

private:
	OpenCLCompute();
	virtual ~OpenCLCompute();

	OpenCLCompute(const OpenCLCompute&) = delete;
	OpenCLCompute& operator= (const OpenCLCompute&) = delete;

	cl_mem_flags mapFlags(uint32_t flags);

	cl_platform_id platform_ = nullptr;
	cl_device_id device_ = nullptr;
	cl_context context_ = nullptr;
	cl_command_queue commandQueue_ = nullptr;

	cl_program currProgram_ = nullptr;
	cl_kernel currKernel_ = nullptr;

	std::string cKernelsFolder_;
	std::string programFolder_;

	std::vector<cl_mem> acquiredResourses_;

	cl_image_format imageFormats_[ITexture::MaxFormats] = {0};

	Render pRender_ = nullptr;

	HMODULE clDll_ = nullptr;

public:	
	// OpenCL functions by dynamic binding.
	cl_int (__stdcall* clFlush)(cl_command_queue) = nullptr;
	cl_int (__stdcall* clFinish)(cl_command_queue) = nullptr;
	cl_int (__stdcall* clReleaseCommandQueue)(cl_command_queue) = nullptr;
	cl_int (__stdcall* clReleaseContext)(cl_context) = nullptr;
	cl_int (__stdcall* clReleaseDevice) (cl_device_id) = nullptr;
	cl_int (__stdcall* clGetPlatformIDs) (cl_uint, cl_platform_id*, cl_uint*) = nullptr;
	cl_int (__stdcall* clGetPlatformInfo) (cl_platform_id, cl_platform_info, size_t, void*, size_t*) = nullptr;
	void*  (__stdcall* clGetExtensionFunctionAddressForPlatform) (cl_platform_id, const char*) = nullptr;
	cl_int (__stdcall* clGetDeviceIDs) (cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint*) = nullptr;
	cl_int (__stdcall* clGetDeviceInfo) (cl_device_id, cl_device_info, size_t, void *, size_t *) = nullptr;
	cl_context (__stdcall* clCreateContext) (const cl_context_properties*, cl_uint, const cl_device_id*,
												void (CL_CALLBACK*)(const char*, const void*, size_t, void *), void*, cl_int*) = nullptr;
	cl_command_queue (__stdcall* clCreateCommandQueue) (cl_context, cl_device_id, cl_command_queue_properties, cl_int *) = nullptr;
	cl_mem (__stdcall* clCreateBuffer) (cl_context, cl_mem_flags, size_t, void*, cl_int*) = nullptr;
	cl_mem (__stdcall* clCreateImage) (cl_context, cl_mem_flags, const cl_image_format*, const cl_image_desc*, void*, cl_int*) = nullptr;
	cl_int (__stdcall* clReleaseMemObject) (cl_mem) = nullptr;
	cl_int (__stdcall* clGetMemObjectInfo) (cl_mem, cl_mem_info, size_t, void*, size_t*) = nullptr;
	cl_program (__stdcall* clCreateProgramWithSource) (cl_context, cl_uint, const char**, const size_t*, cl_int*) = nullptr;
	cl_int (__stdcall* clReleaseProgram) (cl_program) = nullptr;
	cl_int (__stdcall* clBuildProgram) (cl_program, cl_uint, const cl_device_id*, const char*, void (CL_CALLBACK *)(cl_program, void* ), void*) = nullptr;
	cl_int (__stdcall* clGetProgramInfo) (cl_program, cl_program_info, size_t, void*, size_t*) = nullptr;
	cl_int (__stdcall* clGetProgramBuildInfo) (cl_program, cl_device_id, cl_program_build_info, size_t, void*, size_t*) = nullptr;
	cl_kernel (__stdcall* clCreateKernel) (cl_program, const char*, cl_int*) = nullptr;
	cl_int (__stdcall* clReleaseKernel) (cl_kernel) = nullptr;
	cl_int (__stdcall* clSetKernelArg) (cl_kernel, cl_uint, size_t, const void*) = nullptr;
	cl_int (__stdcall* clEnqueueReadBuffer) (cl_command_queue, cl_mem, cl_bool, size_t, size_t, void*, cl_uint, const cl_event*, cl_event*) = nullptr;
	cl_int (__stdcall* clEnqueueWriteBuffer) (cl_command_queue, cl_mem, cl_bool, size_t, size_t, const void*, cl_uint, const cl_event*, cl_event*) = nullptr;
	cl_int (__stdcall* clEnqueueNDRangeKernel) (cl_command_queue, cl_kernel, cl_uint, const size_t*, const size_t*, const size_t*, cl_uint, const cl_event*, cl_event*) = nullptr;

#define  GET_CL_FUNCTION( func ) \
	func = reinterpret_cast<decltype(func)> (GetProcAddress(clDll_, #func)); \
	if (nullptr == func) \
		return ERROR_UNINITIALIZED;


	// GPU sharing funcs.
		//  Kronos
		clCreateFromD3D11Texture2DKHR_fn pCreateFromD3D11Texture2D_ = nullptr;
		clCreateFromD3D11BufferKHR_fn pCreateFromD3D11Buffer_ = nullptr;
		clEnqueueAcquireD3D11ObjectsKHR_fn pEnqueueAcquireD3D11Objects_ = nullptr;
		clEnqueueReleaseD3D11ObjectsKHR_fn pEnqueueReleaseD3D11Objects_ = nullptr;

		//  NVidia
		bool sharingNV_ = false;
		//clCreateFromD3D11Texture2DNV_fn pCreateFromD3D11Texture2DNV_ = nullptr;
		//clCreateFromD3D11BufferNV_fn pCreateFromD3D11BufferNV_ = nullptr;
		//clEnqueueAcquireD3D11ObjectsNV_fn pEnqueueAcquireD3D11ObjectsNV_ = nullptr;
		//clEnqueueReleaseD3D11ObjectsNV_fn pEnqueueReleaseD3D11ObjectsNV_ = nullptr;
	///////////////////////
};




// Buffer for access by OpenCL device.
class BufferCLimpl : public IComputeBuffer
{
public:
	virtual void* getNativeBuffer() override
	{
		return pClBuffer_;
	}

	virtual void* getGraphicsBuffer() override
	{
		return pUaDxBuffer_;
	}

	virtual size_t getSizeBytes() override
	{
		if (pClBuffer_ && pOpenCLCompute)
		{
			size_t mem_size = 0;
			cl_int res = pOpenCLCompute->clGetMemObjectInfo(pClBuffer_, CL_MEM_SIZE, sizeof(size_t), &mem_size, nullptr);
			if (CL_SUCCESS != res)
				return 0;

			return mem_size;
		}

		return 0;
	}

	virtual ~BufferCLimpl()
	{
		if (pClBuffer_ && pOpenCLCompute)
			pOpenCLCompute->clReleaseMemObject(pClBuffer_);

		if (pUaDxBuffer_)
			pUaDxBuffer_->release();
	}

private:
	cl_mem pClBuffer_ = nullptr;	// Native CL buffer.

	UABufferDX11* pUaDxBuffer_ = nullptr;  // DX11 UA (unordered access) buffer if CL shared with DX11.

public:
	static OpenCLCompute* pOpenCLCompute;
	friend class OpenCLCompute;
};

typedef std::shared_ptr<BufferCLimpl> ComputeBufferCL;




class ComputeTaskCLimpl : public IComputeTask
{
public:
	virtual void release() final
	{
		if (kernel_ && pOpenCLCompute)
			pOpenCLCompute->clReleaseKernel(kernel_);
		kernel_ = nullptr;

		if (program_ && pOpenCLCompute)
			pOpenCLCompute->clReleaseProgram(program_);
		program_ = nullptr;
	}

private:
	static const bool bDebugEnabled_ = false;

	cl_program program_ = nullptr;
	cl_kernel kernel_ = nullptr;

public:
	ComputeTaskCLimpl() {}

	virtual ~ComputeTaskCLimpl()
	{
		release();
	}

	static OpenCLCompute* pOpenCLCompute;
	friend class OpenCLCompute;
};

typedef std::shared_ptr<ComputeTaskCLimpl> ComputeTaskCL;


