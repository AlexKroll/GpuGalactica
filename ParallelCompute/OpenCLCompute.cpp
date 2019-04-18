#include "OpenCLCompute.h"

// We use dynamic linking of the OpenCL API.
// Because we want to avoid dependence on vendor`s API (such as AMD, nVidia ...)

//#pragma comment(lib, "OpenCL.lib" )
//C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v10.0\include


OpenCLCompute* BufferCLimpl::pOpenCLCompute = nullptr;
OpenCLCompute* ComputeTaskCLimpl::pOpenCLCompute = nullptr;



OpenCLCompute::OpenCLCompute()
{
	ZeroMemory(imageFormats_, sizeof(imageFormats_));
	imageFormats_[ITexture::UnknownFormat] = { 0, 0 };
	imageFormats_[ITexture::ColorRGBA8] = { CL_RGBA, CL_UNORM_INT8 };
	imageFormats_[ITexture::ColorRGB8] = { CL_RGB, CL_UNORM_INT8 };
	imageFormats_[ITexture::ColorR5G6B5] = { CL_RGB, CL_UNORM_SHORT_565 };
	imageFormats_[ITexture::ColorR8] = { CL_R, CL_UNORM_INT8 };
	imageFormats_[ITexture::ColorR16F] = { CL_R, CL_HALF_FLOAT };
	imageFormats_[ITexture::ColorR32F] = { CL_R, CL_FLOAT };
	imageFormats_[ITexture::ColorA8] = { CL_A, CL_UNORM_INT8 };
	imageFormats_[ITexture::ColorBGRA8] = { CL_BGRA, CL_UNORM_INT8 };
	imageFormats_[ITexture::Depth16] = { CL_DEPTH, CL_HALF_FLOAT };
	imageFormats_[ITexture::Depth32] = { CL_DEPTH, CL_FLOAT };
	imageFormats_[ITexture::Float16_4] = { CL_RGBA, CL_UNORM_INT16 };
}


OpenCLCompute::~OpenCLCompute()
{
	release();
}


int OpenCLCompute::init(Render pRender)
{
	pRender_ = pRender;

	// Load the OpenCL dynamically, so we will avoid the dependence on vendor`s API.
	if (nullptr == clDll_)
	{
		clDll_ = LoadLibraryA( "OpenCL.dll" );
		if (nullptr == clDll_)
			return ERROR_UNINITIALIZED;
	
		GET_CL_FUNCTION(clFlush);
		GET_CL_FUNCTION(clFinish);
		GET_CL_FUNCTION(clReleaseCommandQueue);
		GET_CL_FUNCTION(clReleaseContext);
		GET_CL_FUNCTION(clReleaseDevice);
		GET_CL_FUNCTION(clGetPlatformIDs);
		GET_CL_FUNCTION(clGetPlatformInfo);
		GET_CL_FUNCTION(clGetExtensionFunctionAddressForPlatform);
		GET_CL_FUNCTION(clGetDeviceIDs);
		GET_CL_FUNCTION(clGetDeviceInfo);
		GET_CL_FUNCTION(clCreateContext);
		GET_CL_FUNCTION(clCreateCommandQueue);
		GET_CL_FUNCTION(clCreateBuffer);
		GET_CL_FUNCTION(clCreateImage);
		GET_CL_FUNCTION(clReleaseMemObject);
		GET_CL_FUNCTION(clGetMemObjectInfo);
		GET_CL_FUNCTION(clCreateProgramWithSource);
		GET_CL_FUNCTION(clReleaseProgram);
		GET_CL_FUNCTION(clBuildProgram);
		GET_CL_FUNCTION(clGetProgramInfo);
		GET_CL_FUNCTION(clGetProgramBuildInfo);
		GET_CL_FUNCTION(clCreateKernel);
		GET_CL_FUNCTION(clReleaseKernel);
		GET_CL_FUNCTION(clSetKernelArg);
		GET_CL_FUNCTION(clEnqueueReadBuffer);
		GET_CL_FUNCTION(clEnqueueWriteBuffer);
		GET_CL_FUNCTION(clEnqueueNDRangeKernel);
	}

	cKernelsFolder_ = "Shaders/CL/";

	char buff[FILENAME_MAX];
	buff[0] = 0;
	::GetCurrentDirectory(FILENAME_MAX, buff);
	programFolder_ = buff;
	programFolder_ += "/";

	// Iterate the CL platforms.
	const cl_uint max_platforms = 8;
	cl_platform_id platforms[max_platforms] = { nullptr };
	cl_uint num_platforms = 0;

	cl_int res = clGetPlatformIDs(max_platforms, platforms, &num_platforms);
	if (CL_SUCCESS != res)
	{	ShowError(nullptr, __FUNCTION__, " error: clGetPlatformIDs failed.");
		return res;
	}

	char chBuffer[2048];

	// Try to find the compute device in order: GPU, Accelerator, CPU.
	cl_device_type device_type_priorities[] = { CL_DEVICE_TYPE_GPU, CL_DEVICE_TYPE_ACCELERATOR, CL_DEVICE_TYPE_CPU };
	for (cl_device_type tp : device_type_priorities)
	{
		for (cl_uint iplatform = 0;  iplatform < num_platforms;  ++iplatform)
		{
			res = clGetPlatformInfo(platforms[iplatform], CL_PLATFORM_VERSION, sizeof(chBuffer), chBuffer, nullptr);
			if (CL_SUCCESS != res)
			{	ShowError(nullptr, __FUNCTION__, " error: clGetPlatformInfo failed.");
				continue;
			}

			cl_platform_id platform = platforms[iplatform];

			res = clGetPlatformInfo (platform, CL_PLATFORM_NAME, sizeof(chBuffer), chBuffer, nullptr);
			/*if (CL_SUCCESS == res)
			{
				if (strstr(chBuffer, "NVIDIA") != nullptr)
				{	gpu_platform = platforms[i];
					break;
				}

				if (strstr(chBuffer, "AMD") != nullptr)
				{	gpu_platform = platforms[i];
					break;
				}
			}*/

			const cl_uint max_devices = 8;
			cl_device_id devices[max_devices] = { nullptr };
			cl_uint num_devices = 0;

			res = clGetPlatformInfo(platforms[iplatform], CL_PLATFORM_EXTENSIONS, sizeof(chBuffer), chBuffer, nullptr); //clGetPlatformInfo
			if (CL_SUCCESS != res)
			{	ShowError(nullptr, __FUNCTION__, " error: clGetPlatformInfo failed.");
				continue;
			}

//char* cfind = strstr(chBuffer, "cl_nv_d3d11_sharing"); //cl_khr_dx11_sharing
//char* pc = chBuffer + 150;

			// Try get extensions for DX11 GPU device. If found, we are going to get CL compute from DirectX11 GPU device
			// and bind its textures to CL images.
			if (IRender::DX11 == pRender_->getDeviceType())
			{
				if (strstr(chBuffer, "cl_nv_d3d11_sharing"))
				{
					sharingNV_ = true;

					pCreateFromD3D11Texture2D_ =
						(clCreateFromD3D11Texture2DKHR_fn)clGetExtensionFunctionAddressForPlatform(platform, "clCreateFromD3D11Texture2DNV");

					pCreateFromD3D11Buffer_ =
						(clCreateFromD3D11BufferKHR_fn)clGetExtensionFunctionAddressForPlatform(platform, "clCreateFromD3D11BufferNV");

					pEnqueueAcquireD3D11Objects_ =
						(clEnqueueAcquireD3D11ObjectsKHR_fn)clGetExtensionFunctionAddressForPlatform(platform, "clEnqueueAcquireD3D11ObjectsNV");

					pEnqueueReleaseD3D11Objects_ =
						(clEnqueueReleaseD3D11ObjectsKHR_fn)clGetExtensionFunctionAddressForPlatform(platform, "clEnqueueReleaseD3D11ObjectsNV");
				}
				else
				{
					pCreateFromD3D11Texture2D_ =
						(clCreateFromD3D11Texture2DKHR_fn)clGetExtensionFunctionAddressForPlatform(platform, "clCreateFromD3D11Texture2DKHR");

					pCreateFromD3D11Buffer_ =
						(clCreateFromD3D11BufferKHR_fn)clGetExtensionFunctionAddressForPlatform(platform, "clCreateFromD3D11BufferKHR");

					pEnqueueAcquireD3D11Objects_ =
						(clEnqueueAcquireD3D11ObjectsKHR_fn)clGetExtensionFunctionAddressForPlatform(platform, "clEnqueueAcquireD3D11ObjectsKHR");

					pEnqueueReleaseD3D11Objects_ =
						(clEnqueueReleaseD3D11ObjectsKHR_fn)clGetExtensionFunctionAddressForPlatform(platform, "clEnqueueReleaseD3D11ObjectsKHR");
				}

				/*clGetDeviceIDsFromD3D11NV_fn pGetDeviceIDsFromD3D11NV =
					(clGetDeviceIDsFromD3D11NV_fn)clGetExtensionFunctionAddressForPlatform(platform,"clGetDeviceIDsFromD3D11NV");

				//clGetDeviceIDsFromD3D11KHR_fn pGetDeviceIDsFromD3D11KHR =
				//	(clGetDeviceIDsFromD3D11KHR_fn)clGetExtensionFunctionAddressForPlatform(platform,"clGetDeviceIDsFromD3D11KHR");
				if (pGetDeviceIDsFromD3D11NV) //pGetDeviceIDsFromD3D11KHR
				{
					IRender* render = pRender_.get();
					RenderDX11* pRenderDX11 = dynamic_cast<RenderDX11*>(render);

					res = pGetDeviceIDsFromD3D11NV(platform, CL_D3D11_DEVICE_KHR, pRenderDX11->getDevice(), CL_PREFERRED_DEVICES_FOR_D3D11_KHR,
							max_devices, devices, &num_devices);

					if (CL_SUCCESS == res)
					{
						platform_ = platform;
						device_ = devices[0];
						//break;
					}
				}*/
			}

			res = clGetDeviceIDs(platform, tp, max_devices, devices, &num_devices);
			if (CL_SUCCESS != res)
			{	ShowError(nullptr, __FUNCTION__, " error: clGetDeviceIDs failed.");
				continue;
			}

			// Find the device with maximum cores and clock. Test some kernel and measure the execution time.
			platform_ = platform;
			device_ = devices[0];
			break;

			/*for (cl_uint idvc = 0;  idvc < num_devices;  ++idvc)
			{
				res = clGetDeviceInfo(devices[idvc], CL_DEVICE_TYPE, sizeof(device_type), &device_type, nullptr);
				if (CL_SUCCESS == res)
				{
					if (device_type == tp)
					{
						best_platform = platforms[ipltfrm];
						best_device = devices[idvc];
						break;
					}
				}
			}*/
		}

		if (platform_ && device_)
			break;
	}

	if (nullptr == platform_ || nullptr == device_)
	{	ShowError(nullptr, __FUNCTION__, " error: Hardware compute not found.");
		return ERROR_NOT_FOUND;
	}

	cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform_,
										   0,0, //CL_CONTEXT_INTEROP_USER_SYNC, CL_FALSE,
										   0,0, 0,0, 0,0, 0,0 };
	if (IRender::DX11 == pRender->getDeviceType())
	{
		RenderDX11* pRenderDX11 = dynamic_cast<RenderDX11*>(pRender_.get());
		if (pRenderDX11)
		{
			if( sharingNV_ ) //pCreateFromD3D11Texture2DNV_
				properties[2] = CL_CONTEXT_D3D11_DEVICE_NV;
			else
				properties[2] = CL_CONTEXT_D3D11_DEVICE_KHR;
			properties[3] = (cl_context_properties)pRenderDX11->getDevice();

//properties[4] = CL_CONTEXT_INTEROP_USER_SYNC;
//properties[5] = CL_FALSE;
		}
	}

	// Get info about the device.
	{
		cl_ulong maxConstBuffSize_ = 0;
		res = clGetDeviceInfo(device_, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &maxConstBuffSize_, nullptr);

		cl_ulong localMemSize_ = 0;
		res = clGetDeviceInfo(device_, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &localMemSize_, nullptr);

		cl_uint maxConstArgs_ = 0;
		res = clGetDeviceInfo(device_, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(cl_uint), &maxConstArgs_, nullptr);

		size_t maxWorkGroupSize_ = 0;
		res = clGetDeviceInfo(device_, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkGroupSize_, nullptr);

		size_t kernelWorkGroupSize_ = 0;
		res = clGetDeviceInfo(device_, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &kernelWorkGroupSize_, nullptr);

		cl_uint maxComputeUnits_ = 0;
		res = clGetDeviceInfo(device_, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &maxComputeUnits_, nullptr);

		size_t PrefWorkGroupSize_ = 0;
		res = clGetDeviceInfo(device_, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(size_t), &PrefWorkGroupSize_, nullptr);
		//CL_DEVICE_MAX_WORK_ITEM_SIZES
	}

	context_ = clCreateContext(properties, 1, &device_, nullptr, nullptr, &res);
	if (CL_SUCCESS != res)
	{	ShowError(nullptr, __FUNCTION__, " error: clCreateContext failed.");
		return res;
	}

	commandQueue_ = clCreateCommandQueue(context_, device_, 0, &res);
	if (CL_SUCCESS != res)
	{	ShowError(nullptr, __FUNCTION__, " error: clCreateCommandQueue failed.");
		return res;
	}

	return S_OK;
}


void OpenCLCompute::release()
{
	cl_int res = clFlush(commandQueue_);
    res = clFinish(commandQueue_);

	res = clReleaseCommandQueue(commandQueue_);
    res = clReleaseContext(context_);

	res = clReleaseDevice(device_);
}


cl_mem_flags OpenCLCompute::mapFlags(uint32_t flags)
{
	cl_mem_flags cl_flags = 0;

	if (kBufferReadWrite & flags)
		cl_flags = CL_MEM_READ_WRITE;
	else if(kBufferWrite & flags)
		cl_flags = CL_MEM_WRITE_ONLY;
		else if(kBufferRead & flags)
			cl_flags = CL_MEM_READ_ONLY;

	if( kCpuNoAccess & flags)
		cl_flags |= CL_MEM_HOST_NO_ACCESS;

	return cl_flags;
}


ComputeBuffer OpenCLCompute::createBuffer(int amountOfElems, int elemSize, uint32_t flags)
{
	cl_mem_flags cl_flags = mapFlags(flags);

	cl_int res = CL_INVALID_VALUE;
	cl_mem cl_buffer = nullptr;

	if (flags & kAutoLength)
		amountOfElems++; // first element is used as counter (see CL shaders).

	ComputeBufferCL comp_buffer = std::shared_ptr<BufferCLimpl>(new BufferCLimpl);
	if (nullptr == comp_buffer)
		return nullptr;

	comp_buffer->pOpenCLCompute = this;

	//  If the buffer is shared, create a native buffer of the selected graphics API.
	if (flags & kShared)
	{
		if (IRender::DX11 == pRender_->getDeviceType())
		{
			RenderDX11* pRenderDX11 = dynamic_cast<RenderDX11*>(pRender_.get());
			if (pRenderDX11)
			{
				UABufferDX11* ua_buffer = pRenderDX11->createUABuffer(amountOfElems, elemSize, flags);

				if (ua_buffer)
				{
					ID3D11Buffer* dx_buffer = ua_buffer->getDxBuffer();

					if (dx_buffer)
					{
						if (pCreateFromD3D11Buffer_)
						{
							cl_buffer = pCreateFromD3D11Buffer_(context_, cl_flags, dx_buffer, &res);
						}
						/*else if (pCreateFromD3D11Buffer_)
						{
							cl_buffer = pCreateFromD3D11Buffer_(context_, cl_flags, dx_buffer, &res);
						}*/

						comp_buffer->pUaDxBuffer_ = ua_buffer;

						comp_buffer->shareBuffer();
					}
				}
			}
		}
		else if (IRender::GL == pRender_->getDeviceType())
		{
			//cl_buffer = clCreateFromGLBuffer(...);
		}
	}
	else
	{
		cl_buffer = clCreateBuffer(context_, cl_flags, amountOfElems * elemSize, nullptr, &res);
		if (CL_SUCCESS != res)
			return nullptr;
	}

	if (cl_buffer)
		comp_buffer->pClBuffer_ = cl_buffer;

	return comp_buffer;
}


ComputeBuffer OpenCLCompute::createConstantBuffer(size_t bytes)
{
	cl_int res = CL_INVALID_VALUE;

	cl_mem cl_buffer = clCreateBuffer(context_, CL_MEM_READ_ONLY, bytes, nullptr, &res);
	if (CL_SUCCESS != res)
		return nullptr;

	ComputeBufferCL pBuffer = std::shared_ptr<BufferCLimpl>(new BufferCLimpl);
	pBuffer->pClBuffer_ = cl_buffer;

	pBuffer->pOpenCLCompute = this;

	return pBuffer;
}


void OpenCLCompute::writeBuffer(ComputeBuffer buffer, const void* data, size_t bytes)
{
	if (nullptr == buffer)
		return;

	cl_mem pBufferCL = reinterpret_cast<cl_mem>(buffer->getNativeBuffer());
	if (pBufferCL)
	{
		if (buffer->isShared() && pEnqueueAcquireD3D11Objects_)
		{
			cl_int res = pEnqueueAcquireD3D11Objects_(commandQueue_, 1, &pBufferCL, 0, nullptr, nullptr);
			if (CL_SUCCESS != res)
			{	res++;
			}
		}

		cl_int res = clEnqueueWriteBuffer(commandQueue_, pBufferCL, CL_FALSE, 0, bytes, data, 0, nullptr, nullptr);
		if (CL_SUCCESS != res)
		{	res++;
		}

		if (buffer->isShared() && pEnqueueReleaseD3D11Objects_)
		{
			res = pEnqueueReleaseD3D11Objects_(commandQueue_, 1, &pBufferCL, 0, nullptr, nullptr);
			if (CL_SUCCESS != res)
			{	res++;
			}
		}
	}
}


void OpenCLCompute::setConstantBuffer(ComputeBuffer buffer, uint32_t slot)
{
	setComputeBuffer(buffer, slot);
}


int OpenCLCompute::debugDumpBuffer(void* dest, ComputeBuffer buffer)
{
	if (nullptr == buffer || nullptr == dest)
		return -1;

	size_t buff_bytes = buffer->getSizeBytes();

	cl_mem pBufferCL = reinterpret_cast<cl_mem>(buffer->getNativeBuffer());
	if (pBufferCL)
	{
		//ComputeBuffer buff_dump = createBuffer(buff_bytes, sizeof(BYTE), false, kBufferReadWrite);
		//if (buff_dump)
		{
			//cl_mem clbuff_dump = reinterpret_cast<cl_mem>(buff_dump->getNativeBuffer());

			//cl_int res = clEnqueueCopyBuffer(commandQueue_, pBufferCL, clbuff_dump, 0, 0, buff_bytes, 0, nullptr, nullptr);
			//if (CL_SUCCESS != res)
			//	return -2;

			cl_int res = clFlush(commandQueue_);
			if (CL_SUCCESS != res)
				return res;

			res = clFinish(commandQueue_);
			if (CL_SUCCESS != res)
				return res;

			if (buffer->isShared() && pEnqueueAcquireD3D11Objects_)
			{
				res = pEnqueueAcquireD3D11Objects_(commandQueue_, 1, &pBufferCL, 0, nullptr, nullptr);
				if (CL_SUCCESS != res)
					return res;
			}

			res = clEnqueueReadBuffer(commandQueue_, pBufferCL/*clbuff_dump*/, CL_TRUE, 0, buff_bytes, dest, 0, nullptr, nullptr);
			if (CL_SUCCESS != res)
				return res;

			if (buffer->isShared() && pEnqueueReleaseD3D11Objects_)
			{
				res = pEnqueueReleaseD3D11Objects_(commandQueue_, 1, &pBufferCL, 0, nullptr, nullptr);
				if (CL_SUCCESS != res)
					return res;
			}
		}
	}

	return -5;
}


Texture OpenCLCompute::createImage(UINT width, UINT height, ITexture::PixelFormat format, ITexture::Type, uint32_t flags, BYTE* pPixels)
{
	cl_mem_flags cl_flags = mapFlags(flags);

	cl_int res = CL_INVALID_VALUE;

	cl_mem cl_image = nullptr;

	cl_image_desc desc;
	desc.image_type = CL_MEM_OBJECT_IMAGE2D; //(1 == height) ? CL_MEM_OBJECT_IMAGE1D : CL_MEM_OBJECT_IMAGE2D;
								// Old nvidia GPUs doesnt support 1D images.
	desc.image_width = width;
	desc.image_height = height;
	desc.image_depth = 0;//1;
	desc.image_array_size = 0;//1
	desc.image_row_pitch = width * ITexture::getBytesPerPixel(format);
	desc.image_slice_pitch = 0;
	desc.num_mip_levels = 0;
	desc.num_samples = 0;
	desc.buffer = nullptr;

	if (pPixels)
		cl_flags |= CL_MEM_COPY_HOST_PTR;

	cl_image = clCreateImage(context_, cl_flags, &imageFormats_[format], &desc, pPixels, &res);

	EmptyTexture texture = std::shared_ptr<EmptyTexture_impl>(new EmptyTexture_impl);
	texture->setComputeImage(cl_image);

	return texture;
}


int OpenCLCompute::createImageIntoTexture(Texture texture, uint32_t flags)
{
	if (nullptr == pRender_)
		return -1;

	cl_mem_flags cl_flags = mapFlags(flags);

	cl_int res = CL_INVALID_VALUE;

	cl_mem cl_image = nullptr;

	// Map DX11 texture to OpenCL image.
	if (IRender::DX11 == pRender_->getDeviceType())
	{
		TextureDX11 pTexture = std::dynamic_pointer_cast<TextureDX11impl>(texture);
		if (pTexture == nullptr)
			return -2;

		ID3D11Texture2D* pDxTexture = pTexture->getDxTexture();
		if (pDxTexture == nullptr)
			return -3;

		if (pCreateFromD3D11Texture2D_)
		{
			cl_image = pCreateFromD3D11Texture2D_(context_, cl_flags, pDxTexture, 0, &res);
		}
		/*else if (pCreateFromD3D11Texture2DNV_)
		{
			cl_image = pCreateFromD3D11Texture2DNV_(context_, cl_flags, pDxTexture, 0, &res);
		}*/
	}

	if (cl_image)
	{	texture->setComputeImage(cl_image);
		texture->shareImage();
		return 0;
	}

	return -4;
}


void OpenCLCompute::releaseImage(Texture& texture)
{
	cl_mem pImageCL = reinterpret_cast<cl_mem>(texture->getComputeImage());

	if (texture->isShared() && pRender_)
		pRender_->releaseTexture(texture);

	if (pImageCL)
	{
		cl_int res = clReleaseMemObject(pImageCL);
		if (CL_SUCCESS != res)
			res++;
	}
}


ComputeTask OpenCLCompute::createComputeTask(cstring csFile, CShaderMacroStrings defines)
{
	if (0 == csFile.length())
		return nullptr;

	std::string path = cKernelsFolder_ + csFile;

	// Get file size.
	std::unique_ptr<char[]> text;
	size_t file_size = MyGetFileSize(path.c_str());

	// Read file.
	{	FileAutoClose hf;
		errno_t file_res = fopen_s(hf.getAddrOf(), path.c_str(), "rb");
		if (0 == file_res)
		{
			text = std::make_unique<char[]>(file_size+1);
			fread(text.get(), file_size, 1, hf);
			text[file_size] = 0;
		}
	}

	std::string options;

	// Assemble macroses
	if (defines.size() != 0)
	{
		options.reserve(1024);

		for (const std::pair<cstring, cstring>& pair : defines)
		{
			options += "-D ";
			options += pair.first.c_str();
			if (pair.second.length() != 0)
			{	options += "=";
				options += pair.second.c_str();
			}
			options += " ";
		}
	}

//options += "-cl-mad-enable ";
//options += "-cl-fast-relaxed-math ";

	options += "-I ";
	//options += programFolder_;
	options += "Shaders/CL/ParticleEmitters/";

	// Compile the kernel.
	cl_int res;

	char* pc = text.get();
	cl_program program = clCreateProgramWithSource(context_, 1, (const char**)&pc, &file_size, &res);
	if (CL_SUCCESS != res)
		return nullptr;

	res = clBuildProgram(program, 1, &device_, options.c_str(), nullptr, nullptr);
	if (CL_SUCCESS != res)
	{
		size_t log_size;
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);

		std::unique_ptr<char[]> log = std::make_unique<char[]>(log_size+1);
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG, log_size, log.get(), nullptr);
#ifdef _DEBUG
		OutputDebugString( log.get() );
#endif

		clReleaseProgram(program);
		return nullptr;
	}

	cl_kernel kernel = clCreateKernel(program, "Main", &res);
	if (CL_SUCCESS != res)
	{	clReleaseProgram(program);
		return nullptr;
	}

/*size_t sz = 0;
res = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t), &sz, nullptr);
unsigned char* binary = new unsigned char [sz+16];
res = clGetProgramInfo(program, CL_PROGRAM_BINARIES, sz, &binary, 0);

FILE* hf = nullptr;
errno_t file_res = fopen_s(&hf, "c://alexbase/cl_kernel_asm.txt", "wb");
fwrite(binary, 1, sz, hf);
fclose(hf);
int kk;*/

	ComputeTaskCL task = std::shared_ptr<ComputeTaskCLimpl>(new ComputeTaskCLimpl);
	if (task)
	{	task->program_ = program;
		task->kernel_ = kernel;

		task->pOpenCLCompute = this;
	}

	return task;
}


void OpenCLCompute::setComputeTask(ComputeTask task)
{
	ComputeTaskCL taskCL = std::dynamic_pointer_cast<ComputeTaskCLimpl>(task);
	if (taskCL)
	{	currProgram_ = taskCL->program_;
		currKernel_ = taskCL->kernel_;
	}
}


void OpenCLCompute::setComputeBuffer(ComputeBuffer buffer, uint32_t slot)
{
	if (nullptr == buffer)
		return;

	cl_mem pBufferCL = reinterpret_cast<cl_mem>(buffer->getNativeBuffer());
	if (pBufferCL)
	{
		if (buffer->isShared() && pEnqueueAcquireD3D11Objects_)
		{
			cl_int res = pEnqueueAcquireD3D11Objects_(commandQueue_, 1, &pBufferCL, 0, nullptr, nullptr );
			if (CL_SUCCESS != res)
			{	res++;
			}

			acquiredResourses_.push_back(pBufferCL);
		}

		cl_int res = clSetKernelArg(currKernel_, slot, sizeof(cl_mem), (void*)&pBufferCL);
		if (CL_SUCCESS != res)
		{	res++;
		}
	}
}


void OpenCLCompute::setImage(Texture texture, uint32_t slot)
{
	if (nullptr == texture)
		return;

	cl_mem pImageCL = reinterpret_cast<cl_mem>(texture->getComputeImage());
	if (pImageCL)
	{
		if (texture->isShared() && pEnqueueAcquireD3D11Objects_)
		{
			cl_int res = pEnqueueAcquireD3D11Objects_(commandQueue_, 1, &pImageCL, 0, nullptr, nullptr );
			if (CL_SUCCESS != res)
			{	res++;
			}

			acquiredResourses_.push_back(pImageCL);
		}

		cl_int res = clSetKernelArg(currKernel_, slot, sizeof(cl_mem), (void*)&pImageCL);
		if (CL_SUCCESS != res)
		{	res++;
		}
	}
}


void OpenCLCompute::compute(uint32_t dim_x, uint32_t dim_y, uint32_t num_groups)
{
	/*size_t global_work_size[2] = { dim_x, dim_y };
	cl_uint work_dim = (1 == dim_y) ? 1 : 2;
	cl_int res = clEnqueueNDRangeKernel(commandQueue_, currKernel_, work_dim, nullptr, global_work_size, nullptr, 0, nullptr, nullptr);
	if (CL_SUCCESS != res)
		res++;*/

size_t global_work_size[2] = { dim_x, dim_y };
size_t local_work_size[2] = { dim_x/num_groups, dim_y };
if (0 == local_work_size[0])
	local_work_size[0] = 1;
cl_uint work_dim = (1 == dim_y) ? 1 : 2;
cl_int res = clEnqueueNDRangeKernel(commandQueue_, currKernel_, work_dim, nullptr, global_work_size, local_work_size, 0, nullptr, nullptr);
if (CL_SUCCESS != res)
	res++;


	if (acquiredResourses_.size() != 0 && pEnqueueReleaseD3D11Objects_)
	{
		res = pEnqueueReleaseD3D11Objects_(commandQueue_, (cl_uint)acquiredResourses_.size(), acquiredResourses_.data(), 0, nullptr, nullptr);
		if (CL_SUCCESS != res)
		{	res++;
		}
		acquiredResourses_.clear();
	}
}


int OpenCLCompute::finish()
{
	//cl_int res = clFlush(commandQueue_);
	//if (CL_SUCCESS != res)
	//	return -1;

	cl_int res = clFinish(commandQueue_);
	if (CL_SUCCESS != res)
		return -2;

	return 0;
}