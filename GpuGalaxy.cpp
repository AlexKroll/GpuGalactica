#include "GpuGalaxy.h"


GpuGalaxy* GpuGalaxy::getInstance()
{
	static GpuGalaxy* pGpuGalaxy = nullptr;
	if (pGpuGalaxy == nullptr)
	{
		pGpuGalaxy = new GpuGalaxy;
	}

	return pGpuGalaxy;
}


GpuGalaxy::GpuGalaxy()
{
}


GpuGalaxy::~GpuGalaxy()
{
	if (pGui_)
		pGui_->destroyWidget(pSetsPanel_);

	pScene_.reset();
}


int GpuGalaxy::init(HINSTANCE hInstance)
{
	int res = super::init(hInstance);
	if (res != S_OK)
		return res;

	// Global settings of the GUI.
	if (pGui_)
	{
		pGuiTexture_ = pRender_->LoadTextureFromFile("Gui/Settings.dds");
		if (nullptr == pGuiTexture_)
			return -1;

		pGui_->setColors(0xFFFFEEBB, 0xFFFFFFFF);
		pGui_->setWidgetHighlighting(0xFFFFEEDD, 0.0f, 0xFFEECCAA, 0.3f);
		pGui_->setButtonBoxData(pGuiTexture_, Rect(492,1,500,9), Rect(502,1,510,9), Rect(492,11,500,19), Rect(502,11,510,19));
	}

	res = initGalaxy();
	if (res != S_OK)
		return res;

	// Settings of the compute.
	createSettingsPanel();

	return S_OK;
}


int GpuGalaxy::initGalaxy()
{
	// Parallel compute (DX11 by default).
	pCompute_ = std::shared_ptr<IParallelCompute>(IParallelCompute::getCompute(computeType_));
	if (nullptr == pCompute_.get())
		return E_NOINTERFACE;

	int res = pCompute_->init(pRender_);
	if (res != S_OK)
		return res;

	// Main scene with galaxy objects.
	pScene_ = std::unique_ptr<Scene>(new Scene(pRender_, pInput_.get(), pGui_, pCompute_));
	if (nullptr == pScene_.get())
		return ERROR_UNINITIALIZED;

	pScene_->createGalaxyObjects();

	pScene_->setGalaxyObjectsToDraw(whatObjectsToDraw_);

	return 0;
}


void GpuGalaxy::deleteGalaxy()
{
	pScene_.reset();
	pCompute_.reset();
}


void GpuGalaxy::run()
{
	super::run();
}


void GpuGalaxy::processScene(float elapsedTime)
{
	if (pScene_)
		pScene_->process(elapsedTime);

	if (pCompute_)
		pCompute_->finish();
}


void GpuGalaxy::renderScene(float elapsedTime)
{
	if (pRender_ == nullptr)
		return;

	if (pScene_)
		pScene_->render();

	//  Draw FPS one time per second.
	{
		Fps_++;
		timeFPS_ += elapsedTime;
		if (timeFPS_ >= 1.0f)
		{
			float fps = static_cast<float>(Fps_) / timeFPS_;
			timeFPS_ = 0.0f;
			Fps_ = 0;
			sprintf_s(fpsText_, "FPS: %d", static_cast<int>(fps));
		}

		CFont pFont = pRender_->getDefaultFont();
		if (pFont)
		{
			//int width, height;
			//pRender_->getScreenSize(width, height);

			pFont->drawText(10, 10, fpsText_, 0xFFFFFFFF);
		}
	}
}