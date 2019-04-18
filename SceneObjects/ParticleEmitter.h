#pragma once

#include "../ParallelCompute/ParallelCompute.h"

#include "BaseObject.h"


struct EmitterShaderContext;

// Particle emitter.
// It processes and renders the particle array. The array has fixed length.
// Live particles are in view, died are outside.

class ParticleEmitter : public BaseObject
{
public:
	int setProperties(int maxParticles, float emitPeriod, int numTexFrames, CVec3 velocity, Render pRender,
					  int intensity, CVec2 size12, float sizeRate, CVec2 life12, CVec2 alphaFade, CVec2 radius12, CVec2 velocity12,
					  CShaderMacroStrings macros);

	void setExtendedProperties(CColor color1, CColor color2, float factorPow);

protected:
	virtual void init(cstring name, CVec4 pos, CVec4 orient, CVec4 size, Mesh pMesh, Texture pTexture, Render pRender) override;

	virtual void process(float elapsedTime, Scene* pScene) override;

	virtual void render(Render pRender) override;

	static int initGeneralData(Render pRender, ParallelCompute pCompute);

	static Mesh pQuadMesh;
	static VertexInput pVertexInput;
	static ParallelCompute pCompute;
	static const int kRandomTexSize = 4096;
	static Texture randomTexture;	// This image consists of random values in 0..1 range.

	static std::vector<EmitterShaderContext> ShaderContexts;

	static void finalize(Render pRender);

	struct ParticleVertex
	{
		Vec2 pos;
		Vec2 tcoords;
	};

	struct Particle				// It should correspond to file "csParticle.txt"
	{
		Vec4	pos;			// xyz - position, w - size
		Vec4	vel;			// xyz - velocity, w - rate of size
		Vec4	accel;			// xyz - accelerate, w - accelerate of size
		float   time;
		float   life;
		float	factor01;
		unsigned short random;
		unsigned short uv_add_x;
	};


	float period_ = 0.0f;		// The period of emitting of the particles.
	float time_ = 0.0f;

	int maxParticles_ = 0;		// Max particles per emitter. This the amount of the quads per DIP.

	int computeDimX_ = 0;		// Number of compute threads (kernels).

	int intensity_ = 0;			// Number of emitted particles per frame.

	uint32_t procGroups_ = 16;	// Number of hardware units (group of threads, compute units) on GPU.
								// It may be various for diffrenet hardware.

	struct EmitProps			// The emitter properties: emittimg volume (sphere, cube, etc), size, radius, etc.
	{							// It should correspond to file "csEmitProperties.txt"
		// c0
		Vec4 pos;				// position of the emitter
		Vec4 size;				// size of the emitting volume. w - is the rate of size
		// c2
		float pack1x;
		float time;				// elapsed time of the application frame
		float uvMulX;			// UvMulX = 1/AmountOfTextureFrames
		int   numTexFrames;		// Amount of texture frames.
		// c3
		float life1;			// random duration of the particle life..
		float life2;			// ..from life1 to life2
		float radius1;			// radius if shpere is used for emitting..
		float radius2;			// ..from radius1 to radius2
		// c4
		float size1;			// random size of the particle..
		float size2;			// .. from size1 to size2
		float pack4z;
		float pack4w;

		// c5
		Vec4 randoms;			// 4x random numbers [0..1]

		// c6
		float velocity1;			// Random particle velocity from velocity1 to velocity2
		float velocity2;
		float pack6z;
		float pack6w;

		// c7
		float alphaFadeInOut;	// .x  1-alpha_fade in/out,   .y  alpha_in,   .z  1-alpha_out
		float alphaFadeIn;
		float alphaFadeOut;
		float AFadeOutMax;

		// c8
		Color color;

		// c9
		Vec3 velocityDir;
		float factorPow;

		// c10
		Color color1;
		Color color2;

		// c12
		Matrix4x3T mWorld;
	};
	EmitProps properties_;
	ConstantBuffer propsRenderCBuffer_ = nullptr;	//  We can`t share constant buffer between DX and CL.
	ComputeBuffer propsComputeCBuffer_ = nullptr;	//  So we have to keep two copies.

	ComputeBuffer partsBuffer_ = nullptr;		// GPU buffer where the particles lay.
	ComputeBuffer freeIndsBuffer_ = nullptr;	// GPU buffer where the indices of died particles lay.

	int16_t shaderContextInd = -1;

friend class Scene;
};




struct EmitterShaderContext		// Different emitters have different ways of emitting and processing particles.
{								// Their compute shaders must be compiled with different macroses.

	int maxParticles = 0;
	ShaderMacroStrings macroStrings;
	ComputeTask pInitPartsTask;	// First inition: set the particles outside of view.
	ComputeTask pEmitPartsTask;	// Emitting the particles through (by means of) EmitProps.
	ComputeTask pProcPartsTask;	// Processing the particles: position, velocity, size, color, uv-transformation, etc.
	ShaderProgram pDrawProgram;	// Draw the particles.

	EmitterShaderContext(int maxParts, ComputeTask pInitTask, ComputeTask pEmitTask, ComputeTask pProcTask, ShaderProgram pProgram, CShaderMacroStrings macroses)
	{
		maxParticles = maxParts;

		macroStrings = macroses;

		pInitPartsTask = pInitTask;
		pEmitPartsTask = pEmitTask;
		pProcPartsTask = pProcTask;
		pDrawProgram = pProgram;
	}

	void release()
	{
		if (pInitPartsTask)
			pInitPartsTask->release();
		pInitPartsTask = nullptr;

		if (pEmitPartsTask)
			pEmitPartsTask->release();
		pEmitPartsTask = nullptr;

		if (pProcPartsTask)
			pProcPartsTask->release();
		pProcPartsTask = nullptr;

		if (pDrawProgram)
			pDrawProgram->release();
		pDrawProgram = nullptr;
	}
};