#pragma once

#include <windows.h>

#include "../Defines.h"


class IShader abstract
{
public:
	virtual void release() = 0;
};

typedef std::shared_ptr<IShader> Shader;



class IShaderProgram abstract
{
public:
	virtual void release() = 0;

	virtual Shader getVertexShader() = 0;

	virtual Shader getPixelShader() = 0;

	virtual ~IShaderProgram() {}
};

typedef std::shared_ptr<IShaderProgram> ShaderProgram;
//typedef IShaderProgram* ShaderProgram;



// Pairs of the macro name and macro definition. It uses for the variant compilation of the shaders.
typedef std::vector<std::pair<std::string, std::string>> ShaderMacroStrings;
typedef const ShaderMacroStrings& CShaderMacroStrings;

