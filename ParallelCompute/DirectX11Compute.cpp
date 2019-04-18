#include <cassert>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include "DirectX11Compute.h"



DirectX11Compute::DirectX11Compute()
{
}


DirectX11Compute::~DirectX11Compute()
{
}


int DirectX11Compute::init(Render pRender)
{
	// Need shared_ptr
__pragma( message( "Warning: Need shared_ptr for RenderDX11" ) )

	IRender* render = pRender.get();
	pRender_ = dynamic_cast<RenderDX11*>(render);

	return S_OK;
}


void DirectX11Compute::release()
{
}


//Render DirectX11Compute::getRender()
//{	return pRender_;
//}


ComputeBuffer DirectX11Compute::createBuffer(int amountOfElems, int elemSize, uint32_t flags)
{
	if (nullptr == pRender_)
		return nullptr;

	return pRender_->createComputeBuffer(amountOfElems, elemSize, flags);
}


int DirectX11Compute::debugDumpBuffer(void* dest, ComputeBuffer buffer)
{
	if (nullptr == buffer || nullptr == dest)
		return -1;

	size_t buff_bytes = buffer->getSizeBytes();

	UABufferDX11* uabuff = (UABufferDX11*)buffer->getNativeBuffer();
	if (nullptr == uabuff)
		return -2;

	ID3D11Buffer* dxbuff = uabuff->getDxBuffer();
	if (nullptr == dxbuff)
		return -3;

	//D3D11_BUFFER_DESC desc;
	//dxbuff->GetDesc(&desc);
	//if (desc.ByteWidth > 0 && pRender_)
	if (buff_bytes > 0 && pRender_)
	{
		ComPtr<ID3D11Buffer> dxbuff_dump = pRender_->createDxApiBuffer((D3D11_BIND_FLAG)0, buff_bytes, false, true, nullptr);
		if (dxbuff_dump)
		{
			pRender_->copyBuffers(dxbuff_dump.Get(), dxbuff);
			void* sour = pRender_->openDxApiBuffer(dxbuff_dump.Get());
			if (sour)
			{
				//BYTE* bytes_array = new BYTE[buff_bytes];
				//if (bytes_array)
				{
					memcpy(dest, sour, buff_bytes);
					//*dest = bytes_array;
				}

				pRender_->closeDxApiBuffer(dxbuff_dump.Get());
			}
			else
				return -4;

			return 0;
		}

		return -5;
	}

	return -6;
}


Texture DirectX11Compute::createImage(UINT width, UINT height, ITexture::PixelFormat format, ITexture::Type type, uint32_t flags, BYTE* pPixels)
{
	if (nullptr == pRender_)
		return nullptr;

	return pRender_->createTexture(width, height, format, type, flags, pPixels);
}


void DirectX11Compute::releaseImage(Texture& texture)
{
	if (pRender_)
		pRender_->releaseTexture(texture);
}


ComputeTask DirectX11Compute::createComputeTask(cstring csFile, CShaderMacroStrings defines)
{
	if (nullptr == pRender_)
		return nullptr;

	ID3D11ComputeShader* shader = pRender_->createComputeShader(csFile, defines);
	if (shader)
	{
		ComputeTaskDX11 task = std::shared_ptr<ComputeTaskDX11impl>(new ComputeTaskDX11impl);
		if (task)
			task->pShader_ = shader;

		return task;
	}
	else
		return nullptr;
}


void DirectX11Compute::setComputeTask(ComputeTask task)
{
	if (nullptr == pRender_ || nullptr == task)
		return;

	ComputeTaskDX11 pBufferDX11 = std::dynamic_pointer_cast<ComputeTaskDX11impl>(task);
	if (pBufferDX11)
	{
		pRender_->setComputeShader(pBufferDX11->pShader_);
	}
}


ComputeBuffer DirectX11Compute::createConstantBuffer(size_t bytes)
{
	if (pRender_)
		return pRender_->createConstantComputeBuffer(bytes);
	else
		return nullptr;
}


void DirectX11Compute::writeBuffer(ComputeBuffer buffer, const void* data, size_t bytes)
{
	if (pRender_)
	{
		UABufferDX11* pUABuffer = static_cast<UABufferDX11*>(buffer->getNativeBuffer());
		if (pUABuffer)
		{
			ID3D11Buffer* pDxApiBuffer = pUABuffer->getDxBuffer();
			if (pDxApiBuffer)
			{
				void* pDestination = pRender_->openDxApiBuffer(pDxApiBuffer);
				if (pDestination)
				{	memcpy(pDestination, data, bytes);
					pRender_->closeDxApiBuffer(pDxApiBuffer);
				}
			}
		}
	}
}


void DirectX11Compute::setConstantBuffer(ComputeBuffer buffer, uint32_t slot)
{
	if (nullptr == pRender_)
		return;

	pRender_->setConstantBufferCS(buffer, slot);
}


void DirectX11Compute::setComputeBuffer(ComputeBuffer buffer, uint32_t slot)
{
	if (nullptr == pRender_)
		return;

	//IComputeBuffer* pb = buffer.get();
	ComputeBufferDX11 pBufferDX11 = std::dynamic_pointer_cast<ComputeBufferDX11impl>(buffer);
	if (pBufferDX11)
		pRender_->setUABuffer(pBufferDX11->pUABuffer_, slot);
	else
		pRender_->setUABuffer(nullptr, slot);
}


/*void DirectX11Compute::bindBufferToTextureVS(ComputeBuffer buffer, UINT slot)
{
	if (nullptr == pRender_)
		return;

	//IComputeBuffer* pb = buffer.get();
	//UABufferDX11* pBufferDX11 = dynamic_cast<UABufferDX11*>(pb);
	ComputeBufferDX11 pBufferDX11 = std::dynamic_pointer_cast<ComputeBufferDX11impl>(buffer);
	if (pBufferDX11)
		pRender_->bindUABufferToTextureVS(pBufferDX11->pUABuffer_, slot);
	else
		pRender_->bindUABufferToTextureVS(nullptr, slot);
}*/


void DirectX11Compute::setImage(Texture texture, uint32_t slot)
{
	if (nullptr == pRender_)
		return;

	pRender_->setComputeTexture(texture, slot);
}


void DirectX11Compute::setSampler(uint32_t samplerType, uint32_t slot)
{
	if (nullptr == pRender_)
		return;

	pRender_->setComputeSampler(samplerType, slot);
}


void DirectX11Compute::compute(uint32_t dim_x, uint32_t dim_y, uint32_t num_groups)
{
	if (nullptr == pRender_)
		return;

	pRender_->compute(dim_x, dim_y, num_groups);
}