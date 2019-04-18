#pragma once

#include <cassert>
#include <wrl/client.h>
//using Microsoft::WRL::ComPtr;

#include "../Defines.h"



// Buffer for storing the constant data (constants/uniforms) for the shaders.
interface IConstantBuffer
{
	virtual void* getNativeBuffer() = 0;

	virtual ~IConstantBuffer() = default;
};

typedef std::shared_ptr<IConstantBuffer> ConstantBuffer;



// Buffer for storing the vertex data for transforming in the vertex shaders.
interface IVertexBuffer
{
	virtual void* getNativeBuffer() = 0;

	virtual UINT getVertexSize() = 0;

	virtual ~IVertexBuffer() = default;
};

typedef std::shared_ptr<IVertexBuffer> VertexBuffer;



// Buffer for storing the indices of the vertices in geometry.
interface IIndexBuffer
{
	virtual void* getNativeBuffer() = 0;

	virtual bool isIndices32() = 0;

	virtual ~IIndexBuffer() = default;
};

typedef std::shared_ptr<IIndexBuffer> IndexBuffer;



