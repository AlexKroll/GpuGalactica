#pragma once

#include "../Framework/Utils/PointRect.h"
#include "../Framework/Math/Matrix4x4.h"

#include "BaseObject.h"

class Scene;


class Camera : public BaseObject
{
public:
	void initView(CVec4 pos, CVec4 target, float fov, float _near, float _far, Render pRender);

	Vec4 getDir()  {  return dir_;  }

protected:
	Camera();

private:
	Vec4 target_ = Vec4(0,0,0,1);
	Vec4 up_ = Vec4(0,1,0,0);

	float fov_ = 0.7f;

	float aspect_ = 1.0f;

	float near_ = 10.0f;
	float far_ = 2000.0f;

	int screenWidth_ = 0;
	int screenHeight_ = 0;

	// Transposed matices of the view.
	Matrix4x4 mView_;
	Matrix4x4 mProjection_;
	Matrix4x4 mViewProj_;

	byte states_ = 0;
	static const byte kRotate = 0x01;
	static const byte kZoom   = 0x02;
	static const byte kMove   = 0x04;

	float rotateSpeed_ = 4.0f;

	float zoomSpeed_ = 50.0f;

	float moveSpeed_ = 1.3f;

	Point capturedRotatePoint_;
	Point capturedMovePoint_;

	void buildMatrices();

	void buildViewMatrix(CVec4 pos, CVec4 target, CVec4 up);

	void buildPerspectiveMatrix(float fov, float aspect, float _near, float _far);

friend class Scene;

protected:
	virtual void process(float elapsedTime, Scene* pScene) override;

	virtual void render(Render pRender) override;
};