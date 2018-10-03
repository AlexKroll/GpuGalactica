#include "ParallelCompute.h"
#include "DirectCompute11.h"
#include "OpenCLCompute.h"
#include "CudaCompute.h"



IParallelCompute* IParallelCompute::getCompute(Type type)
{
	IParallelCompute* pCompute = nullptr;

	switch (type)
	{
		case DX11:
			pCompute = new DirectCompute11;
			break;

		case OPENCL:
			pCompute = new OpenCLCompute;
			break;

		case CUDA:
			pCompute = new CudaCompute;
			break;
	}

	return pCompute;
}