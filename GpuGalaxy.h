#pragma once

#include <windows.h>
#include <memory>

#include "Framework/Core/Application.h"
#include "ParallelCompute/ParallelCompute.h"
#include "Scene.h"


class GpuGalaxy : public Application
{
public:
	static GpuGalaxy* getInstance();

	virtual int init(HINSTANCE hInstance) final;

	virtual void run() final;

	virtual void processScene(float elapsedTime) final;

	virtual void renderScene(float elapsedTime) final;

	IParallelCompute::Type computeType_ = IParallelCompute::DX11; //DX11 OPENCL

	void createSettingsPanel();

	int testButtonClick();

	int changeGpuCompute();

	int whatObjectsToDraw();

	~GpuGalaxy();

private:
	typedef Application super;

	GpuGalaxy(const GpuGalaxy&) = delete;
	GpuGalaxy& operator= (const GpuGalaxy&) = delete;

	GpuGalaxy();

	int initGalaxy();
	void deleteGalaxy();

	std::unique_ptr<Scene> pScene_ = nullptr;

	ParallelCompute pCompute_ = nullptr;

	Texture pGuiTexture_ = nullptr;
	Panel* pSetsPanel_ = nullptr;

	uint32_t whatObjectsToDraw_ = 0xFFFFFFFF;

	int Fps_ = 0;
	float timeFPS_ = 1.0f;
	char fpsText_[64];
};