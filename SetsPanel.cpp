#include "GpuGalaxy.h"

// GUI panel where we can select the objects of the galaxy and the computing API.


void GpuGalaxy::createSettingsPanel()
{
	pSetsPanel_ = pGui_->createPanel(Point(10,40), pGuiTexture_, Rect(0, 0, 275, 169), "Compute settings");
	if (nullptr == pSetsPanel_)
		return;
	pSetsPanel_->color_ = 0xFFFFDDBB;

	//Button* pButton = pGui_->createButton(Point(20,20), pGuiTexture_, Rect(0, 239, 126, 255), "Button1", this, &GpuGalaxy::testButtonClick);
	//pSetsPanel_->addChild(pButton);

	uint32_t check = static_cast<uint32_t>(pCompute_->getType());
	std::vector<std::string> buttons_text;
	buttons_text.push_back("DirectCompute");
	buttons_text.push_back("OpenCL");
	buttons_text.push_back("Cuda (not avail.)");
	ButtonBox* pBox = pGui_->createRadioButtonBox(Point(10,30), pGuiTexture_, check, buttons_text, "Compute type:", this, &GpuGalaxy::changeGpuCompute);
	pSetsPanel_->addChild(pBox);
	pBox->disableItem(4);

	TextLabel* pLabel = pGui_->createTextLabel(Point(10,104), "Mouse:", 0xFFEEBB99);
	pSetsPanel_->addChild(pLabel);

	pLabel = pGui_->createTextLabel(Point(10,120), "Right button - rotation", 0xFFDDAA88);
	pSetsPanel_->addChild(pLabel);

	pLabel = pGui_->createTextLabel(Point(10,136), "Wheel - zooming", 0xFFDDAA88);
	pSetsPanel_->addChild(pLabel);

	pLabel = pGui_->createTextLabel(Point(10,152), "Medium button - moving", 0xFFDDAA88);
	pSetsPanel_->addChild(pLabel);

	buttons_text.clear();
	buttons_text.push_back("Galo");
	buttons_text.push_back("Galo jets");
	buttons_text.push_back("Spiral stars");
	buttons_text.push_back("Spiral star dust");
	buttons_text.push_back("Supernovas");
	buttons_text.push_back("Space");
	pBox = pGui_->createCheckButtonBox(Point(165,30), pGuiTexture_, 0x3F, buttons_text, "Particles:", this, &GpuGalaxy::whatObjectsToDraw);
	pSetsPanel_->addChild(pBox);

	pGui_->activateWidget(pSetsPanel_);
}


int GpuGalaxy::testButtonClick()
{
	return 0;
}


int GpuGalaxy::changeGpuCompute()
{
	IParallelCompute::Type type = static_cast<IParallelCompute::Type>(pGui_->getButtonBoxCheckIndex() + 1);
	if (type != computeType_)
	{
		deleteGalaxy();

		computeType_ = type;

		initGalaxy();
	}

	return 0;
}


int GpuGalaxy::whatObjectsToDraw()
{
	whatObjectsToDraw_ = pGui_->getkButtonBoxCheckFlags();

	if (pScene_)
		pScene_->setGalaxyObjectsToDraw(whatObjectsToDraw_);

	return 0;
}