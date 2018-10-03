#include "../Framework/Core/Input.h"
#include "../Framework/Gui/Gui.h"

#include "Camera.h"
#include "../Scene.h"



Camera::Camera()
{
	pos_ = Vec4(0.0f, 200.0f, -200.0f, 1.0f);

	constBuffSlot_ = 2;
}


void Camera::initView(CVec4 pos, CVec4 target, float fov, float _near, float _far, Render pRender)
{
	pos_ = pos,  target_ = target;

	fov_ = fov;

	near_ = _near,  far_ = _far;

	pRender->getScreenSize(screenWidth_, screenHeight_);

	aspect_ = static_cast<float>(screenWidth_) / static_cast<float>(screenHeight_);

	dir_ = target_ - pos_;
	dir_.normalize();

	OrientFromDir(orient_, dir_);

	if (nullptr == pCBufferVS_)
	{	pCBufferVS_ = pRender->createConstantBuffer(sizeof(mView_) - 16 + sizeof(mProjection_) + sizeof(mViewProj_)); // mView_ used as Matrix4x3
	}
}


void Camera::buildMatrices()
{
	buildViewMatrix(pos_, target_, up_);
	mView_ = mView_.transpose();

	buildPerspectiveMatrix(fov_, aspect_, near_, far_);
	mProjection_ = mProjection_.transpose();

	mViewProj_ = MatrixMultiply(mProjection_, mView_);
}


void Camera::buildViewMatrix(CVec4 pos, CVec4 target, CVec4 up)
{
	Vec4 dir = target - pos;
	dir.normalize();

	Vec4 right = Cross(up, dir);
	right.normalize();

	Vec4 up2 = Cross(dir, right);
	up2.normalize();
 
    mView_.m[0][0] = right.x;
    mView_.m[1][0] = right.y;
    mView_.m[2][0] = right.z;
    mView_.m[3][0] = -Dot3(right, pos); 
    mView_.m[0][1] = up2.x; 
    mView_.m[1][1] = up2.y; 
    mView_.m[2][1] = up2.z; 
    mView_.m[3][1] = -Dot3(up2, pos); 
    mView_.m[0][2] = dir.x; 
    mView_.m[1][2] = dir.y; 
    mView_.m[2][2] = dir.z; 
    mView_.m[3][2] = -Dot3(dir, pos); 
    mView_.m[0][3] = 0.0;
    mView_.m[1][3] = 0.0;
    mView_.m[2][3] = 0.0;
    mView_.m[3][3] = 1.0;
}


void Camera::buildPerspectiveMatrix(float fov, float aspect, float _near, float _far)
{
	float half_fov = fov * 0.5f;
	float fovy = cosf(half_fov) / sinf(half_fov);
	float fovy2 = _far / (_far - _near);

	float f11 = fovy / aspect;
	float f43 = -fovy2 * _near;

	mProjection_._11 = f11;		mProjection_._12 = 0.0;		mProjection_._13 = 0.0;		mProjection_._14 = 0.0;
	mProjection_._21 = 0.0;		mProjection_._22 = fovy;	mProjection_._23 = 0.0;		mProjection_._24 = 0.0;
	mProjection_._31 = 0.0;		mProjection_._32 = 0.0;		mProjection_._33 = fovy2;	mProjection_._34 = 1.0;
	mProjection_._41 = 0.0;		mProjection_._42 = 0.0;		mProjection_._43 = f43;		mProjection_._44 = 0.0;
}


void Camera::process(float time, Scene* pScene)
{
	Gui* pGui = pScene->getGui();
	if (nullptr == pGui->getFocusWidget())
	{
		Input* pInput = pScene->getInput();
		if (pInput)
		{
			// Rotation.
			if (pInput->isMouseButtonDown(Input::mouseButtonRight))
			{
				if (!(states_ & kRotate))
				{	states_ |= kRotate;
					capturedRotatePoint_ = pInput->getMousePoint();
				}

				if (states_ & kRotate)
				{
					Point pnt = pInput->getMousePoint() - capturedRotatePoint_;
					float dx = static_cast<float>(pnt.x) * rotateSpeed_ * time;
					float dy = static_cast<float>(pnt.y) * rotateSpeed_ * time;

					// yaw rotation
					Vec4 vec = pos_ - target_;
					float sn = sinf(dx);
					float cs = cosf(dx);
					pos_.x = vec.x * cs + vec.z * sn + target_.x;
					pos_.z = vec.z * cs - vec.x * sn + target_.z;

					dir_ = target_ - pos_;
					dir_.normalize();

					OrientFromDir(orient_, dir_);

					// pitch rotation
					orient_.x -= dy;
					if (orient_.x < -1.57f)
						orient_.x = -1.57f;
					if (orient_.x > 1.57f)
						orient_.x = 1.57f;

					vec = pos_ - target_;
					float distance = vec.length();
					pos_.y = target_.y - distance * sinf(orient_.x);
					pos_.x = target_.x - distance * cosf(orient_.x) * sinf(orient_.y);
					pos_.z = target_.z - distance * cosf(orient_.x) * cosf(orient_.y);

					capturedRotatePoint_ = pInput->getMousePoint();
				}
			}
			else
			{	states_ &= ~kRotate;
			}

			// Zoom.
			int zoom = pInput->getMouseMediumWheelRolling();
			if (zoom != 0)
			{
				float zoom_dir = (zoom > 0) ? 1.0f : -1.0f;
				float speed = zoomSpeed_ * zoom_dir * time;

				dir_ = target_ - pos_;

				pos_ += dir_ * speed;

				dir_ = target_ - pos_;
				float len = dir_.length_normalize();
				if (len < near_)
				{	pos_ = target_ - dir_ * near_;
				}
				else if (len > (far_ * 0.5f))
				{	pos_ = target_ - dir_ * far_ * 0.49f;
				}
			}
			else
			{	states_ &= ~kZoom;
			}

			// Move
			if (pInput->isMouseButtonDown(Input::mouseButtonMedium))
			{
				if (!(states_ & kMove))
				{	states_ |= kMove;
					capturedMovePoint_ = pInput->getMousePoint();
				}

				if (states_ & kMove)
				{
					dir_ = target_ - pos_;
					float len = dir_.length_normalize();

					Point pnt = pInput->getMousePoint() - capturedMovePoint_;
					float dx = static_cast<float>(pnt.x) * moveSpeed_ * len * time;
					float dy = static_cast<float>(pnt.y) * moveSpeed_ * len * time;

					mWorld_.setOrient(orient_);

					Vec4 vec(-dx, dy, 0, 1);
					vec = mWorld_.transformCoord(vec);
					vec.w = 0;

					pos_ += vec;
					target_ += vec;

					capturedMovePoint_ = pInput->getMousePoint();
				}
			}
			else
			{	states_ &= ~kMove;
			}
		}
	}

	buildMatrices();

	dir_ = target_ - pos_;
	dir_.normalize();
}


void Camera::render(Render pRender)
{
	if (nullptr == pCBufferVS_)
		return;

	// Write matrices to constant buffers.
	byte* pMatrices = static_cast<byte*>(pRender->openConstantBuffer(pCBufferVS_));
	if (pMatrices)
	{	memcpy(pMatrices, &mView_, sizeof(mView_) - 16);
		pMatrices += sizeof(mView_) - 16;
		memcpy(pMatrices, &mProjection_, sizeof(mProjection_));
		pMatrices += sizeof(mProjection_);
		memcpy(pMatrices, &mViewProj_, sizeof(mViewProj_));
		pRender->closeConstantBuffer(pCBufferVS_);
	}

	pRender->setConstantBufferVS(pCBufferVS_, constBuffSlot_);
}