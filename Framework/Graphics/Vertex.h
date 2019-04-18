#pragma once

#include <windows.h>

#include "../Math/Vec2.h"
#include "../Math/Vec3.h"
#include "../Math/Vec4.h"



// Vertex component flags.

const uint32_t kVertPosition	= 0x00000001;	// 3 floats xyz
const uint32_t kVertPosition2	= 0x00000002;	// 2 floats xy
const uint32_t kVertNormal		= 0x00000004;	// 3 floats
const uint32_t kVertTexCoord0	= 0x00000008;	// 2 floats of uv coords
const uint32_t kVertTexCoord1	= 0x00000010;	// ...
const uint32_t kVertTexCoord2	= 0x00000020;
const uint32_t kVertTexCoord3	= 0x00000040;
const uint32_t kVertTangent		= 0x00000080;	// 3 floats
const uint32_t kVertBinormal	= 0x00000100;	// 3 floats
const uint32_t kVertBoneIndices	= 0x00000200;	// 4 indices (4 bytes, mapping to int 0..255)
const uint32_t kVertBoneWeights	= 0x00000400;	// 4 weights (4 bytes, mapping to float 0..1)
const uint32_t kVertColor0		= 0x00000800;	// 4 bytes (mapping to float 0..1)
const uint32_t kVertColor1		= 0x00001000;	// 4 bytes (mapping to float 0..1)
//const uint32_t kInstanceId		= 0x00002000;	// 4 bytes of the instance id

const uint32_t kInstFloat4_0	= 0x80000000;	// 4 floats for instance
const uint32_t kInstFloat4_1	= 0x40000000;	// ...
const uint32_t kInstFloat3_0	= 0x20000000;	// 3 floats for instance
const uint32_t kInstFloat3_1	= 0x10000000;	// ...
const uint32_t kInstFloat2_0	= 0x08000000;	// 2 floats for instance
const uint32_t kInstFloat2_1	= 0x04000000;	// ...

const uint32_t kInstBegin = kInstFloat4_0;
const uint32_t kInstEnd = kInstFloat2_1;


const int kMaxBindVertexBuffers = 4;


interface IVertexInput
{
	BYTE numVertexComponents = 0;
	BYTE strides[kMaxBindVertexBuffers] = {0};

	virtual void* getNativeVertexLayout() = 0;

	virtual ~IVertexInput() {}
};

//typedef IVertexInput* VertexInput;
typedef std::shared_ptr<IVertexInput> VertexInput;




// Popular vertices

struct VertexPosUv
{
	Vec3 pos;
	Vec2 tcoord;

	VertexPosUv() = default;

	VertexPosUv( CVec3 _pos, CVec2 _tcoord )
	{
		pos = _pos;
		tcoord = _tcoord;
	}
};



struct VertexPosNormUv
{
	Vec3 pos;
	Vec3 normal;
	Vec2 tcoord;

	VertexPosNormUv() = default;

	VertexPosNormUv( CVec3 _pos, CVec3 _normal, CVec2 _tcoord )
	{
		pos = _pos;
		normal = _normal;
		tcoord = _tcoord;
	}
};




