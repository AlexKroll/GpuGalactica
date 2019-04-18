#pragma once

#include <d3d11.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>

#include "ShaderProgram.h"


template <typename ShaderType>
class ShaderDX11 : public IShader
{
public:
	virtual void release()
	{
		if (pDxShader_)
		{	pDxShader_->Release();
			pDxShader_ = nullptr;
		}
	}

	ShaderType* getDxShader()
	{
		return pDxShader_;
	}


private:
	ShaderType* pDxShader_ = nullptr;

	friend class RenderDX11;
};

// DX Vertex shader
typedef ShaderDX11<ID3D11VertexShader> VShaderDX11;
typedef std::shared_ptr<VShaderDX11> VertexShaderDX11;

// DX Pixel shader
typedef ShaderDX11<ID3D11PixelShader> PShaderDX11;
typedef std::shared_ptr<PShaderDX11> PixelShaderDX11;

// DX Compute shader
//typedef ShaderDX11<ID3D11ComputeShader> CShaderDX11;
//typedef std::shared_ptr<CShaderDX11> ComputeShaderDX11;




class ShaderProgramDX11 : public IShaderProgram
{
public:
	virtual void release() final;

	virtual Shader getVertexShader() final;

	virtual Shader getPixelShader() final;

private:
	static const bool bDebugEnabled_ = false;

	VertexShaderDX11 pVertexShader_ = nullptr;
	PixelShaderDX11 pPixelShader_ = nullptr;

	ID3DBlob* compileShaderCode(PCCHAR pCodeText, PCCHAR pTarget, D3D_SHADER_MACRO* pDefines);

	ID3D11DeviceChild* createShaderFromFile(cstring filePath, PCCHAR pTarget, CShaderMacroStrings defines, ID3D11Device* pDevice);

public:
	ShaderProgramDX11() {}

	virtual ~ShaderProgramDX11()
	{
		release();
	}

	friend class RenderDX11;
};



