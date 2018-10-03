#include <algorithm>

#include "ParticleEmitter.h"
#include "../Scene.h"

Mesh ParticleEmitter::pQuadMesh = nullptr;
VertexInput ParticleEmitter::pVertexInput = nullptr;
ParallelCompute ParticleEmitter::pCompute = nullptr;
//ShaderProgram ParticleEmitter::pShaderProgram = nullptr;
Texture ParticleEmitter::randomTexture = nullptr;

std::vector<EmitterShaderContext> ParticleEmitter::ShaderContexts;



void ParticleEmitter::init(cstring name, CVec4 pos, CVec4 orient, CVec4 size, Mesh pMesh, Texture pTexture, Render pRender)
{
	BaseObject::init(name, pos, orient, size, pMesh, pTexture, pRender);

	// Quad geometry.
	if (nullptr == pQuadMesh && pRender)
	{
		ParticleEmitter::pVertexInput = pRender->createVertexInput(kVertPosition2 | kVertTexCoord0, 0); //kInstanceId

		ParticleVertex verts[4];
		verts[0] = ParticleVertex{Vec2(-0.5f,  0.5f), Vec2(0.0f, 0.0f)};
		verts[1] = ParticleVertex{Vec2( 0.5f,  0.5f), Vec2(1.0f, 0.0f)};
		verts[2] = ParticleVertex{Vec2(-0.5f, -0.5f), Vec2(0.0f, 1.0f)};
		verts[3] = ParticleVertex{Vec2( 0.5f, -0.5f), Vec2(1.0f, 1.0f)};

		WORD indices[6] = { 0,1,2, 1,3,2 }; // Two triangles - 6 indices.

		pQuadMesh = pRender->createMesh(sizeof(verts), verts, sizeof(ParticleVertex), indices, 2, nullptr);

		// Generate a 1D texture that contains 16-bit float random numbers [0..1]. One texel consists of 4 numbers.
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
				randomTexture = pRender->createTexture1D(kRandomTexSize, ITexture::Float16_4, ITexture::StaticTexture, pb);
			}
		}
	}

	mesh_ = pQuadMesh;
}


//#include "../Framework/Graphics/RenderDX11.h"
int ParticleEmitter::setProperties(int maxParticles, float emitPeriod, int numTexFrames, CVec3 velocity, Render pRender, ParallelCompute pComput,
								int intensity, CVec2 size12, float sizeRate, CVec2 life12, CVec2 alphaFade, CVec2 radius12, CVec2 velocity12,
								CShaderMacroStrings macroses)
{
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

	pCompute = pComput;

	// Constants/uniforms for emitter properties.
	propsCBuffer_ = pRender->createConstantBuffer( sizeof(EmitProps) );

	// Arbitrary read/write buffers for processing the particles.
	partsBuffer_ = pCompute->createUABuffer(maxParticles_, sizeof(Particle), false);
	freeIndsBuffer_ = pCompute->createUABuffer(maxParticles_, sizeof(int), true);  // Indices buffer can Append/Consume in the compute shaders.

	// Instance buffer
	//pInstanceVBuffer_ = pRender->createVertexBuffer(sizeof(UINT) * maxParticles_, false, nullptr, sizeof(UINT));

	// Assemble the macroses according to the emitter properties.
	ShaderMacroStrings emitter_macros =
	{
		std::make_pair("MAX_PARTICLES", std::to_string(maxParticles_)),
		std::make_pair("INTENSITY", std::to_string(intensity))
	};

	emitter_macros.insert(emitter_macros.end(), macroses.begin(), macroses.end());

	// Find an emitter context with certain compute shaders (that compiled by certain set of macroses).
	std::vector<EmitterShaderContext>::iterator it = std::find_if(ShaderContexts.begin(), ShaderContexts.end(),
		[&emitter_macros] (const EmitterShaderContext& context) { return context.macroStrings == emitter_macros; } );

	// There is not found. Create a new context: complie new compute shaders.
	if (it == ShaderContexts.end())
	{
		Shader pInitPartsShader = pCompute->createComputeProgram("ParticleEmitters/csInitParticles.txt", emitter_macros);

		Shader pEmitPartsShader = pCompute->createComputeProgram("ParticleEmitters/csEmitParticles.txt", emitter_macros);

		Shader pProcPartsShader = pCompute->createComputeProgram("ParticleEmitters/csProcParticles.txt", emitter_macros);

		ShaderProgram pShaderProgram = pRender->createShaderProgram("vsParticles.txt", "psTexMulVColor.txt", emitter_macros);

		EmitterShaderContext* pContext_ = new EmitterShaderContext(maxParticles_, pInitPartsShader, pEmitPartsShader, pProcPartsShader,
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

		pCompute->setComputeProgram(context_.pInitPartsShader);
		pCompute->setComputeBuffer(partsBuffer_, 0);
		pCompute->setComputeBuffer(freeIndsBuffer_, 1);

		pCompute->setTexture(randomTexture, 0);
		pCompute->setSampler(IRender::kTexSamplerLinearWrap, 0);

		pCompute->compute(1, 1, 1);

		pCompute->setComputeBuffer(nullptr, 0);
		pCompute->setComputeBuffer(nullptr, 1);

// Post compute dump.
/*
pCompute->setComputeBuffer(nullptr, 0);
pCompute->setComputeBuffer(nullptr, 1);
int hh=0;
//static ID3D11Buffer* DxBufferIndsDebug = 0;
static ID3D11Buffer* DxBufferPartsDebug = 0;
RenderDX11* pRender11 = (RenderDX11*)pRender;
if( DxBufferPartsDebug == 0 )
{	//DxBufferIndsDebug = pRender11->createDxApiBuffer((D3D11_BIND_FLAG)0, maxParticles_ * sizeof(int), false, true, nullptr);
	DxBufferPartsDebug = pRender11->createDxApiBuffer((D3D11_BIND_FLAG)0, maxParticles_ * sizeof(Particle), false, true, nullptr);
}
//pRender11->copyBuffers(DxBufferIndsDebug, (ID3D11Buffer*)freeIndsBuffer_->getNativeBuffer() );
//UINT* pFreeInds = (UINT*)pRender11->openDxApiBuffer(DxBufferIndsDebug);
//pRender11->closeDxApiBuffer(DxBufferIndsDebug);

pRender11->copyBuffers(DxBufferPartsDebug, (ID3D11Buffer*)partsBuffer_->getNativeBuffer() );
Particle* pParts = (Particle*)pRender11->openDxApiBuffer(DxBufferPartsDebug);
pRender11->closeDxApiBuffer(DxBufferPartsDebug);
int kk;*/
	}

	return 0;
}


void ParticleEmitter::setExtendedProperties(CColor color1, CColor color2, float factorPow)
{
	properties_.color1 = color1;
	properties_.color2 = color2;

	properties_.factorPow = factorPow;
}


void ParticleEmitter::finalize()
{
	for (EmitterShaderContext& context : ShaderContexts)
	{
		context.release();
	}

	ShaderContexts.clear();
}


void ParticleEmitter::process(float elapsedTime, Scene*)
{
	//Render pRender = pScene->getRender();

	orient_.y -= 0.02f * elapsedTime;

	if (!(states_ & kPresent))
		return;

	properties_.mWorld.setOrient(orient_);

	//	Write emitter properties into contants
	properties_.pos = pos_;
	properties_.size = size_;
	//properties_.radius = ;
	properties_.time = elapsedTime;

	properties_.randoms = V4Random01();

	pCompute->writeConstantBuffer(propsCBuffer_, properties_);
	pCompute->setConstantBuffer(propsCBuffer_, 0);

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
				pCompute->setComputeProgram(pContext_->pEmitPartsShader);
				pCompute->setComputeBuffer(partsBuffer_, 0);
				pCompute->setComputeBuffer(freeIndsBuffer_, 1);

				pCompute->setTexture(randomTexture, 0);
				pCompute->setSampler(IRender::kTexSamplerLinearWrap, 0);

				pCompute->compute(1, 1, 1);
			}
		}

		if (period_ == 0.0f)
			period_ = -1.0f;
	}

	if (pCompute)
	{
		pCompute->setComputeProgram(pContext_->pProcPartsShader);
		pCompute->setComputeBuffer(partsBuffer_, 0);
		pCompute->setComputeBuffer(freeIndsBuffer_, 1);

		pCompute->compute(1, 1, 1);

		pCompute->setComputeBuffer(nullptr, 0);
		pCompute->setComputeBuffer(nullptr, 1);

// Post compute dump.
/*Render pRender = pScene->getRender();
pCompute->setComputeBuffer(nullptr, 0);
pCompute->setComputeBuffer(nullptr, 1);
int hh=0; //"../Framework/Graphics/RenderDX11.h"
static ID3D11Buffer* DxBufferIndsDebug = 0;
static ID3D11Buffer* DxBufferPartsDebug = 0;
RenderDX11* pRender11 = (RenderDX11*)pRender;
if( DxBufferIndsDebug == 0 )
{	DxBufferIndsDebug = pRender11->createDxApiBuffer((D3D11_BIND_FLAG)0, maxParticles_ * sizeof(int), false, true, nullptr);
	DxBufferPartsDebug = pRender11->createDxApiBuffer((D3D11_BIND_FLAG)0, maxParticles_ * sizeof(Particle), false, true, nullptr);
}
pRender11->copyBuffers(DxBufferIndsDebug, (ID3D11Buffer*)freeIndsBuffer_->getNativeBuffer() );
UINT* pFreeInds = (UINT*)pRender11->openDxApiBuffer(DxBufferIndsDebug);
pRender11->closeDxApiBuffer(DxBufferIndsDebug);

pRender11->copyBuffers(DxBufferPartsDebug, (ID3D11Buffer*)partsBuffer_->getNativeBuffer() );
Particle* pParts = (Particle*)pRender11->openDxApiBuffer(DxBufferPartsDebug);
pRender11->closeDxApiBuffer(DxBufferPartsDebug);
int kk;*/
	}
}


void ParticleEmitter::render(Render pRender)
{
	if (!(states_ & kPresent))
		return;

	EmitterShaderContext* pContext_ = nullptr;
	if (shaderContextInd >= 0 && shaderContextInd < ShaderContexts.size())
		pContext_ = &ShaderContexts[shaderContextInd];
	else
		return;

	if (nullptr == pContext_ || nullptr == pRender || nullptr == pCompute)
		return;

	pCompute->bindUABufferToTextureVS(partsBuffer_, 0);
	pRender->setConstantBufferVS(propsCBuffer_, 0);

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

	pCompute->bindUABufferToTextureVS(0, 0);
}