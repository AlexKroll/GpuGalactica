#include "DirectCompute11.h"



DirectCompute11::DirectCompute11()
{
}


DirectCompute11::~DirectCompute11()
{
}


int DirectCompute11::init(Render pRender)
{
	IRender* render = pRender.get();
	pRender_ = dynamic_cast<RenderDX11*>(render);

	return S_OK;
}


void DirectCompute11::release()
{
}


//Render DirectCompute11::getRender()
//{	return pRender_;
//}


UABuffer DirectCompute11::createUABuffer(int amountOfElems, int elemSize, bool bGPUlength)
{
	if (nullptr == pRender_)
		return nullptr;

	return pRender_->createUABuffer(amountOfElems, elemSize, bGPUlength);
}


Shader DirectCompute11::createComputeProgram(cstring csFile, CShaderMacroStrings defines)
{
	if (nullptr == pRender_)
		return nullptr;

	return pRender_->createComputeProgram(csFile, defines);
}


void DirectCompute11::setComputeProgram(Shader pProgram)
{
	if (nullptr == pRender_)
		return;

	pRender_->setComputeProgram(pProgram);
}


void DirectCompute11::writeConstantBuffer(ConstantBuffer buffer, const void* data, size_t bytes)
{
	if (pRender_)
	{
		void* pDestination = pRender_->openConstantBuffer(buffer);
		if (pDestination)
		{	memcpy(pDestination, data, bytes);
			pRender_->closeConstantBuffer(buffer);
		}
	}

}


void DirectCompute11::setConstantBuffer(ConstantBuffer buffer, uint32_t slot)
{
	if (nullptr == pRender_)
		return;

	pRender_->setConstantBufferCS(buffer, slot);
}


void DirectCompute11::setComputeBuffer(UABuffer buffer, uint32_t slot)
{
	if (nullptr == pRender_)
		return;

	IUABuffer* ibuffer = buffer.get();

	pRender_->setComputeBuffer(ibuffer, slot);
}


void DirectCompute11::bindUABufferToTextureVS(UABuffer buffer, UINT slot)
{
	if (nullptr == pRender_)
		return;

	pRender_->bindUABufferToTextureVS(buffer, slot);
}


void DirectCompute11::setTexture(Texture texture, uint32_t slot)
{
	if (nullptr == pRender_)
		return;

	pRender_->setComputeTexture(texture, slot);
}


void DirectCompute11::setSampler(uint32_t samplerType, uint32_t slot)
{
	if (nullptr == pRender_)
		return;

	pRender_->setComputeSampler(samplerType, slot);
}


void DirectCompute11::compute(UINT x, UINT y, UINT z)
{
	if (nullptr == pRender_)
		return;

	pRender_->compute(x, y, z);
}