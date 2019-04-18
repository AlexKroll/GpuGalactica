#include "ParallelCompute.h"
#include "DirectX11Compute.h"
#include "OpenCLCompute.h"
#include "CudaCompute.h"



IParallelCompute* IParallelCompute::getCompute(Type type)
{
	IParallelCompute* pCompute = nullptr;

	switch (type)
	{
		case DX11:
			pCompute = new DirectX11Compute;
			break;

		case OPENCL:
			pCompute = new OpenCLCompute;
			break;

		case CUDA:
			pCompute = new CudaCompute;
			break;
	}

	pCompute->type_ = type;

	return pCompute;
}


IParallelCompute::Type IParallelCompute::getType() const
{
	return type_;
}