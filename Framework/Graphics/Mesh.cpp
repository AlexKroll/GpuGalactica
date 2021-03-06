#include "Mesh.h"


void IMesh::release()
{
	// buffers are shared ptr.
	//pVertBuffer->release();
	//pIndBuffer->release();
}


VertexBuffer IMesh::getVertexBuffer()
{
	return pVertBuffer;
}


IndexBuffer IMesh::getIndexBuffer()
{
	return pIndBuffer;
}


byte IMesh::getVertexBufferStride()
{
	if (pVertBuffer)
		return static_cast<byte>(pVertBuffer->getVertexSize());
	else
		return 0;
}