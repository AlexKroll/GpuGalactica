#pragma once

#include "BaseObject.h"



class Billboard : public BaseObject
{
public:
	

protected:
	virtual void init(cstring name, CVec4 pos, CVec4 orient, CVec4 size, Mesh pMesh, Texture pTexture, Render pRender) override;

	static Mesh pQuadMesh;
	static VertexInput pVertexInput;
	static ShaderProgram pShaderProgram;

	struct Vertex
	{
		Vec3 pos;
		Vec2 tcoords;
	};

private:
	
friend class Scene;

protected:
	virtual void process(float elapsedTime, Scene* pScene) override;

	virtual void render(Render pRender) override;
};
