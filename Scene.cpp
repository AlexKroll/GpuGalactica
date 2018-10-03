#include "Scene.h"

static const char* strObjNames[Scene::kObjTypeMax] = { "", "Galo", "GaloJets", "SpiralStars", "SpiralDust", "Supernovas", "Space" };


Scene::Scene(Render pRender, Input* pInput, Gui* pGui, ParallelCompute pCompute)
{
	pRender_ = pRender;
	pInput_ = pInput;
	pGui_ = pGui;
	pCompute_ = pCompute;
}


Scene::~Scene()
{
	for (BaseObject* pUnit : objects_)
		SAFE_DELETE(pUnit);

	ParticleEmitter::finalize();
}


void Scene::createGalaxyObjects()
{
	if (nullptr == pRender_)
		return;

	// Camera
	pCamera_ = createObject<Camera>("Camera", Vec4(0,0,0,1), Vec4(0,0,0,0), Vec4(1,1,1,1), nullptr, nullptr);
	if (pCamera_)
	{	pCamera_->initView(Vec4(0,250,-400,1), Vec4(0,-40,0,1), 0.7f, 20.0f, 4000.0f, pRender_);
	}

	//Texture test1_texture = pRender_->LoadTextureFromFile("Textures/Test1.dds");
	//Billboard* pBillboard = 
		//createObject<Billboard>("Billboard", Vec4(0,0,0,1), Vec4(0,0,0,0), Vec4(50,50,1,1), nullptr, test1_texture);

	spaceTexture_ = pRender_->LoadTextureFromFile("Textures/Space.dds");
	galoTexture_ = pRender_->LoadTextureFromFile("Textures/Galo.dds");
	galoJetsTexture_ = pRender_->LoadTextureFromFile("Textures/GaloJets.dds");
	spiralStarsTexture_ = pRender_->LoadTextureFromFile("Textures/SpiralStars.dds");
	spiralStarDustTexture_ = pRender_->LoadTextureFromFile("Textures/SpiralStarDust.dds");
	supernovaStarsTexture_ = pRender_->LoadTextureFromFile("Textures/SupernovaStars.dds");

	{
		#define Vel Vec3
		#define Size12 Vec2
		#define Life12 Vec2
		#define AlphaFade12 Vec2
		#define NumTexFrames int
		#define Intensity int
		#define Pos Vec4
		#define Orient Vec4
		#define Size Vec4
		#define Period float
		#define SizeRate
		#define Radius12 Vec2
		#define Vel12 Vec2
	}

	// Space.
	{
		ParticleEmitter* pEmitter = nullptr;

		ShaderMacroStrings macros = {
			 std::make_pair("PARTICLE_POS_SPHERE", "")
			,std::make_pair("PARTICLE_SIZE_RANDOM", "")
			,std::make_pair("PARTICLE_RANDOM_TFRAME", "")
			,std::make_pair("PARTICLE_DONT_FACTOR", "")
		};

		pEmitter = createObject<ParticleEmitter>(strObjNames[kObjSpace], Pos(0,0,0,1), Orient(0,0,0,0), Size(1,1,1,0), nullptr, spaceTexture_);
		if (pEmitter)
		{	pEmitter->color_ = Color(0xFFCC9944);

			pEmitter->setProperties(512, Period(0), NumTexFrames(4), Vel(0,0,0), pRender_, pCompute_, Intensity(512),
					Size12(20,30), SizeRate(0), Life12(9999,9999), AlphaFade12(0,0), Radius12(1000,1900), Vel12(0,0), macros);
		}

		pEmitter = createObject<ParticleEmitter>(strObjNames[kObjSpace], Pos(0,0,0,1), Orient(0,0,0,0), Size(1,1,1,0), nullptr, spaceTexture_);
		if (pEmitter)
		{	pEmitter->color_ = Color(0xFFCC9944);

			pEmitter->setProperties(512, Period(0), NumTexFrames(4), Vel(0,0,0), pRender_, pCompute_, Intensity(512),
					Size12(20,30), SizeRate(0), Life12(9999,9999), AlphaFade12(0,0), Radius12(1000,1900), Vel12(0,0), macros);
		}
	}

	int num_spirals = 5;

	// Spiral star dust.
	{
		ParticleEmitter* pEmitter = nullptr;

		ShaderMacroStrings macros = {
			 std::make_pair("PARTICLE_POS_SPIRAL", "")
			,std::make_pair("PARTICLE_SIZE_RANDOM", "")
			,std::make_pair("PARTICLE_RANDOM_TFRAME", "")
			,std::make_pair("PARTICLE_PROC_COLOR", "")
			,std::make_pair("PARTICLE_DONT_FACTOR", "")
			,std::make_pair("PARTICLE_FACTOR_POW", "")
			,std::make_pair("PARTICLE_SPIRAL_CN", "")
			,std::make_pair("PARTICLE_MWORLD", "")
		};
		
		float ang_shift = 2.0f * 3.1516f / static_cast<float>(num_spirals);
		float ang = 0.0f;
		int num_parts = 256;

		for (int spiral = 0; spiral < num_spirals; ++spiral)
		{
			// Size.x - expand radius of the spiral
			// Size.y - angle shift of the spiral

			pEmitter = createObject<ParticleEmitter>(strObjNames[kObjSpiralStarDust], Pos(0,0,0,1), Orient(0,0,0,0), Size(150,ang,0,0), nullptr, spiralStarDustTexture_);
			if (pEmitter)
			{	pEmitter->color_ = Color(0xFFFFAA66);

				pEmitter->setProperties(num_parts, Period(0), NumTexFrames(8), Vel(0,0,0), pRender_, pCompute_, Intensity(num_parts),
					Size12(12,25), SizeRate(0), Life12(9999,9999), AlphaFade12(0,0), Radius12(4,7), Vel12(0,0), macros);

				pEmitter->setExtendedProperties(0xFF997711, 0xFF2255AA, 8.0);
			}

			pEmitter = createObject<ParticleEmitter>(strObjNames[kObjSpiralStarDust], Pos(0,0,0,1), Orient(0,0,0,0), Size(150,ang+0.02f,0,0), nullptr, spiralStarDustTexture_);
			if (pEmitter)
			{	pEmitter->color_ = Color(0xFFFFAA66);

				pEmitter->setProperties(num_parts, Period(0), NumTexFrames(8), Vel(0,0,0), pRender_, pCompute_, Intensity(num_parts),
					Size12(12,25), SizeRate(0), Life12(9999,9999), AlphaFade12(0,0), Radius12(4,7), Vel12(0,0), macros);

				pEmitter->setExtendedProperties(0xFF997711, 0xFF2255AA, 8.0);
			}

			ang += ang_shift;
		}
	}

	// Galo
	{
		ShaderMacroStrings macros = {
			 std::make_pair("PARTICLE_POS_SPHERE", "")
			,std::make_pair("PARTICLE_SIZE_RANDOM", "")
			,std::make_pair("PARTICLE_RANDOM_TFRAME", "")
			,std::make_pair("PARTICLE_PROC_ALPHA", "")
			,std::make_pair("PARTICLE_COLOR_MUL_ALPHA", "")
		};

		ParticleEmitter* pEmitter = createObject<ParticleEmitter>(strObjNames[kObjGalo], Pos(0,0,0,1), Orient(0,0,0,0), Size(11,5,11,1), nullptr, galoTexture_);
		if (pEmitter)
		{	pEmitter->color_ = Color(0xFFFFAA66);
			pEmitter->setProperties(256, Period(0.05f), NumTexFrames(4), Vel(0,0,0), pRender_, pCompute_, Intensity(6),
				Size12(7,11), SizeRate(1.7f), Life12(2,3), AlphaFade12(0.3f,0.7f), Radius12(1,2), Vel12(0,0), macros);
		}
	}

	// Galo jets.
	{
		ShaderMacroStrings macros = {
			 std::make_pair("PARTICLE_POS_SPHERE", "")
			,std::make_pair("PARTICLE_VELOCITY", "")
			,std::make_pair("PARTICLE_SIZE_RANDOM", "")
			,std::make_pair("PARTICLE_RANDOM_TFRAME", "")
			,std::make_pair("PARTICLE_PROC_ALPHA", "")
			,std::make_pair("PARTICLE_COLOR_MUL_ALPHA", "")
		};

		ParticleEmitter* pEmitter = nullptr;

		pEmitter = createObject<ParticleEmitter>(strObjNames[kObjGaloJets], Pos(0,0,0,1), Orient(0,0,0,0), Size(1,0,1,1), nullptr, galoJetsTexture_);
		if (pEmitter)
		{	pEmitter->color_ = Color(0xFFFFAA66);
			pEmitter->setProperties(256, Period(0.05f), NumTexFrames(4), Vel(0,90,0), pRender_, pCompute_, Intensity(2),
				Size12(15,25), SizeRate(-10), Life12(1.0f,1.5f), AlphaFade12(0.1f,0.9f), Radius12(1,2), Vel12(0.9f,1), macros);
		}

		pEmitter = createObject<ParticleEmitter>(strObjNames[kObjGaloJets], Pos(0,0,0,1), Orient(0,0,0,0), Size(1,0,1,1), nullptr, galoJetsTexture_);
		if (pEmitter)
		{	pEmitter->color_ = Color(0xFFFFAA66);
			pEmitter->setProperties(256, Period(0.05f), NumTexFrames(4), Vel(0,-90,0), pRender_, pCompute_, Intensity(2),
				Size12(15,25), SizeRate(-10), Life12(1.0f,1.5f), AlphaFade12(0.1f,0.9f), Radius12(1,2), Vel12(0.9f,1), macros);
		}
	}

	// Spiral stars.
	{
		ParticleEmitter* pEmitter = nullptr;

		ShaderMacroStrings macros = {
			 std::make_pair("PARTICLE_POS_SPIRAL", "")
			,std::make_pair("PARTICLE_SIZE_RANDOM", "")
			,std::make_pair("PARTICLE_RANDOM_TFRAME", "")
			,std::make_pair("PARTICLE_DONT_FACTOR", "")
			,std::make_pair("PARTICLE_FACTOR_POW", "")
			,std::make_pair("PARTICLE_SPIRAL_CN", "")
			,std::make_pair("PARTICLE_MWORLD", "")
		};

		float ang_shift = 2.0f * 3.1516f / static_cast<float>(num_spirals);
		float ang = 0.0f;
		int num_parts = 256+128;

		for (int spiral = 0; spiral < num_spirals; ++spiral)
		{
			// Size.x - expand radius of the spiral
			// Size.y - angle shift of the spiral

			pEmitter = createObject<ParticleEmitter>(strObjNames[kObjSpiralStars], Pos(0,0,0,1), Orient(0,0,0,0), Size(150,ang,0,0), nullptr, spiralStarsTexture_);
			if (pEmitter)
			{	pEmitter->color_ = Color(0xFFFFAA66);

				pEmitter->setProperties(num_parts, Period(0), NumTexFrames(8), Vel(0,0,0), pRender_, pCompute_, Intensity(num_parts),
					Size12(5,11), SizeRate(0), Life12(9999,9999), AlphaFade12(0,0), Radius12(2,3), Vel12(0,0), macros);
			}

			pEmitter = createObject<ParticleEmitter>(strObjNames[kObjSpiralStars], Pos(0,0,0,1), Orient(0,0,0,0), Size(150,ang+0.01f,0,0), nullptr, spiralStarsTexture_);
			if (pEmitter)
			{	pEmitter->color_ = Color(0xFFFFAA66);

				pEmitter->setProperties(num_parts, Period(0), NumTexFrames(8), Vel(0,0,0), pRender_, pCompute_, Intensity(num_parts),
					Size12(5,11), SizeRate(0), Life12(9999,9999), AlphaFade12(0,0), Radius12(2,3), Vel12(0,0), macros);
			}

			ang += ang_shift;
		}
	}

	// Supernova stars.
	{
		ParticleEmitter* pEmitter = nullptr;

		ShaderMacroStrings macros = {
			 std::make_pair("PARTICLE_POS_SPIRAL", "")
			,std::make_pair("PARTICLE_SIZE_RANDOM", "")
			,std::make_pair("PARTICLE_RANDOM_TFRAME", "")
			,std::make_pair("PARTICLE_POS_SPIRAL_RANDOM", "")
			,std::make_pair("PARTICLE_PROC_ALPHA", "")
			,std::make_pair("PARTICLE_COLOR_MUL_ALPHA", "")
		};

		float ang_shift = 2.0f * 3.1516f / static_cast<float>(num_spirals);
		float ang = 0.0f;
		int num_parts = 64;

		for (int spiral = 0; spiral < num_spirals; ++spiral)
		{
			// Size.x - expand radius of the spiral
			// Size.y - angle shift of the spiral

			pEmitter = createObject<ParticleEmitter>(strObjNames[kObjSupernovas], Pos(0,0,0,1), Orient(0,0,0,0), Size(150,ang,0,0), nullptr, supernovaStarsTexture_);
			if (pEmitter)
			{	pEmitter->color_ = Color(0xFFFFCCAA);

				pEmitter->setProperties(num_parts, Period(0.3f), NumTexFrames(4), Vel(0,0,0), pRender_, pCompute_, Intensity(1),
					Size12(5,7), SizeRate(30), Life12(0.7f,1.1f), AlphaFade12(0.1f,0.6f), Radius12(2,3), Vel12(0,0), macros);
			}

			ang += ang_shift;
		}
		
	}

	{
		#undef Vel
		#undef Size12
		#undef Life12
		#undef AlphaFade12
		#undef NumTexFrames
		#undef Intensity
		#undef Pos
		#undef Orient
		#undef Size
		#undef Period
		#undef SizeRate
		#undef Radius12
		#undef Vel12
	}
}


void Scene::setGalaxyObjectsToDraw(uint32_t objFlags)
{
	for (uint32_t obj_type = kObjGalo; obj_type < kObjTypeMax; ++obj_type)
	{
		uint32_t flag = 1 << (obj_type-kObjGalo);
		for (BaseObject* object : objects_)
		{
			if (strObjNames[obj_type] == object->name_)
			{
				if (flag & objFlags)
					object->showOrHide(true);
				else
					object->showOrHide(false);
			}
		}
	}
}


void Scene::initObject(BaseObject* pObject, cstring name, CVec4 pos, CVec4 orient, CVec4 size, Mesh pMesh, Texture pTexture)
{
	if (nullptr == pObject)
		return;

	pObject->init(name, pos, orient, size, pMesh, pTexture, pRender_);

	objects_.push_back(pObject);
}


void Scene::process(float elapsedTime)
{
	for (BaseObject* pUnit : objects_)
		pUnit->process(elapsedTime, this);
}


void Scene::render()
{
	for (BaseObject* pUnit : objects_)
		pUnit->render(pRender_);
}