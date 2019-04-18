#pragma once

#include <cassert>
#include <wrl/client.h>
//using Microsoft::WRL::ComPtr;
	
#include "../Framework/Math/Maths.h"

#include "../Framework/Graphics/Render.h"

class Scene;



class BaseObject
{
public:
	// Location in 3D space.
	Vec4 pos_ = Vec4(0,0,0,1);		// Position. We use 4x components for SIMD optimizations.
	Vec4 orient_ = Vec4(0,0,0,0);	// Orientation: [ roll pitch yaw ] order.
	Vec4 size_ = Vec4(1,1,1,1);		// Scaling.
	Vec4 dir_ = Vec4(0,0,1,0);		// Where the unit looks - to azimuth.

	Matrix4x3T mWorld_;  // Transposed matrix of the world location.

	Color color_ = Color(1,1,1,1);

	Mesh mesh_ = nullptr;

	Texture texture_ = nullptr;

	virtual ~BaseObject() = default;

	void showOrHide(bool bShow);

protected:
	std::string name_;

	uint8_t states_ = kPresent;
	static const uint8_t kPresent = 0x01;

	ConstantBuffer pCBufferVS_ = nullptr;
	ConstantBuffer pCBufferPS_ = nullptr;
	UINT constBuffSlot_ = 0;

	virtual void init(cstring name, CVec4 pos, CVec4 orient, CVec4 size, Mesh pMesh, Texture pTexture, Render pRender);

	void openConstantBuffers(byte*& pConstVS, byte*& pConstPS, Render pRender);
	void closeConstantBuffers(Render pRender);

	virtual void process(float, Scene*) {}

	virtual void render(Render pRender);


friend class Scene;
};
