#pragma once

#include <cassert>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include "../Defines.h"



interface IConstantBuffer				// Buffer for storing the constant data (constants/uniforms) for the shaders.
{
	virtual void* getNativeBuffer() = 0;

	virtual ~IConstantBuffer() = default;
};

typedef std::shared_ptr<IConstantBuffer> ConstantBuffer;




interface IVertexBuffer					// Buffer for storing the vertex data for transforming in the vertex shaders.
{
	virtual void* getNativeBuffer() = 0;

	virtual UINT getVertexSize() = 0;

	virtual ~IVertexBuffer() = default;
};

//typedef IVertexBuffer* VertexBuffer;
typedef std::shared_ptr<IVertexBuffer> VertexBuffer;




interface IIndexBuffer					// Buffer for storing the indices of the vertices in geometry.
{
	virtual void* getNativeBuffer() = 0;

	virtual bool isIndices32() = 0;

	virtual ~IIndexBuffer() = default;
};

//typedef IIndexBuffer* IndexBuffer;
typedef std::shared_ptr<IIndexBuffer> IndexBuffer;



interface IUABuffer						// Unordered access buffer for arbitrary reading/writing the data by the shaders.
{
	virtual void* getNativeBuffer() = 0;

	virtual ~IUABuffer() = default;
};

typedef std::shared_ptr<IUABuffer> UABuffer;