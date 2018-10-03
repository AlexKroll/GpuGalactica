#pragma once

#include "Buffers.h"


class IMesh //abstract
{
public:
	VertexBuffer getVertexBuffer();

	IndexBuffer getIndexBuffer();

	byte getVertexBufferStride();

	enum IndexType : byte
	{
		Ind16,
		Ind32,
	};

	struct MeshUsage
	{
		byte boneIndex = 0;
		byte boneWeightIndex = 0;
		IndexType indType = Ind16;
	};

	friend class IRender;


protected:
	IMesh() = default;
	//virtual ~IMesh() = default;

	VertexBuffer pVertBuffer = nullptr;
	IndexBuffer pIndBuffer = nullptr;

	IndexType indexType_ = Ind16;

	byte vertexSize_ = 0;

	int numPrimitives_ = 0;

	void release();

	friend class IRender;
	friend class RenderDX11;
};

typedef std::shared_ptr<IMesh> Mesh;