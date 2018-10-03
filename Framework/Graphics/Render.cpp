#include "../Utils/Utils.h"

#include "Render.h"
//#include "RenderDX9.h"
#include "RenderDX11.h"
//#include "RenderGLES.h"



IRender* IRender::getRender(DeviceType type)
{
	IRender* pRender = nullptr;

	switch (type)
	{
		case DX9:
			//pRender = new RenderDX9;
			break;

		case DX11:
			pRender = new RenderDX11;
			break;

		case GLES20:
			//pRender = new RenderGLES(20);
			break;

		case GLES30:
			//pRender = new RenderGLES(30);
			break;
	}

	return pRender;
}


void IRender::commonInit()
{
	pDefaultFont_ = loadFont("Fonts/Arial9.dds");

	if (bDrawStatistics_)
		pStatisticTexture_ = createTexture(128, 64, ITexture::ColorRGBA8, ITexture::RenderTarget);
}


void IRender::commonRelease()
{
	releaseTexture(pStatisticTexture_);

	SAFE_RELEASE(pDefaultFont_);
}


void IRender::getScreenSize(int& width, int& height)
{
	width = screenW_;
	height = screenH_;
}