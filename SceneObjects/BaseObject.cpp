#include "BaseObject.h"




void BaseObject::showOrHide(bool bShow)
{
	if (bShow)
		states_ |= kPresent;
	else
		states_ &= ~kPresent;
}


void BaseObject::init(cstring name, CVec4 pos, CVec4 orient, CVec4 size, Mesh pMesh, Texture pTexture, Render)
{
	name_ = name;

	pos_ = pos,  size_= size,  orient_ = orient;

	mesh_ = pMesh;

	texture_ = pTexture;
}


void BaseObject::openConstantBuffers(byte*& pConstVS, byte*& pConstPS, Render pRender)
{
	pConstVS = pConstPS = nullptr;

	if (pCBufferVS_)
	{	pConstVS = static_cast<byte*>(pRender->openConstantBuffer(pCBufferVS_));
	}

	if (pCBufferPS_)
	{	pConstPS = static_cast<byte*>(pRender->openConstantBuffer(pCBufferPS_));
	}
}


void BaseObject::closeConstantBuffers(Render pRender)
{
	if (pCBufferVS_)
		pRender->closeConstantBuffer(pCBufferVS_);

	if (pCBufferPS_)
		pRender->closeConstantBuffer(pCBufferPS_);
}



void BaseObject::render(Render pRender)
{
	if (nullptr == mesh_ || nullptr == pRender)
		return;

	pRender->setConstantBufferVS(pCBufferVS_, constBuffSlot_);
	pRender->setConstantBufferPS(pCBufferPS_, constBuffSlot_);

	pRender->setVertexBuffer(mesh_->getVertexBuffer(), 0);
	pRender->setIndexBuffer(mesh_->getIndexBuffer());

	pRender->setTexture(0, texture_);

	pRender->setDefaultRasterState();
	pRender->setAlphaBlendOff();
	pRender->setEnableDepthBuffer(true);
	pRender->setEnableDepthWrite(true);

	pRender->drawIndexedTriangles(2);
}