#include <algorithm>

#include "ParticleEmitter.h"
#include "../Scene.h"
#include "../Framework/Graphics/RenderDX11.h"

Mesh ParticleEmitter::pQuadMesh = nullptr;
VertexInput ParticleEmitter::pVertexInput = nullptr;
ParallelCompute ParticleEmitter::pCompute = nullptr;
//ShaderProgram ParticleEmitter::pShaderProgram = nullptr;
Texture ParticleEmitter::randomTexture = nullptr;

std::vector<EmitterShaderContext> ParticleEmitter::ShaderContexts;



int ParticleEmitter::initGeneralData(Render pRender, ParallelCompute compute)
{
	pCompute = compute;

	// The geometry of the quad (billboard).
	if (nullptr == pQuadMesh && pRender && pCompute)
	{
		ParticleEmitter::pVertexInput = pRender->createVertexInput(kVertPosition2 | kVertTexCoord0, 0); //kInstanceId

		ParticleVertex verts[4];
		verts[0] = ParticleVertex{Vec2(-0.5f,  0.5f), Vec2(0.0f, 0.0f)};
		verts[1] = ParticleVertex{Vec2( 0.5f,  0.5f), Vec2(1.0f, 0.0f)};
		verts[2] = ParticleVertex{Vec2(-0.5f, -0.5f), Vec2(0.0f, 1.0f)};
		verts[3] = ParticleVertex{Vec2( 0.5f, -0.5f), Vec2(1.0f, 1.0f)};

		WORD indices[6] = { 0,1,2, 1,3,2 }; // Two triangles - 6 indices.

		pQuadMesh = pRender->createMesh(sizeof(verts), verts, sizeof(ParticleVertex), indices, 2, nullptr);
	}

	// Generate a 1D texture that contains 16-bit float random numbers [0..1]. One texel consists of 4 numbers (RGBA).
	{	::InitRandomGenerator();
		std::unique_ptr<WORD[]> pTextureNumbers = std::make_unique<WORD[]>(kRandomTexSize * 4);
		if (pTextureNumbers)
		{
			WORD* pNumbers = pTextureNumbers.get();
			for (int i = 0; i < kRandomTexSize; ++i, pNumbers += 4)
			{
				pNumbers[0] = Random(0, 65535);
				pNumbers[1] = Random(0, 65535);
				pNumbers[2] = Random(0, 65535);
				pNumbers[3] = Random(0, 65535);
			}

			BYTE* pb = reinterpret_cast<BYTE*>(pTextureNumbers.get());

			//randomTexture = pRender->createTexture(kRandomTexSize, 1, ITexture::Float16_4, ITexture::DefaultUsage, ITexture::kMiscShared, pb);
			//if (randomTexture)
			//	pCompute->createImageIntoTexture(randomTexture, IParallelCompute::kBufferRead);

			randomTexture = pCompute->createImage(kRandomTexSize, 1, ITexture::Float16_4, ITexture::DefaultUsage, IParallelCompute::kBufferRead, pb);
		}
	}

	return 0;
}


void ParticleEmitter::init(cstring name, CVec4 pos, CVec4 orient, CVec4 size, Mesh pMesh, Texture pTexture, Render pRender)
{
	BaseObject::init(name, pos, orient, size, pMesh, pTexture, pRender);
}


int ParticleEmitter::setProperties(int maxParticles, float emitPeriod, int numTexFrames, CVec3 velocity, Render pRender,
								int intensity, CVec2 size12, float sizeRate, CVec2 life12, CVec2 alphaFade, CVec2 radius12, CVec2 velocity12,
								CShaderMacroStrings macroses)
{
	mesh_ = pQuadMesh;

	intensity_ = intensity;

	//emitVolume_ = emitVolume;

	// Fill the properties of the emitter.
	{	maxParticles_ = maxParticles;
		period_ = emitPeriod;

		properties_.velocityDir = velocity;

		properties_.life1 = life12.x;
		properties_.life2 = life12.y;

		properties_.size1 = size12.x;
		properties_.size2 = size12.y;

		properties_.radius1 = radius12.x;
		properties_.radius2 = radius12.y;

		properties_.velocity1 = velocity12.x;
		properties_.velocity2 = velocity12.y;

		// calculate data for alpha procesing.
		float inv_out = 1 - alphaFade.y;
		properties_.alphaFadeInOut = 1 / inv_out / (1 / alphaFade.x + 1 / inv_out);
		properties_.alphaFadeIn = 1.0f / alphaFade.x;
		properties_.alphaFadeOut = -1.0f / inv_out;
		properties_.AFadeOutMax = -properties_.alphaFadeOut;

		properties_.numTexFrames = numTexFrames;
		properties_.uvMulX = 1.0f / (float)numTexFrames;

		properties_.size.w = size_.w = sizeRate;

		properties_.color = color_;
	}

	if (IParallelCompute::DX11 == pCompute->getType())
		computeDimX_ = 1;
	else
		computeDimX_ = maxParticles_;

	// Constants/uniforms for emitter properties.
	propsRenderCBuffer_ = pRender->createConstantBuffer(sizeof(EmitProps));

	//if (IParallelCompute::OPENCL == pCompute->getType())
	//	propsComputeCBuffer_ = pCompute->createBuffer(1, sizeof(EmitProps), IParallelCompute::kBufferRead);
	propsComputeCBuffer_ = pCompute->createConstantBuffer(sizeof(EmitProps));

	// Arbitrary read/write buffers for processing the particles..
	// Array of the particles.
	partsBuffer_ = pCompute->createBuffer(maxParticles_, sizeof(Particle), IParallelCompute::kBufferReadWrite | IParallelCompute::kShared
																		 //| IParallelCompute::kCpuNoAccess
																							);
	// Indices of the free (not processing) particles.
	freeIndsBuffer_ = pCompute->createBuffer(maxParticles_, sizeof(int), IParallelCompute::kBufferReadWrite
																	   | IParallelCompute::kAutoLength
																		);
																		 // kAutoLength - The counter of index buffer may be
																		 // increased/decreased in the compute shaders.
																										  
	// Instance buffer
	//pInstanceVBuffer_ = pRender->createVertexBuffer(sizeof(UINT) * maxParticles_, false, nullptr, sizeof(UINT));

	// Assemble the macroses according to the emitter properties.
	ShaderMacroStrings emitter_macros =
	{
		std::make_pair("MAX_PARTICLES", std::to_string(maxParticles_)),
		std::make_pair("INTENSITY", std::to_string(intensity)),
		std::make_pair("NUM_GROUPS", std::to_string(procGroups_))
	};

	emitter_macros.insert(emitter_macros.end(), macroses.begin(), macroses.end());

	// Find an emitter context with certain compute shaders (that compiled by certain set of macroses).
	std::vector<EmitterShaderContext>::iterator it = std::find_if(ShaderContexts.begin(), ShaderContexts.end(),
		[&emitter_macros] (const EmitterShaderContext& context) { return context.macroStrings == emitter_macros; } );

	// There is not found. Create a new context: complie new compute tasks.
	if (it == ShaderContexts.end())
	{
		ComputeTask pInitPartsTask = pCompute->createComputeTask("ParticleEmitters/InitParticles.txt", emitter_macros);

		ComputeTask pEmitPartsTask = pCompute->createComputeTask("ParticleEmitters/EmitParticles.txt", emitter_macros);

		ComputeTask pProcPartsTask = pCompute->createComputeTask("ParticleEmitters/ProcParticles.txt", emitter_macros);

		ShaderProgram pShaderProgram = pRender->createShaderProgram("vsParticles.txt", "psTexMulVColor.txt", emitter_macros);

		EmitterShaderContext* pContext_ = new EmitterShaderContext(maxParticles_, pInitPartsTask, pEmitPartsTask, pProcPartsTask,
																   pShaderProgram, emitter_macros);
		if (pContext_)
		{	shaderContextInd = static_cast<int16_t>(ShaderContexts.size());
			ShaderContexts.push_back(*pContext_);
		}
		else
			return -1;
	}
	else
	{	shaderContextInd = static_cast<int16_t>(std::distance(ShaderContexts.begin(), it));

		//pShaderContext_ = it._Ptr;
	}

	// Init the particles. Put them outside of view.
	if (shaderContextInd >= 0 && shaderContextInd < ShaderContexts.size() && pCompute)
	{
		EmitterShaderContext& context_ = ShaderContexts[shaderContextInd];

		//pShaderProgram = context_.pDrawProgram;

		pCompute->setComputeTask(context_.pInitPartsTask);

		pCompute->setComputeBuffer(partsBuffer_, 0);
		pCompute->setComputeBuffer(freeIndsBuffer_, 1);

		pCompute->setImage(randomTexture, 2);
		pCompute->setSampler(IRender::kTexSamplerLinearWrap, 2);

		pCompute->compute(computeDimX_, 1, procGroups_);

		pCompute->setComputeBuffer(nullptr, 0);
		pCompute->setComputeBuffer(nullptr, 1);

// Post-compute dump.
static int dump = 0;
if (dump)
{
	std::vector<Particle> parts = pCompute->debugDumpBuffer<Particle>(partsBuffer_);
	std::vector<UINT> inds = pCompute->debugDumpBuffer<UINT>(freeIndsBuffer_);
	dump++;
}
	}

	return 0;
}


void ParticleEmitter::setExtendedProperties(CColor color1, CColor color2, float factorPow)
{
	properties_.color1 = color1;
	properties_.color2 = color2;

	properties_.factorPow = factorPow;
}


void ParticleEmitter::finalize(Render pRender)
{
	if (nullptr == pRender)
		return;

	for (EmitterShaderContext& context : ShaderContexts)
		context.release();
	ShaderContexts.clear();

	pRender->releaseMesh(pQuadMesh);
	pRender->releaseVertexInput(pVertexInput);

	pCompute->releaseImage(randomTexture);
}


void ParticleEmitter::process(float elapsedTime, Scene* pScene)
{
	orient_.y -= 0.02f * elapsedTime;

	if (!(states_ & kPresent))
		return;

	properties_.mWorld.setPos(pos_);
	properties_.mWorld.setOrient(orient_);

	//	Write emitter properties into contants
	properties_.pos = pos_;
	properties_.size = size_;
	//properties_.radius = ;
	properties_.time = elapsedTime;

	properties_.randoms = V4Random01();

	Render render = pScene->getRender();
	render->updateConstantBuffer(propsRenderCBuffer_, &properties_, sizeof(properties_));

	pCompute->writeBuffer(propsComputeCBuffer_, properties_);

	EmitterShaderContext* pContext_ = nullptr;
	if (shaderContextInd >= 0 && shaderContextInd < ShaderContexts.size())
		pContext_ = &ShaderContexts[shaderContextInd];
	else
		return;

	// Emit the particles.
	if (period_ >= 0.0f)
	{	time_ += elapsedTime;
		if (time_ >= period_)
		{	time_ -= period_;

			if (pCompute)
			{
				pCompute->setComputeTask(pContext_->pEmitPartsTask);

				pCompute->setComputeBuffer(partsBuffer_, 0);
				pCompute->setComputeBuffer(freeIndsBuffer_, 1);

				pCompute->setImage(randomTexture, 2);
				pCompute->setSampler(IRender::kTexSamplerLinearWrap, 2);

				pCompute->setConstantBuffer(propsComputeCBuffer_, 3);

				uint32_t Nums = (IParallelCompute::OPENCL == pCompute->getType()) ? intensity_ : computeDimX_;

				pCompute->compute(Nums, 1, procGroups_);

				pCompute->setComputeBuffer(nullptr, 0);
				pCompute->setComputeBuffer(nullptr, 1);
				pCompute->setComputeBuffer(nullptr, 3);

// Post-compute dump.
static int dump = 0;
if (dump)
{	std::vector<Particle> parts = pCompute->debugDumpBuffer<Particle>(partsBuffer_);
	std::vector<UINT> inds = pCompute->debugDumpBuffer<UINT>(freeIndsBuffer_);
	dump++;
}
			}
		}

		if (period_ == 0.0f)
			period_ = -1.0f;
	}

	// Processing the particles.
	if (pCompute)
	{
		pCompute->setComputeTask(pContext_->pProcPartsTask);

		pCompute->setComputeBuffer(partsBuffer_, 0);
		pCompute->setComputeBuffer(freeIndsBuffer_, 1);

		uint32_t const_slot = (IParallelCompute::OPENCL == pCompute->getType()) ? 2 : 3;
		pCompute->setConstantBuffer(propsComputeCBuffer_, const_slot);

		pCompute->compute(computeDimX_, 1, procGroups_);

		pCompute->setComputeBuffer(nullptr, 0);
		pCompute->setComputeBuffer(nullptr, 1);
		pCompute->setComputeBuffer(nullptr, 3);

// Post-compute dump.
static int dump = 0;
if (dump)
{	std::vector<Particle> parts = pCompute->debugDumpBuffer<Particle>(partsBuffer_);
	std::vector<UINT> inds = pCompute->debugDumpBuffer<UINT>(freeIndsBuffer_);
	dump++;
}
	}
}


void ParticleEmitter::render(Render pRender)
{
//return;
	if (!(states_ & kPresent))
		return;

	EmitterShaderContext* pContext_ = nullptr;
	if (shaderContextInd >= 0 && shaderContextInd < ShaderContexts.size())
		pContext_ = &ShaderContexts[shaderContextInd];
	else
		return;

	if (nullptr == pContext_ || nullptr == pRender || nullptr == pCompute)
		return;

	pRender->bindComputeBufferToTextureVS(partsBuffer_, 0);
	pRender->setConstantBufferVS(propsRenderCBuffer_, 3);

	pRender->setVertexInput(pVertexInput);
	pRender->setShaderProgram(pContext_->pDrawProgram);

	pRender->setVertexBuffer(mesh_->getVertexBuffer(), 0);
	pRender->setIndexBuffer(mesh_->getIndexBuffer());

	//pRender->setVertexBuffer(pInstanceVBuffer_, 1);
	pRender->setVertexBuffer(nullptr, 1);

	pRender->setTexture(0, texture_);

	pRender->setRasterState(IRender::kRasterCullOff);
	pRender->setAlphaBlendAdditive();
	pRender->setEnableDepthWrite(false);

	pRender->drawIndexedTrianglesInstanced(2, maxParticles_);
	//pRender->drawIndexedTriangles(2);

	pRender->bindComputeBufferToTextureVS(nullptr, 0);
}