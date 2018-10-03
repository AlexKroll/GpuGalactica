#pragma once

#include <windows.h>
#include <vector>
#include <memory>

#include "Framework/Graphics/Render.h"
#include "Framework/Core/Input.h"
#include "Framework/Gui/Gui.h"

#include "SceneObjects/ParticleEmitter.h"
#include "SceneObjects/Camera.h"
#include "SceneObjects/Billboard.h"



// The main scene of the demo. It includes and processes the objects in 3D space.

class Scene
{
public:
	Scene(Render pRender, Input* pInput, Gui* pGui, ParallelCompute pCompute);
	~Scene();

	void createGalaxyObjects();

	void setGalaxyObjectsToDraw(uint32_t objFlags);

	template <class T>
	T* createObject(cstring name, CVec4 pos, CVec4 orient, CVec4 size, Mesh pMesh, Texture pTexture)
	{
		T* pObject = new T;
		if (pObject)
			initObject(pObject, name, pos, orient, size, pMesh, pTexture);
		return pObject;
	}

	Camera* getCamera()  {  return pCamera_;  }

	Input* getInput()  {  return pInput_;  }

	Gui* getGui()  {  return pGui_;  }

	Render getRender()  {  return pRender_;  }

	void process(float elapsedTime);

	void render();

	// What objects to process and draw.
	static const uint32_t kObjGalo			 = 1;
	static const uint32_t kObjGaloJets		 = 2;
	static const uint32_t kObjSpiralStars	 = 3;
	static const uint32_t kObjSpiralStarDust = 4;
	static const uint32_t kObjSupernovas	 = 5;
	static const uint32_t kObjSpace			 = 6;
	static const uint32_t kObjTypeMax		 = 7;

private:
	std::vector<BaseObject*> objects_;

	Render pRender_ = nullptr;

	Input* pInput_ = nullptr;

	Gui* pGui_ = nullptr;

	ParallelCompute pCompute_ = nullptr;

	Camera* pCamera_ = nullptr;

	Texture spaceTexture_;
	Texture galoTexture_;
	Texture galoJetsTexture_;
	Texture spiralStarsTexture_;
	Texture spiralStarDustTexture_;
	Texture supernovaStarsTexture_;

	void initObject(BaseObject* pObject, cstring name, CVec4 pos, CVec4 orient, CVec4 size, Mesh pMesh, Texture pTexture);
};
