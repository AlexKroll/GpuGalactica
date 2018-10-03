#include "Billboard.h"
#include "Camera.h"
#include "../Scene.h"

Mesh Billboard::pQuadMesh = nullptr;
VertexInput Billboard::pVertexInput = nullptr;
ShaderProgram Billboard::pShaderProgram = nullptr;



void Billboard::init(cstring name, CVec4 pos, CVec4 orient, CVec4 size, Mesh pMesh, Texture pTexture, Render pRender)
{
	BaseObject::init(name, pos, orient, size, pMesh, pTexture, pRender);

	// Quad geometry.
	if (nullptr == pQuadMesh && pRender)
	{
		Billboard::pVertexInput = pRender->createVertexInput(kVertPosition | kVertTexCoord0, 0);

		Vertex verts[4];
		verts[0] = Vertex{Vec3(-0.5f, -0.5f, 0.0f),  Vec2(1.0f, 1.0f)};
		verts[1] = Vertex{Vec3( 0.5f, -0.5f, 0.0f),  Vec2(0.0f, 1.0f)};
		verts[2] = Vertex{Vec3(-0.5f,  0.5f, 0.0f),  Vec2(1.0f, 0.0f)};
		verts[3] = Vertex{Vec3( 0.5f,  0.5f, 0.0f),  Vec2(0.0f, 0.0f)};

		WORD indices[6] = { 0, 3, 2, 3, 0, 1 }; // Two triangles - 6 indices.

		pQuadMesh = pRender->createMesh(sizeof(verts), verts, sizeof(Vertex), indices, 2, nullptr);
/*
		// Shader.
		TCHAR* vsText =
	   "cbuffer Global : register(b2) \n  \
		{	float4x3  mView		: register(c0); \n  \
			float4x4  mProj		: register(c3); \n \
			float4x4  mViewProj : register(c7); \n \
		};\n  \
											\
		float4x3 mWorld : register(c0); \n		\
		float4 Size : register(c3);		\n	\
											\
		void Main(in float4 VertexPos: POSITION, in float2 VertexTexCrd: TEXCOORD0, \n	\
				  out float4 OutPos: SV_Position, out float2 OutTexCrd: TEXCOORD0) \n	\
		{ \n \
			OutPos = VertexPos * Size; \n \
			OutPos.xyz = mul(OutPos, mWorld); \n  \
			OutPos = mul(OutPos, mViewProj); \n  \
			OutTexCrd = VertexTexCrd; \n  \
		}";

		TCHAR* psText =
		   "Texture2D Tex: register(t0);  \
			sampler Samp: register(s0);   \
			float4 Color : register(c0);  \
			void Main(in float4 Pos: SV_POSITION, in float2 TexCoord: TEXCOORD0, out float4 ColorOut : SV_Target ) \
			{ \
				ColorOut = Tex.Sample(Samp,TexCoord) * Color; \
			}";
*/
		//pShaderProgram = pRender->createShaderProgram(vsText, psText);
		//std::pair<std::string, std::string> def("a", "b");
		//ShaderMacroStrings defs = { std::make_pair("a1", "a2"), std::make_pair("b1", "b2") };
		//defs.emplace_back("c1", "c2");
		pShaderProgram = pRender->createShaderProgram("vsBillboard.txt", "psTexMulColor.txt", ShaderMacroStrings());
	}

	// Prepare constant buffers (uniforms).
	if (nullptr == pCBufferVS_)
	{	pCBufferVS_ = pRender->createConstantBuffer(sizeof(mWorld_) + sizeof(size_));
	}

	if (nullptr == pCBufferPS_)
	{	pCBufferPS_ = pRender->createConstantBuffer(sizeof(color_));
	}

	mesh_ = pQuadMesh;
}


void Billboard::process(float, Scene* pScene)
{
	if (!(states_ & kPresent))
		return;

	// Rotate the quad mesh to the camera.

	Camera* pCamera = pScene->getCamera();

	dir_ = pCamera->getDir();
	dir_ = dir_.negate();

	OrientFromDir(orient_, dir_);

	mWorld_.setPos(pos_);
	mWorld_.setOrient(orient_);
}


void Billboard::render(Render pRender)
{
	if (nullptr == mesh_ || nullptr == pRender || !(states_ & kPresent))
		return;

	byte *pConstVS = nullptr, *pConstPS = nullptr;
	openConstantBuffers(pConstVS, pConstPS, pRender);

	if (pConstVS)
	{	memcpy(pConstVS, &mWorld_, sizeof(mWorld_));
		pConstVS += sizeof(mWorld_);
		memcpy(pConstVS, &size_, sizeof(size_));
	}

	if (pConstPS)
	{	memcpy(pConstPS, &color_, sizeof(color_));
	}

	closeConstantBuffers(pRender);

	pRender->setVertexInput(pVertexInput);
	pRender->setShaderProgram(pShaderProgram);

	pRender->setEnableDepthBuffer(true);
	pRender->setEnableDepthWrite(true);

	pRender->setDefaultSampler(0);
	pRender->setDefaultRasterState();

	pRender->setAlphaBlendOff();

	BaseObject::render(pRender);
}