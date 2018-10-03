#include <cassert>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <locale>
#include <codecvt>

#include "../Utils/Utils.h"
#include "ShaderProgramDX11.h"



void ShaderProgramDX11::release()
{
	if (pVertexShader_)
		pVertexShader_->release();

	if (pPixelShader_)
		pPixelShader_->release();
}


Shader ShaderProgramDX11::getVertexShader()
{
	return pVertexShader_;
}


Shader ShaderProgramDX11::getPixelShader()
{
	return pPixelShader_;
}


void ShowShaderError(ID3DBlob* pError)
{
	if (pError)
	{
		LPVOID pErrorBuff = pError->GetBufferPointer();
		if (pErrorBuff)
		{
			const char* pcError = static_cast<const char*>(pErrorBuff);

			if (pcError)
			{	ShowError(nullptr, __FUNCTION__, " error: D3DCompile != S_OK.\n\nError: ", pcError);
			}
		}
	}
}


ID3DBlob* ShaderProgramDX11::compileShaderCode(PCCHAR pCodeText, PCCHAR pTarget, D3D_SHADER_MACRO* pDefines)
{
	uint32_t flags = (bDebugEnabled_) ? (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION) : D3DCOMPILE_OPTIMIZATION_LEVEL3;

	ID3DBlob* pCode = nullptr;
	ComPtr<ID3DBlob> pError = nullptr;

	HRESULT hr = D3DCompile(pCodeText, strlen(pCodeText), nullptr, pDefines, nullptr, "Main", pTarget, flags, 0, &pCode, &pError);
	if (S_OK != hr)
	{	::ShowShaderError(pError.Get());
		return nullptr;
	}

	return pCode;
}


ID3D11DeviceChild* ShaderProgramDX11::createShaderFromFile(cstring filePath, PCCHAR pTarget, CShaderMacroStrings defines, ID3D11Device* pDevice)
{
	uint32_t flags = (bDebugEnabled_) ? (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION) : D3DCOMPILE_OPTIMIZATION_LEVEL3;

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wpath = converter.from_bytes(filePath);

	// Build DX macros.
	std::vector<D3D_SHADER_MACRO> dx_defines;
	for (const std::pair<cstring, cstring>& pair : defines)
	{
		D3D_SHADER_MACRO macro = { pair.first.c_str(), pair.second.c_str() };
		dx_defines.push_back(macro);
	}
	D3D_SHADER_MACRO macro = { nullptr, nullptr };
	dx_defines.push_back(macro);

	ComPtr<ID3DBlob> pCode = nullptr;
	ComPtr<ID3DBlob> pError = nullptr;

	HRESULT hr = D3DCompileFromFile(wpath.c_str(), &dx_defines.front(), D3D_COMPILE_STANDARD_FILE_INCLUDE,
									"Main", pTarget, flags, 0, &pCode, &pError);
	if (S_OK != hr)
	{	::ShowShaderError(pError.Get());
		return nullptr;
	}

	if (strstr(pTarget, "vs"))
	{
		ID3D11VertexShader* pDxVertexShader = nullptr;
		hr = pDevice->CreateVertexShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), nullptr, &pDxVertexShader);
		if (S_OK == hr)
			return pDxVertexShader;
	}

	if (strstr(pTarget, "ps"))
	{
		ID3D11PixelShader* pDxPixelShader = nullptr;
		hr = pDevice->CreatePixelShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), nullptr, &pDxPixelShader);
		if (S_OK == hr)
			return pDxPixelShader;
	}

	if (strstr(pTarget, "cs"))
	{
		ID3D11ComputeShader* pDxComputeShader = nullptr;
		hr = pDevice->CreateComputeShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), nullptr, &pDxComputeShader);
		if (S_OK == hr)
			return pDxComputeShader;
	}

	return nullptr;
}

	